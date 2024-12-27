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
		
		float getTarget(int target) {
			return targets[target];
		}

		void setTarget(int target, float v) {
			targets[target] = v;
			if (vts_ptr != nullptr) {
				if (auto* intParam = dynamic_cast<juce::AudioParameterFloat*>(vts_ptr->getParameter(idToUseVector[0]))) {
					intParam->setValueNotifyingHost(targets[target]);
				}
			}
		}

		int numPoints = 1024;
		float minCurvature = -20.0f;
		float maxCurvature = 20.0f;
		std::unique_ptr<std::vector<float>> envelope;
		std::unique_ptr<std::vector<SingleCurve>> curves;
		void calcPoints();
		float getPoint(float x);
		void getValue(float timeNow, bool isRelease, float timeAtRelease, float& v, bool& stop);
		void setVTS(juce::AudioProcessorValueTreeState* vts, juce::String idToUse, int target);
	private:
		// 8 vts targets
		float targets[8] = {
			0.1f,0.0f,0.1f,0.1f,0.0f,1.0f,0.1f,0.0f
		};

		// 0 float attack = 0.1f;
		// 1 float attackCurvature = 0.0f;
		// 2 float hold = 0.1f;
		// 3 float decay = 0.1f;
		// 4 float decacyCurvature = 0.0f;
		// 5 float sustain = 0.1f; // 0.0 to 1.0
		// 6 float release = 0.1f;
		// 7 float releaseCurvature = 0.0f;

		void parameterChanged(const juce::String& parameterID, float newValue) override;
		juce::AudioProcessorValueTreeState* vts_ptr = nullptr;
		ADSRWindow* adsrWindow = nullptr;
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
	private:
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