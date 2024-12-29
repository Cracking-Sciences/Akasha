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
	};

	struct JSMainWrapperParams {
		std::array<std::atomic<float>*, 8> macros;
		int numSamples;
		int numChannels;
		double sampleRate;
		double tempo;
		double beat;
		bool justPressed;

		int voiceId;
		int note;
		float velocity;
	};
}



#ifdef _DEBUG
namespace Akasha {
	class JSEngine {
	public:
		JSEngine() {}
		~JSEngine() {}
		bool loadFunction(const std::string& source_code, juce::String& info) { return true; }
		bool callMainWrapperFunction(const JSMainWrapperParams& args, juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples, juce::String& info, int voiceId = 0) { return true; }
		bool isFunctionReady() const { return true; }
		bool isFunctionJustReadyForVoice(int voiceId) const { return false };
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
#include "mainWrapper.h"

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
		bool callMainWrapperFunction(const JSMainWrapperParams& args, juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples, juce::String& info, int voiceId = 0);
		bool isFunctionReady() const;
		// the function is ready and hasn't been called by that voice yet.
		bool isFunctionJustReadyForVoice(int voiceId) const;

	private:
		struct Cache {
			v8::Global<v8::Context> context;
			v8::Global<v8::Object> globalObject;
			v8::Global<v8::Function> mainWrapperFunction;

			v8::Global<v8::ArrayBuffer> arrayBufferArgs1;
			v8::Global<v8::Float64Array> arrayBufferArgs1View;
			v8::Global<v8::ArrayBuffer> arrayBufferArgs2;
			v8::Global<v8::Float64Array> arrayBufferArgs2View;
			std::vector<v8::Global<v8::ArrayBuffer>> channelBuffers;
			v8::Global<v8::Array> channelBuffersView;
			Cache() = default;
			Cache(const Cache&) = delete;
			Cache& operator=(const Cache&) = delete;
			Cache(Cache&&) = default;
			Cache& operator=(Cache&&) = default;
			void Reset(v8::Isolate* isolate);
		};

		void prepareMainWrapperArguments(const JSMainWrapperParams& params, int voiceId);
		void allocateChannelBuffers(const JSMainWrapperParams& params, Cache& cache);
		bool compileAndRunScript(const std::string& scriptSource, v8::Local<v8::Context> context, juce::String& info);
		v8::Local<v8::Function> getGlobalFunction(const std::string& functionName, v8::Local<v8::Context> context, juce::String& info);
		v8::Local<v8::Function> getGlobalClass(const std::string& className, v8::Local<v8::Context> context, juce::String& info);

		v8::Isolate::CreateParams create_params;
		v8::Isolate* isolate;
		std::unique_ptr<v8::Platform> platform;
		Cache cache;

		const int num_voices = 16;
		const int infoArg_len = 11;
		const int macro_len = 8;
		bool function_ready = false;
		std::array<bool, 16> function_just_ready_for_voice;
		mutable std::mutex mutex;
	};
}
#endif