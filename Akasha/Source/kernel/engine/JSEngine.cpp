#include "JSEngine.h"
#ifndef _DEBUG
namespace Akasha {
	// V8GlobalManager implementation
	V8GlobalManager& V8GlobalManager::getInstance() {
		static V8GlobalManager instance;
		return instance;
	}

	V8GlobalManager::V8GlobalManager() {
		v8::V8::SetFlagsFromString("--turbo-inlining --no-lazy --turbo-fast-api-calls");
		v8::V8::InitializeICUDefaultLocation(".");
		v8::V8::InitializeExternalStartupData(".");
		platform = v8::platform::NewDefaultPlatform();
		v8::V8::InitializePlatform(platform.get());
		v8::V8::Initialize();
	}

	V8GlobalManager::~V8GlobalManager() {
		v8::V8::Dispose();
		v8::V8::DisposePlatform();
	}

	v8::Platform* V8GlobalManager::getPlatform() {
		return platform.get();
	}
	// JSEngine implementation
	JSEngine::JSEngine() {
		auto& globalManager = V8GlobalManager::getInstance();
		create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
		isolate = v8::Isolate::New(create_params);
	}

	JSEngine::~JSEngine() {
		cache.Reset(isolate);

		if (isolate) {
			isolate->Dispose();
		}
		delete create_params.array_buffer_allocator;
	}

	bool JSEngine::loadFunction(const std::string& source_code, juce::String& info) {
		std::lock_guard<std::mutex> lock(mutex);
		function_ready = false;
		for (int i = 0; i < num_voices; i++) {
			function_just_ready_for_voice[i] = false;
		}
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);

		auto& context = cache.context;
		context.Reset(isolate, v8::Context::New(isolate));
		v8::Context::Scope context_scope(context.Get(isolate));
		// user's code and main
		if (!compileAndRunScript(source_code, context.Get(isolate), info)) {
			return false;
		}
		v8::Local<v8::Function> VoiceClass = getGlobalClass("Voice", context.Get(isolate), info);
		if (VoiceClass.IsEmpty()) {
			return false;
		}
		// main wrapper
		if (!compileAndRunScript(mainWrapperScript, context.Get(isolate), info)) {
			return false;
		}
		v8::Local<v8::Function> mainWrapperFunc = getGlobalFunction("mainWrapper", context.Get(isolate), info);
		if (mainWrapperFunc.IsEmpty()) {
			return false;
		}
		cache.mainWrapperFunction.Reset(isolate, mainWrapperFunc);
		cache.globalObject.Reset(isolate, context.Get(isolate)->Global());

