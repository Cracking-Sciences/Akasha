#include "JSEngine.h"

namespace Akasha {
	// V8GlobalManager implementation
	V8GlobalManager& V8GlobalManager::getInstance() {
		static V8GlobalManager instance;
		return instance;
	}

	V8GlobalManager::V8GlobalManager() {
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
		cachedList.resize(num_cached_contexts);
	}

	JSEngine::~JSEngine() {
		for (auto& cached : cachedList) {
			cached.Reset(isolate);
		}
		if (isolate) {
			isolate->Dispose();
		}
		delete create_params.array_buffer_allocator;
	}

	bool JSEngine::loadFunction(const std::string& source_code, juce::String& info) {
		std::lock_guard<std::mutex> lock(mutex);
		function_ready = false;
		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope handle_scope(isolate);
		for (int i = 0; i < num_cached_contexts; i++) {
			auto& context = cachedList[i].context;
			context.Reset(isolate, v8::Context::New(isolate));
			v8::Context::Scope context_scope(context.Get(isolate));
			// user's code and main
			if (!compileAndRunScript(source_code, context.Get(isolate), info)) {
				return false;
			}
			v8::Local<v8::Function> mainFunc = getGlobalFunction("main", context.Get(isolate), info);
			if (mainFunc.IsEmpty()) {
				return false;
			}
			cachedList[i].mainFunction.Reset(isolate, mainFunc);
			// main wrapper
			if (!compileAndRunScript(mainWrapperScript, context.Get(isolate), info)) {
				return false;
			}
			v8::Local<v8::Function> mainWrapperFunc = getGlobalFunction("mainWrapper", context.Get(isolate), info);
			if (mainWrapperFunc.IsEmpty()) {
				return false;
			}
			// remember main wrapper
			cachedList[i].mainWrapperFunction.Reset(isolate, mainWrapperFunc);
			cachedList[i].globalObject.Reset(isolate, context.Get(isolate)->Global());
		}
		function_ready = true;
		return true;
	}

	bool JSEngine::callMainFunction(const JSFuncParams& args, std::vector<double>& result_vector, juce::String& info, int voiceId) {
		// legacy method
		std::lock_guard<std::mutex> lock(mutex);
		voiceId = voiceId % num_cached_contexts;
		v8::Global<v8::Context>& cached_context = cachedList[voiceId].context;
		v8::Global<v8::Object>& cached_global = cachedList[voiceId].globalObject;
		v8::Global<v8::Function>& function = cachedList[voiceId].mainFunction;

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
		// return false; // debug
		v8::TryCatch try_catch(isolate);

		// Convert the C++ array to a JavaScript array
		auto& js_args = prepareArguments(args, voiceId);

		// Prepare arguments for the function (only one argument, the array)
		v8::Local<v8::Value> js_func_args[1] = { js_args };
		v8::Local<v8::Value> result;

		if (!cached_function_local->Call(cached_context_local, cached_global_local, 1, js_func_args).ToLocal(&result)) {
			v8::String::Utf8Value error(isolate, try_catch.Exception());
			std::string error_str(*error);
			info = juce::String(error_str) + "\n";
			// std::cerr << "JavaScript Error: " << *error << std::endl;
			try_catch.Reset();
			function_ready = false;
			return false;
		}

		if (result->IsArray()) {
			v8::Local<v8::Array> result_array = result.As<v8::Array>();
			uint32_t length = result_array->Length();
			for (uint32_t i = 0; i < result_vector.size(); i++) {
				if (i < length) {
					v8::Local<v8::Value> element;
					if (result_array->Get(cached_context_local, i).ToLocal(&element)) {
						if (element->IsNumber()) {
							result_vector[i] = element->NumberValue(cached_context_local).ToChecked();
						}
						else {
							result_vector[i] = 0.0; // Default to 0.0 if not a number
						}
					}
				}
				else {
					result_vector[i] = result_vector[0]; // Default to the first element if out of bounds
				}
			}
		}
		else if (result->IsNumber()) {
			for (uint32_t i = 0; i < result_vector.size(); i++) {
				result_vector[i] = (result->NumberValue(cached_context_local).ToChecked());
			}
		}
		else {
			info = juce::String("Result is not an array or a number.\n");
			return false;
		}
		function_ready = true;
		return true;
	}

	bool JSEngine::callMainWrapperFunction(const JSMainWrapperParams& args, juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples, juce::String& info, int voiceId) {
		std::lock_guard<std::mutex> lock(mutex);
		voiceId = voiceId % num_cached_contexts;
		v8::Global<v8::Context>& cached_context = cachedList[voiceId].context;
		v8::Global<v8::Object>& cached_global = cachedList[voiceId].globalObject;
		v8::Global<v8::Function>& function = cachedList[voiceId].mainWrapperFunction;
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
		auto js_args1 = v8::Float64Array::New(cachedList[voiceId].arrayBufferArgs1.Get(isolate),0,macro_len);
		auto js_args2 = v8::Float64Array::New(cachedList[voiceId].arrayBufferArgs2.Get(isolate),0,array_buffer_len);
		auto js_args_buffer = cachedList[voiceId].arrayAudioBuffer.Get(isolate);
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
			return false;
		}
		for (int channel = 0; channel < args.numChannels; ++channel) {
			auto& channelBuffer = cachedList[voiceId].channelBuffers[channel];
			if (channelBuffer.IsEmpty()) {
				info = juce::String("Channel buffer is empty.\n");
				return false;
			}
			// Get the raw float array from the backing store
			v8::Local<v8::ArrayBuffer> arrayBuffer = channelBuffer.Get(isolate);
			float* channelData = static_cast<float*>(arrayBuffer->GetBackingStore()->Data());
			auto* outputChannelData = outputBuffer.getWritePointer(channel, startSample);
			for (int sample = 0; sample < numSamples; ++sample) {
				outputChannelData[sample] += channelData[sample];
			}
		}

