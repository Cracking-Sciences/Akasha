#pragma once


namespace Akasha {
	struct JSFuncParams {
		std::array<std::atomic<float>*, 8> macros;
		double tempo = 0.0;
		double beat = 0.0;
		double sampleRate = 0.0;

		// advanced audio buffer info
		int bufferLen;
		int bufferPos;

		// std::array<double(&)(double), 4> lfoFuncs; // TODO: lfo functions
		// TODO: MPE
		double time;
		int note; // note
		float velocity; // velocity
		bool justPressed; // just note on
		bool justReleased; // just note off
	};
}



#ifdef _DEBUG
namespace Akasha {
	class JSEngine {
	public:
		JSEngine() {}
		~JSEngine() {}
		bool loadFunction(const std::string& source_code, juce::String& info) { return true; }
		bool callFunction(const JSFuncParams& args, std::vector<double>& result_vector, juce::String& info, int voiceId = 0) { return true; }
		bool isFunctionReady() const { return true; }
	};
}
#endif

#ifndef _DEBUG

#include "v8.h"
#include "libplatform/libplatform.h"
#include "v8-context.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-primitive.h"
#include "v8-script.h"
#include <mutex>

namespace Akasha {

	class V8GlobalManager {
	public:
		static V8GlobalManager& getInstance() {
			static V8GlobalManager instance;
			return instance;
		}

		v8::Platform* getPlatform() { return platform.get(); }

	private:
		V8GlobalManager() {
			v8::V8::InitializeICUDefaultLocation(".");
			v8::V8::InitializeExternalStartupData(".");
			platform = v8::platform::NewDefaultPlatform();
			v8::V8::InitializePlatform(platform.get());
			v8::V8::Initialize();
		}

		~V8GlobalManager() {
			v8::V8::Dispose();
			v8::V8::DisposePlatform();
		}
		V8GlobalManager(const V8GlobalManager&) = delete;
		V8GlobalManager& operator=(const V8GlobalManager&) = delete;
		std::unique_ptr<v8::Platform> platform;
	};

	class JSEngine {
	public:
		JSEngine() {
			auto& globalManager = V8GlobalManager::getInstance();
			create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
			isolate = v8::Isolate::New(create_params);
			cachedList.resize(num_cached_contexts);
		}

		~JSEngine() {
			for (auto& cached : cachedList) {
				cached.Reset(isolate);
			}
			if (isolate) {
				isolate->Dispose();
			}
			delete create_params.array_buffer_allocator;
		}

		bool loadFunction(const std::string& source_code, juce::String& info) {
			std::lock_guard<std::mutex> lock(mutex); 
			function_ready = false;
			v8::Isolate::Scope isolate_scope(isolate);
			v8::HandleScope handle_scope(isolate);
			for (int i = 0; i < num_cached_contexts; i++) {
				auto& context = cachedList[i].context;
				context.Reset(isolate, v8::Context::New(isolate));
				v8::Context::Scope context_scope(context.Get(isolate));
				v8::TryCatch try_catch(isolate);
				v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, source_code.c_str()).ToLocalChecked();
				v8::Local<v8::Script> script;
				if (!v8::Script::Compile(context.Get(isolate), source).ToLocal(&script)) {
					v8::String::Utf8Value error(isolate, try_catch.Exception());
					std::string error_str(*error);
					info = juce::String(error_str);
					try_catch.Reset();
					return false;
				}

				if (script->Run(context.Get(isolate)).IsEmpty()) {
					v8::String::Utf8Value error(isolate, try_catch.Exception());
					std::string error_str(*error);
					info = juce::String(error_str);
					try_catch.Reset();
					return false;
				}

				v8::Local<v8::Value> func_value;
				if (!context.Get(isolate)->Global()->Get(context.Get(isolate), v8::String::NewFromUtf8(isolate, "main").ToLocalChecked()).ToLocal(&func_value)) {
					return false;
				}
				if (!func_value->IsFunction()) {
					info = juce::String("function 'main' not found.\n");
					return false;
				}

				cachedList[i].function.Reset(isolate, func_value.As<v8::Function>());
				cachedList[i].globalObject.Reset(isolate, context.Get(isolate)->Global());
				cachedList[i].arrayBufferArguments.Reset(isolate, v8::ArrayBuffer::New(isolate, array_buffer_len * sizeof(double)));
			}
			function_ready = true;
			return true;
		}

		bool callFunction(const JSFuncParams& args, std::vector<double>& result_vector, juce::String& info, int voiceId = 0) {
			std::lock_guard<std::mutex> lock(mutex); 
			// if there is a problem (return false), the info will be filled with the error message.
			voiceId = voiceId % num_cached_contexts;
			v8::Global<v8::Context>& cached_context = cachedList[voiceId].context;
			v8::Global<v8::Object>& cached_global = cachedList[voiceId].globalObject;
			v8::Global<v8::Function>& function = cachedList[voiceId].function;

			if (cached_context.IsEmpty()) {
				info = juce::String("Not compiled.\n");
				function_ready = false;
				return false;
			}
			v8::Isolate::Scope isolate_scope(isolate);
			v8::HandleScope handle_scope(isolate);
			v8::Context::Scope context_scope(cached_context.Get(isolate));
			// return false; // debug
			v8::TryCatch try_catch(isolate);

			// Convert the C++ array to a JavaScript array
			auto& js_args = prepareArguments(args, voiceId);

			// Prepare arguments for the function (only one argument, the array)
			v8::Local<v8::Value> js_func_args[1] = { js_args };
			v8::Local<v8::Value> result;

			if (!function.Get(isolate)->Call(cached_context.Get(isolate), cached_global.Get(isolate), 1, js_func_args).ToLocal(&result)) {
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
						if (result_array->Get(cached_context.Get(isolate), i).ToLocal(&element)) {
							if (element->IsNumber()) {
								result_vector[i] = element->NumberValue(cached_context.Get(isolate)).ToChecked();
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
					result_vector[i] = (result->NumberValue(cached_context.Get(isolate)).ToChecked());
				}
			}
			else {
				info = juce::String("Result is not an array or a number.\n");
				return false;
			}
			function_ready = true;
			return true;
		}

		bool isFunctionReady() const {
			return function_ready;
		}

	private:
		v8::Local<v8::Float64Array> const prepareArguments(const JSFuncParams& params, int voiceId){
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

		struct Cache {
			v8::Global<v8::Context> context;
			v8::Global<v8::Object> globalObject;
			v8::Global<v8::Function> function;
			v8::Global<v8::ArrayBuffer> arrayBufferArguments; // for arguments. Controlling underlying buffer
			v8::Global<v8::Float64Array> float64ArrayArguments; // view of the array buffer

			Cache() = default;

			Cache(const Cache&) = delete;
			Cache& operator=(const Cache&) = delete;
			Cache(Cache&&) = default;
			Cache& operator=(Cache&&) = default;

			void Reset(v8::Isolate* isolate) {
				context.Reset();
				globalObject.Reset();
				function.Reset();
				arrayBufferArguments.Reset();
				float64ArrayArguments.Reset();
			}
		};

		std::vector<Cache> cachedList;

		v8::Isolate::CreateParams create_params;
		v8::Isolate* isolate;
		std::unique_ptr<v8::Platform> platform;

		const int num_cached_contexts = 16;
		const int array_buffer_len = 20;

		bool function_ready = false;
		mutable std::mutex mutex; 
	};
}
#endif
