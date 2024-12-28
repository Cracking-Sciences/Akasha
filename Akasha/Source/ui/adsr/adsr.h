#pragma once
#include <JuceHeader.h>
#include "../slider/slider.h"

namespace Akasha {
	class SingleCurve {
	public:
		SingleCurve();
		~SingleCurve();
		float getPoint(float x);
		float startingValue = 0.0f;
		float endingValue = 1.0f;
		float curvature = 0.0f;
		float curveLength = 1.0f;
	};
	class ADSRWindow;

	class ADSRKernel : private juce::AudioProcessorValueTreeState::Listener{
		// owned by audio processor 
	public:
		ADSRKernel();
		~ADSRKernel();
		enum Stage {
			Attack = 0,
			Hold = 1,
			Decay = 2,
			Release = 3
		};
		
		float getTarget(int target);

		void setTarget(int target, float v);

		// called for drawing
		void calcPoints();
		// called for drawing
		float getPoint(float x);
		// called when use it for actual use (audio volume)
		void getValue(float timeNow, bool isRelease, float timeAtRelease, float& v, bool& stop);
		// called for vts register at processor
		void setVTS(juce::AudioProcessorValueTreeState* vts, juce::String idToUse, int target);

		const int numPoints = 1024;
		const float minCurvature = -20.0f;
		const float maxCurvature = 20.0f;

		std::unique_ptr<std::vector<float>> envelope;
		std::unique_ptr<std::vector<SingleCurve>> curves;

	private:
		// in total 8 vts targets
		// 0  attack = 0.1f;
		// 1  attackCurvature = 0.0f;
		// 2  hold = 0.1f;
		// 3  decay = 0.1f;
		// 4  decacyCurvature = 0.0f;
		// 5  sustain = 0.1f; // 0.0 to 1.0
		// 6  release = 0.1f;
		// 7  releaseCurvature = 0.0f;
		float targets[8] = {
			0.1f,0.0f,0.1f,0.1f,0.0f,1.0f,0.1f,0.0f
		};

		void parameterChanged(const juce::String& parameterID, float newValue) override;
		juce::AudioProcessorValueTreeState* vts_ptr = nullptr;
		std::vector<juce::String> idToUseVector;
	};

	class ADSRWindow : public juce::Component, private juce::ChangeListener, public juce::ChangeBroadcaster {
	public:
		ADSRWindow(ADSRKernel& adsrKernel);
		~ADSRWindow();
		enum ActionType {
			None,
			RemoveCurvature,
			DragCurvature
		};
		void mouseDown(const juce::MouseEvent&) override;
		void mouseDrag(const juce::MouseEvent&) override;
		void paint(juce::Graphics& g) override;
		void resized() override;

		void setAttack(float v);
		void setHold(float v);
		void setDecay(float v);
		void setSustain(float v);
		void setRelease(float v);

	private:
		ADSRKernel& adsrKernel;
		ADSRKernel::Stage currentHoldStage = ADSRKernel::Stage::Attack;
		float currentCurvature = 0.0f;
		ActionType actionType = None;
		void changeListenerCallback(ChangeBroadcaster* source) override;
		void setCurvature(float curvature, ADSRKernel::Stage stage);
		ADSRKernel::Stage getSegmentIndex(int sampleIndex);
	};

	class ADSRWidget : public juce::Component {
	public:
		ADSRWidget(ADSRKernel& adsrKernel);
		~ADSRWidget();
		void resized() override;
		void paint(juce::Graphics& g) override;
	private:
		ADSRKernel& adsrKernel;
		std::unique_ptr<ADSRWindow> adsrWindow;
		SliderWithLabel attackSlider{ "Attack" };
		SliderWithLabel holdSlider{ "Hold" };
		SliderWithLabel decaySlider{ "Decay" };
		SliderWithLabel sustainSlider{ "Sustain" };
		SliderWithLabel releaseSlider{ "Release" };
		float sliderKnobRatio = 2.0f;
		float sliderTextWidth = 20.0f;
	};
}