		function_ready = true;
		return true;
	}

	bool JSEngine::isFunctionReady() const {
		return function_ready;
	}

	v8::Local<v8::Float64Array> const JSEngine::prepareArguments(const JSFuncParams& params, int voiceId) {
		// legacy method
		if (cachedList[voiceId].arrayBufferArguments.IsEmpty() ||
			cachedList[voiceId].float64ArrayArguments.IsEmpty()) {
			auto backingStore = v8::ArrayBuffer::NewBackingStore(
				new double[array_buffer_len],
				array_buffer_len * sizeof(double),
				[](void* data, size_t length, void* deleterData) {
					delete[] static_cast<double*>(data);
				},
				nullptr
			);
			v8::Local<v8::ArrayBuffer> arrayBuffer = v8::ArrayBuffer::New(isolate, std::move(backingStore));
			cachedList[voiceId].arrayBufferArguments.Reset(isolate, arrayBuffer);
			v8::Local<v8::Float64Array> float64Array = v8::Float64Array::New(arrayBuffer, 0, array_buffer_len);
			cachedList[voiceId].float64ArrayArguments.Reset(isolate, float64Array);
		}

		v8::Local<v8::ArrayBuffer> arrayBuffer = cachedList[voiceId].arrayBufferArguments.Get(isolate);
		double* bufferData = static_cast<double*>(arrayBuffer->GetBackingStore()->Data());

		for (size_t i = 0; i < params.macros.size(); ++i) {
			bufferData[i] = static_cast<double>(*params.macros[i]);
		}
		bufferData[8] = params.tempo;
		bufferData[9] = params.beat;
		bufferData[10] = params.sampleRate;
		bufferData[11] = params.bufferLen;
		bufferData[12] = params.bufferPos;
		bufferData[13] = params.time;
		bufferData[14] = params.note;
		bufferData[15] = params.velocity;
		bufferData[16] = params.justPressed ? 1.0 : 0.0;
		bufferData[17] = params.justReleased ? 1.0 : 0.0;

		v8::Local<v8::Float64Array> float64Array = cachedList[voiceId].float64ArrayArguments.Get(isolate);
		return float64Array;
	}

	void JSEngine::prepareMainWrapperArguments(const JSMainWrapperParams& params, int voiceId) {
		auto& cache = cachedList[voiceId];
		// buffer
		bool needsReallocation = false;
		if (cache.arrayAudioBuffer.IsEmpty()) {
			needsReallocation = true;
		}
		else {
			v8::Local<v8::Array> audioBuffer = cache.arrayAudioBuffer.Get(isolate);
			if (audioBuffer->Length() != params.numChannels) {
				needsReallocation = true;
			}
			else {
				for (int i = 0; i < params.numChannels; ++i) {
					v8::Local<v8::Value> bufferValue = audioBuffer->Get(cache.context.Get(isolate), i).ToLocalChecked();
					if (!bufferValue->IsArrayBuffer()) {
						needsReallocation = true;
						break;
					}
					v8::Local<v8::ArrayBuffer> arrayBuffer = bufferValue.As<v8::ArrayBuffer>();
					if (arrayBuffer->ByteLength() < params.numSamples * sizeof(float)) {
						needsReallocation = true;
						break;
					}
				}
			}
		}
		if (needsReallocation) {
			allocateArrayAudioBuffer(params, cache);
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
		}
		// args2
		if (cache.arrayBufferArgs2.IsEmpty()) {
			auto backingStore = v8::ArrayBuffer::NewBackingStore(
				new double[array_buffer_len],
				array_buffer_len * sizeof(double),
				[](void* data, size_t length, void* deleterData) {
					delete[] static_cast<double*>(data);
				},
				nullptr
			);
			v8::Local<v8::ArrayBuffer> arrayBuffer = v8::ArrayBuffer::New(isolate, std::move(backingStore));
			cache.arrayBufferArgs2.Reset(isolate, arrayBuffer);
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
		bufferData2[6] = static_cast<double>(params.justReleased ? 1.0 : 0.0);
		bufferData2[7] = static_cast<double>(params.note);
		bufferData2[8] = static_cast<double>(params.velocity);
	}

	void JSEngine::allocateArrayAudioBuffer(const JSMainWrapperParams& params, Cache& cache) {
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
		cache.arrayAudioBuffer.Reset(isolate, audioBuffer);
	}

	bool JSEngine::compileAndRunScript(const std::string& scriptSource, v8::Local<v8::Context> context, juce::String& info) {
		v8::TryCatch try_catch(isolate);
		v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, scriptSource.c_str()).ToLocalChecked();
		v8::Local<v8::Script> script;
		if (!v8::Script::Compile(context, source).ToLocal(&script)) {
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

	void JSEngine::Cache::Reset(v8::Isolate* isolate) {
		context.Reset();
		globalObject.Reset();
		mainFunction.Reset();
		arrayBufferArguments.Reset();
		float64ArrayArguments.Reset();
		arrayAudioBuffer.Reset();
		for (auto& channelBuffer : channelBuffers) {
			channelBuffer.Reset();
		}
		mainWrapperFunction.Reset();
		channelBuffers.clear();
		arrayBufferArgs1.Reset();
		arrayBufferArgs2.Reset();
	}
}