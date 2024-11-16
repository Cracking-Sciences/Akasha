#pragma once


namespace Akasha {
	struct JSFuncParams {
		std::array<double, 8>& macros; // macros
		double tempo; // tempo
		double beat;
		double sampleRate; // sample rate
		double time; // time

		// std::array<double(&)(double), 4> lfoFuncs; // TODO: lfo functions
		// TODO: MPE
		int note; // note
		float velocity; // velocity

		JSFuncParams(std::array<double, 8>& macrosRef)
			: macros(macrosRef), tempo(0.0), beat(0.0), sampleRate(0.0), time(0.0), note(0), velocity(0.0f) {
		}
	};
}





#ifdef _DEBUG
namespace Akasha {
	class JSEngine {
	public:
		JSEngine() {}
		~JSEngine() {}
		bool loadFunction(const std::string& source_code, juce::String& info) { return true; }
		bool callFunction(const JSFuncParams& args, std::vector<double>& result_vector, juce::String& info) { return true; }
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


namespace Akasha {
	class JSEngine {
	public:
		JSEngine() {
			v8::V8::InitializeICUDefaultLocation(".");
			v8::V8::InitializeExternalStartupData(".");
			platform = v8::platform::NewDefaultPlatform();
			v8::V8::InitializePlatform(platform.get());
			v8::V8::Initialize();
			create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
			isolate = v8::Isolate::New(create_params);

			cached_context_list.resize(num_cached_contexts);
			cached_global_list.resize(num_cached_contexts);

		}

		~JSEngine() {
			context.Reset();
			function.Reset();
			for (auto& cached_context : cached_context_list) {
				cached_context.Reset();
			}
			for (auto& cached_global : cached_global_list) {
				cached_global.Reset();
			}
			isolate->Dispose();
			v8::V8::Dispose();
			v8::V8::DisposePlatform();
			delete create_params.array_buffer_allocator;
		}

		bool loadFunction(const std::string& source_code, juce::String& info) {
			v8::Isolate::Scope isolate_scope(isolate);
			v8::HandleScope handle_scope(isolate);
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

			function.Reset(isolate, func_value.As<v8::Function>());

			// Cache the context and global object as persistent handles
			for (int i = 0; i < num_cached_contexts; i++) {
				cached_context_list[i].Reset(isolate, context.Get(isolate));
				cached_global_list[i].Reset(isolate, cached_context_list[i].Get(isolate)->Global());
			}
			// cached_context.Reset(isolate, context.Get(isolate));
			// cached_global.Reset(isolate, cached_context.Get(isolate)->Global());
			function_ready = true;
			return true;
		}

		bool callFunction(const JSFuncParams& args, std::vector<double>& result_vector, juce::String& info, int voiceId = 0) {
			// if there is a problem (return false), the info will be filled with the error message.
			voiceId = voiceId % num_cached_contexts;
			v8::Global<v8::Context>& cached_context = cached_context_list[voiceId];
			v8::Global<v8::Object>& cached_global = cached_global_list[voiceId];

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
			v8::Local<v8::Object> js_args = convertToJSObject(args);

			// Prepare arguments for the function (only one argument, the array)
			v8::Local<v8::Value> js_func_args[1] = { js_args };
			v8::Local<v8::Value> result;
			
			if (!function.Get(isolate)->Call(cached_context.Get(isolate), cached_global.Get(isolate), 1, js_func_args).ToLocal(&result)) {
				v8::String::Utf8Value error(isolate, try_catch.Exception());
				std::string error_str(*error);
				info = juce::String(error_str)+"\n";
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
			}else if (result->IsNumber()) {
				for (uint32_t i = 0; i < result_vector.size(); i++) {
					result_vector[i] = (result->NumberValue(cached_context.Get(isolate)).ToChecked());
				}
			}else {
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
		v8::Local<v8::Object> convertToJSObject(const JSFuncParams& params) {
			v8::Local<v8::Object> jsObject = v8::Object::New(isolate);
			v8::Local<v8::Array> jsMacros = v8::Array::New(isolate, params.macros.size());
			for (size_t i = 0; i < params.macros.size(); ++i) {
				jsMacros->Set(isolate->GetCurrentContext(), i, v8::Number::New(isolate, params.macros[i])).Check();
			}
			jsObject->Set(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "macros").ToLocalChecked(), jsMacros).Check();
			jsObject->Set(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "tempo").ToLocalChecked(), v8::Number::New(isolate, params.tempo)).Check();
			jsObject->Set(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "beat").ToLocalChecked(), v8::Number::New(isolate, params.beat)).Check();
			jsObject->Set(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "sampleRate").ToLocalChecked(), v8::Number::New(isolate, params.sampleRate)).Check();
			jsObject->Set(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "time").ToLocalChecked(), v8::Number::New(isolate, params.time)).Check();
			jsObject->Set(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "note").ToLocalChecked(), v8::Number::New(isolate, params.note)).Check();
			jsObject->Set(isolate->GetCurrentContext(), v8::String::NewFromUtf8(isolate, "velocity").ToLocalChecked(), v8::Number::New(isolate, params.velocity)).Check();
			return jsObject;
		}

		v8::Isolate::CreateParams create_params;
		v8::Isolate* isolate;
		std::unique_ptr<v8::Platform> platform;
		v8::Global<v8::Context> context;
		v8::Global<v8::Function> function;

		// Cache as persistent handles for memory safety
		std::vector<v8::Global<v8::Context>> cached_context_list;
		std::vector<v8::Global<v8::Object>> cached_global_list;

		const int num_cached_contexts = 16;

		bool function_ready = false;
	};
}
#endif
