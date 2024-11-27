#pragma once

#include <JuceHeader.h>

namespace Akasha {
	struct JSFuncParams {
		std::array<std::atomic<float>*, 8> macros;
		double tempo = 0.0;
		double beat = 0.0;
		double sampleRate = 0.0;

		int bufferLen;
		int bufferPos;

		double time;
		int note;
		float velocity;
		bool justPressed;
		bool justReleased;
	};

	struct JSMainWrapperParams {
		std::array<std::atomic<float>*, 8> macros;
		int numSamples;
		int numChannels;
		double sampleRate;
		double tempo;
		double beat;
		bool justPressed;
		bool justReleased;

		int note;
		float velocity;
		std::vector<std::vector<float*>> outputBuffer; // numChannels -> numSamples
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
#include "assets/mainWrapper.h"

namespace Akasha {

	class V8GlobalManager {
	public:
		static V8GlobalManager& getInstance();
		v8::Platform* getPlatform();

	private:
		V8GlobalManager();
		~V8GlobalManager();
		V8GlobalManager(const V8GlobalManager&) = delete;
		V8GlobalManager& operator=(const V8GlobalManager&) = delete;
		std::unique_ptr<v8::Platform> platform;
	};

	class JSEngine {
	public:
		JSEngine();
		~JSEngine();

		bool loadFunction(const std::string& source_code, juce::String& info);
		bool callFunction(const JSFuncParams& args, std::vector<double>& result_vector, juce::String& info, int voiceId = 0);
		bool isFunctionReady() const;

	private:
		struct Cache {
			v8::Global<v8::Context> context;
			v8::Global<v8::Object> globalObject;
			v8::Global<v8::Function> function;
			v8::Global<v8::ArrayBuffer> arrayBufferArguments;
			v8::Global<v8::Float64Array> float64ArrayArguments;
			std::vector<v8::Global<v8::ArrayBuffer>> channelBuffers;
			v8::Global<v8::Array> arrayAudioBuffer;
			v8::Global<v8::ArrayBuffer> arrayBufferArgs1;
			v8::Global<v8::ArrayBuffer> arrayBufferArgs2;
			Cache() = default;
			Cache(const Cache&) = delete;
			Cache& operator=(const Cache&) = delete;
			Cache(Cache&&) = default;
			Cache& operator=(Cache&&) = default;
			void Reset(v8::Isolate* isolate);
		};

		v8::Local<v8::Float64Array> const prepareArguments(const JSFuncParams& params, int voiceId);
		void prepareMainWrapperArguments(const JSMainWrapperParams& params, int voiceId);
		void allocateArrayAudioBuffer(const JSMainWrapperParams& params, Cache& cache);
		bool compileAndRunScript(const std::string& scriptSource, v8::Local<v8::Context> context, juce::String& info);
		v8::Local<v8::Function> getGlobalFunction(const std::string& functionName, v8::Local<v8::Context> context, juce::String& info);

		v8::Isolate::CreateParams create_params;
		v8::Isolate* isolate;
		std::unique_ptr<v8::Platform> platform;
		std::vector<Cache> cachedList;

		const int num_cached_contexts = 16;
		const int array_buffer_len = 20;
		const int macro_len = 8;
		bool function_ready = false;
		mutable std::mutex mutex;
	};
}
#endif