		function_ready = true;
		for (int i = 0; i < num_voices; i++) {
			function_just_ready_for_voice[i] = true;
		}
		return true;
	}

	bool JSEngine::callMainWrapperFunction(const JSMainWrapperParams& args, juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples, juce::String& info, int voiceId) {
		std::lock_guard<std::mutex> lock(mutex);
		voiceId = voiceId % num_voices;
		v8::Global<v8::Context>& cached_context = cache.context;
		v8::Global<v8::Object>& cached_global = cache.globalObject;
		v8::Global<v8::Function>& function = cache.mainWrapperFunction;
		if (cached_context.IsEmpty()) {
			info = juce::String("Not compiled.\n");
			function_ready = false;
			return false;
		}
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);
		auto cached_context_local = cached_context.Get(isolate);
		auto cached_global_local = cached_global.Get(isolate);
		auto cached_function_local = function.Get(isolate);

		v8::Context::Scope context_scope(cached_context_local);
		v8::TryCatch try_catch(isolate);

		prepareMainWrapperArguments(args, voiceId);
		auto js_args1 = cache.arrayBufferArgs1View.Get(isolate);
		auto js_args2 = cache.arrayBufferArgs2View.Get(isolate);
		auto js_args_buffer = cache.channelBuffersView.Get(isolate);
		v8::Local<v8::Value> js_func_args[3] = { js_args1, js_args2, js_args_buffer };
		v8::Local<v8::Value> result;
		if (!cached_function_local->Call(cached_context_local, cached_global_local, 3, js_func_args).ToLocal(&result)) {
			v8::String::Utf8Value error(isolate, try_catch.Exception());
			std::string error_str(*error);
			info = juce::String(error_str) + "\n";
			try_catch.Reset();
			function_ready = false;
			return false;
		}

		// fill outputBuffer
		if (outputBuffer.getNumChannels() < args.numChannels || outputBuffer.getNumSamples() < startSample + numSamples) {
			info = juce::String("Output buffer size is insufficient.\n");
			function_ready = false;
			return false;
		}
		for (int channel = 0; channel < args.numChannels; ++channel) {
			auto& channelBuffer = cache.channelBuffers[channel];
			if (channelBuffer.IsEmpty()) {
				info = juce::String("Channel buffer is empty.\n");
				function_ready = false;
				return false;
			}
			// Get the raw float array from the backing store
			v8::Local<v8::ArrayBuffer> arrayBuffer = channelBuffer.Get(isolate);
			float* channelData = static_cast<float*>(arrayBuffer->GetBackingStore()->Data());
			outputBuffer.addFrom(channel, startSample, channelData, numSamples, 1.0f);
			// auto* outputChannelData = outputBuffer.getWritePointer(channel, startSample);
			// for (int sample = 0; sample < numSamples; ++sample) {
			// 	outputChannelData[sample] += channelData[sample];
			// }
		}
		function_ready = true;
		function_just_ready_for_voice[voiceId] = false;
		return true;
	}


	bool JSEngine::isFunctionReady() const {
		return function_ready;
	}

	bool JSEngine::isFunctionJustReadyForVoice(int voiceId) const {
		return function_just_ready_for_voice[voiceId];
	}

	void JSEngine::prepareMainWrapperArguments(const JSMainWrapperParams& params, int voiceId) {
		// buffer
		bool needsReallocation = false;
		if (cache.channelBuffersView.IsEmpty()) {
			needsReallocation = true;
		}
		else {
			v8::Local<v8::Array> audioBuffer = cache.channelBuffersView.Get(isolate);
			if (audioBuffer->Length() != params.numChannels) {
				needsReallocation = true;
			}
			else {
				v8::Local<v8::Value> bufferValue = audioBuffer->Get(cache.context.Get(isolate), 0).ToLocalChecked();
				v8::Local<v8::ArrayBuffer> arrayBuffer = bufferValue.As<v8::ArrayBuffer>();
				if (arrayBuffer->ByteLength() < params.numSamples * sizeof(float)) {
					needsReallocation = true;
				}
			}
		}
		if (needsReallocation) {
			allocateChannelBuffers(params, cache);
		}
		// args1
		if (cache.arrayBufferArgs1.IsEmpty()) {
			auto backingStore = v8::ArrayBuffer::NewBackingStore(
				new double[macro_len],
				macro_len * sizeof(double),
				[](void* data, size_t length, void* deleterData) {
					delete[] static_cast<double*>(data);
				},
				nullptr
			);
			v8::Local<v8::ArrayBuffer> arrayBuffer = v8::ArrayBuffer::New(isolate, std::move(backingStore));
			cache.arrayBufferArgs1.Reset(isolate, arrayBuffer);
			cache.arrayBufferArgs1View.Reset(isolate, v8::Float64Array::New(arrayBuffer, 0, macro_len));
		}
		// args2
		if (cache.arrayBufferArgs2.IsEmpty()) {
			auto backingStore = v8::ArrayBuffer::NewBackingStore(
				new double[infoArg_len],
				infoArg_len * sizeof(double),
				[](void* data, size_t length, void* deleterData) {
					delete[] static_cast<double*>(data);
				},
				nullptr
			);
			v8::Local<v8::ArrayBuffer> arrayBuffer = v8::ArrayBuffer::New(isolate, std::move(backingStore));
			cache.arrayBufferArgs2.Reset(isolate, arrayBuffer);
			cache.arrayBufferArgs2View.Reset(isolate, v8::Float64Array::New(arrayBuffer, 0, infoArg_len));
		}
		// set data
		double* bufferData1 = static_cast<double*>(cache.arrayBufferArgs1.Get(isolate)->GetBackingStore()->Data());
		double* bufferData2 = static_cast<double*>(cache.arrayBufferArgs2.Get(isolate)->GetBackingStore()->Data());
		for (size_t i = 0; i < params.macros.size(); ++i) {
			bufferData1[i] = static_cast<double>(*params.macros[i]);
		}
		bufferData2[0] = static_cast<double>(params.numSamples);
		bufferData2[1] = static_cast<double>(params.numChannels);
		bufferData2[2] = static_cast<double>(params.sampleRate);
		bufferData2[3] = static_cast<double>(params.tempo);
		bufferData2[4] = static_cast<double>(params.beat);
		bufferData2[5] = static_cast<double>(params.justPressed ? 1.0 : 0.0);
		bufferData2[6] = static_cast<double>(params.voiceId);
		bufferData2[7] = static_cast<double>(params.note);
		bufferData2[8] = static_cast<double>(params.velocity);
	}

	void JSEngine::allocateChannelBuffers(const JSMainWrapperParams& params, Cache& cache) {
		for (auto& channelBuffer : cache.channelBuffers) {
			channelBuffer.Reset();
		}
		if (cache.channelBuffers.size() != params.numChannels) {
			cache.channelBuffers.resize(params.numChannels);
		}
		v8::Local<v8::Array> audioBuffer = v8::Array::New(isolate, params.numChannels);
		for (int channel = 0; channel < params.numChannels; ++channel) {
			float* channelData = new float[params.numSamples]();
			auto backingStore = v8::ArrayBuffer::NewBackingStore(
				channelData,
				params.numSamples * sizeof(float),
				[](void* data, size_t length, void* deleterData) {
					delete[] static_cast<float*>(data);
				},
				nullptr
			);
			v8::Local<v8::ArrayBuffer> arrayBuffer = v8::ArrayBuffer::New(isolate, std::move(backingStore));
			v8::Local<v8::Float32Array> floatArray = v8::Float32Array::New(arrayBuffer, 0, params.numSamples);
			cache.channelBuffers[channel].Reset(isolate, arrayBuffer);
			audioBuffer->Set(cache.context.Get(isolate), channel, floatArray).Check();
		}
		cache.channelBuffersView.Reset(isolate, audioBuffer);
	}

	bool JSEngine::compileAndRunScript(const std::string& scriptSource, v8::Local<v8::Context> context, juce::String& info) {
		v8::TryCatch try_catch(isolate);
		v8::ScriptCompiler::Source source(v8::String::NewFromUtf8(isolate, scriptSource.c_str()).ToLocalChecked());
		v8::Local<v8::Script> script;
		if (!v8::ScriptCompiler::Compile(context, &source).ToLocal(&script)) {
			v8::String::Utf8Value error(isolate, try_catch.Exception());
			info = juce::String(*error);
			return false;
		}
		if (script->Run(context).IsEmpty()) {
			v8::String::Utf8Value error(isolate, try_catch.Exception());
			info = juce::String(*error);
			return false;
		}
		return true;
	}

	v8::Local<v8::Function> JSEngine::getGlobalFunction(const std::string& functionName, v8::Local<v8::Context> context, juce::String& info) {
		v8::Local<v8::Value> func_value;
		if (!context->Global()->Get(context, v8::String::NewFromUtf8(isolate, functionName.c_str()).ToLocalChecked()).ToLocal(&func_value)) {
			info = juce::String("Function '" + functionName + "' not found.\n");
			return v8::Local<v8::Function>();
		}
		if (!func_value->IsFunction()) {
			info = juce::String("'" + functionName + "' is not a function.\n");
			return v8::Local<v8::Function>();
		}
		return func_value.As<v8::Function>();
	}


	v8::Local<v8::Function> JSEngine::getGlobalClass(const std::string& className, v8::Local<v8::Context> context, juce::String& info) {
		v8::Local<v8::Value> class_value;

		if (!context->Global()->Get(context, v8::String::NewFromUtf8(isolate, className.c_str()).ToLocalChecked()).ToLocal(&class_value)) {
			info = juce::String("Class '" + className + "' not found.\n");
			return v8::Local<v8::Function>();
		}

		// if (!class_value->IsFunction()) {
		// 	info = juce::String("'" + className + "' is not a valid class (not a function).\n");
		// 	return v8::Local<v8::Function>();
		// }

		return class_value.As<v8::Function>();
	}

	void JSEngine::Cache::Reset(v8::Isolate* isolate) {
		context.Reset();
		globalObject.Reset();
		channelBuffersView.Reset();
		for (auto& channelBuffer : channelBuffers) {
			channelBuffer.Reset();
		}
		mainWrapperFunction.Reset();
		channelBuffers.clear();
		arrayBufferArgs1.Reset();
		arrayBufferArgs1View.Reset();
		arrayBufferArgs2.Reset();
		arrayBufferArgs2View.Reset();
	}
}
#endif