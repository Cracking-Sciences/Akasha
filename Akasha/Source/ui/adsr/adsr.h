#pragma once
#include <JuceHeader.h>

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

	class ADSRWindow : public juce::Component, private juce::ChangeListener, public juce::ChangeBroadcaster {
	public:
		ADSRWindow();
		~ADSRWindow();
		enum Stage {
			Attack = 0,
			Hold = 1,
			Decay = 2,
			Release = 3
		};
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

		void getValue(float timeNow, bool isRelease, float timeAtRelease, float& v, bool& stop);

	private:
		float attack = 0.1f;
		float attackCurvature = 0.0f;
		float hold = 0.1f;
		float decay = 0.1f;
		float deacyCurvature = 0.0f;
		float sustain = 0.1f; // 0.0 to 1.0
		float release = 0.1f;
		float releaseCurvature = 0.0f;

		int numPoints = 100;
		float minCurvature = -20.0f;
		float maxCurvature = 20.0f;
		std::unique_ptr<std::vector<float>> envelope;
		std::unique_ptr<std::vector<SingleCurve>> curves;

		Stage currentHoldStage = Attack;
		float currentCurvature = 0.0f;
		ActionType actionType = None;
		void changeListenerCallback(ChangeBroadcaster* source) override;
		void calcPoints();
		float getPoint(float x);
		void setCurvature(float curvature, Stage stage);
		Stage getSegmentIndex(int sampleIndex);
	};

}