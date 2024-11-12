#pragma once
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
        }

        ~JSEngine() {
            context.Reset();
            function.Reset();
            cached_context.Reset();
            cached_global.Reset();
            isolate->Dispose();
            v8::V8::Dispose();
            v8::V8::DisposePlatform();
            delete create_params.array_buffer_allocator;
        }

        bool loadFunction(const std::string& source_code) {
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);
            context.Reset(isolate, v8::Context::New(isolate));
            v8::Context::Scope context_scope(context.Get(isolate));

            v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, source_code.c_str()).ToLocalChecked();
            v8::Local<v8::Script> script;
            if (!v8::Script::Compile(context.Get(isolate), source).ToLocal(&script)) {
                return false;
            }
            script->Run(context.Get(isolate));

            v8::Local<v8::Value> func_value;
            if (!context.Get(isolate)->Global()->Get(context.Get(isolate), v8::String::NewFromUtf8(isolate, "js_funcion").ToLocalChecked()).ToLocal(&func_value)) {
                return false;
            }
            if (!func_value->IsFunction()) return false;
            function.Reset(isolate, func_value.As<v8::Function>());

            // Cache the context and global object as persistent handles
            cached_context.Reset(isolate, context.Get(isolate));
            cached_global.Reset(isolate, cached_context.Get(isolate)->Global());

            return true;
        }

        std::vector<double> callFunction(const std::array<double, 8>& args_array) {
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);
            v8::Context::Scope context_scope(cached_context.Get(isolate));
            v8::TryCatch try_catch(isolate);

            // Convert the C++ array to a JavaScript array
            v8::Local<v8::Array> js_args = v8::Array::New(isolate, 8);
            for (int i = 0; i < 8; ++i) {
                js_args->Set(cached_context.Get(isolate), i, v8::Number::New(isolate, args_array[i])).Check();
            }
            // Prepare arguments for the function (only one argument, the array)
            v8::Local<v8::Value> args[1] = { js_args };
            v8::Local<v8::Value> result;
            std::vector<double> result_vector;

            if (!function.Get(isolate)->Call(cached_context.Get(isolate), cached_global.Get(isolate), 1, args).ToLocal(&result)) {
                // v8::String::Utf8Value error(isolate, try_catch.Exception());
                // std::cerr << "JavaScript Error: " << *error << std::endl;
                try_catch.Reset();
                return result_vector;
            }
            if (result->IsArray()) {
                v8::Local<v8::Array> result_array = result.As<v8::Array>();
                uint32_t length = result_array->Length();
                result_vector.reserve(length);
                for (uint32_t i = 0; i < length; i++) {
                    v8::Local<v8::Value> element;
                    if (result_array->Get(cached_context.Get(isolate), i).ToLocal(&element)) {
                        if (element->IsNumber()) {
                            result_vector.push_back(element->NumberValue(cached_context.Get(isolate)).ToChecked());
                        } else {
                            result_vector.push_back(0.0); // Default to 0.0 if not a number
                        }
                    }
                }
            }
            return result_vector;
        }

    private:
        v8::Isolate::CreateParams create_params;
        v8::Isolate* isolate;
        std::unique_ptr<v8::Platform> platform;
        v8::Global<v8::Context> context;
        v8::Global<v8::Function> function;

        // Cache as persistent handles for memory safety
        v8::Global<v8::Context> cached_context;
        v8::Global<v8::Object> cached_global;
    };
}
