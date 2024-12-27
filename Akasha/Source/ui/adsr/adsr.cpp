#include "adsr.h"

namespace Akasha {
	ADSRWindow::ADSRWindow() {
		envelope = std::make_unique<std::vector<float>>(numPoints);
		curves = std::make_unique<std::vector<SingleCurve>>(4);
		calcPoints();
		setInterceptsMouseClicks(true, true);
		addChangeListener(this);
	}

	ADSRWindow::~ADSRWindow() {
	}

	void ADSRWindow::calcPoints() {
		(*curves)[Attack].startingValue = 0.0f;
		(*curves)[Attack].endingValue = 1.0f;
		(*curves)[Attack].curvature = attackCurvature;
		(*curves)[Attack].curveLength = attack;
		(*curves)[Hold].startingValue = 1.0f;
		(*curves)[Hold].endingValue = 1.0f;
		(*curves)[Hold].curvature = 0.0f;
		(*curves)[Hold].curveLength = hold;
		(*curves)[Decay].startingValue = 1.0f;
		(*curves)[Decay].endingValue = sustain;
		(*curves)[Decay].curvature = deacyCurvature;
		(*curves)[Decay].curveLength = decay;
		(*curves)[Release].startingValue = sustain;
		(*curves)[Release].endingValue = 0.0f;
		(*curves)[Release].curvature = releaseCurvature;
		(*curves)[Release].curveLength = release;
		float totalLength = attack + hold + decay + release;
		for (int i = 0; i < numPoints; i++) {
			float x = (float)i / (float)numPoints * totalLength;
			if (x < attack) {
				(*envelope)[i] = (*curves)[Attack].getPoint(x);
			}
			else if (x < attack + hold) {
				(*envelope)[i] = (*curves)[Hold].getPoint(x - attack);
			}
			else if (x < attack + hold + decay) {
				(*envelope)[i] = (*curves)[Decay].getPoint(x - attack - hold);
			}
			else {
				(*envelope)[i] = (*curves)[Release].getPoint(x - attack - hold - decay);
			}
		}
		repaint();
	}

	void ADSRWindow::changeListenerCallback(juce::ChangeBroadcaster* source) {
		if (source == this) {
			calcPoints();
		}
	}

	void ADSRWindow::mouseDown(const juce::MouseEvent& event) {
		int x = event.x;
		int y = event.y;
		juce::Rectangle<int> screenBounds = getLocalBounds();
		int screenBoundsWidth = screenBounds.getWidth();
		int screenBoundsHeight = screenBounds.getHeight();
		int sampleIndex = x * numPoints / screenBoundsWidth;
		if (sampleIndex < 0) {
			sampleIndex = 0;
		}
		if (sampleIndex >= numPoints) {
			sampleIndex = numPoints - 1;
		}
		currentHoldStage = getSegmentIndex(sampleIndex);
		currentCurvature = (*curves)[currentHoldStage].curvature;
		if (event.getNumberOfClicks() > 1) {
			actionType = RemoveCurvature;
			setCurvature(0.0f, currentHoldStage);
			sendChangeMessage();
		}
		else {
			actionType = DragCurvature;
		}
	}

	void ADSRWindow::mouseDrag(const juce::MouseEvent& event) {
		int x = event.x;
		int y = event.y;
		int yDistance = event.getDistanceFromDragStartY();
		float fx = (float)x / (float)getWidth();
		float fdy = (float)yDistance / (float)getHeight();
		if (currentHoldStage == Decay || currentHoldStage == Release) {
			fdy = -fdy;
		}
		float curvature = currentCurvature + fdy * 5;
		switch (actionType) {
		case RemoveCurvature:
			break;
		case DragCurvature:
			if (curvature < minCurvature) curvature = minCurvature;
			if (curvature > maxCurvature) curvature = maxCurvature;
			setCurvature(curvature, currentHoldStage);
			sendChangeMessage();
			break;
		default:
			return;
		}
	}

	void ADSRWindow::paint(juce::Graphics& g) {
		g.setColour(juce::Colours::black);
		g.fillRect(getLocalBounds());
		int height = getHeight();
		int width = getWidth();
		juce::Path p;
		p.startNewSubPath(0, height);
		float totalLength = attack + hold + decay + release;
		for (int x = 0; x < width; x++) {
			float time = (float)x / (float)width * totalLength;
			float fy = getPoint(time);
			int y = height * (1.0f - fy);
			p.lineTo(x, y);
		}
		g.setColour(juce::Colours::white);
		g.strokePath(p, juce::PathStrokeType(2.0f));
	}

	void ADSRWindow::resized() {
		calcPoints();
	}

	ADSRWindow::Stage ADSRWindow::getSegmentIndex(int sampleIndex) {
		float totalLength = attack + hold + decay + release;
		float x = (float)sampleIndex / (float)numPoints * totalLength;
		if (x < attack) {
			return Attack;
		}
		else if (x < attack + hold) {
			return Hold;
		}
		else if (x < attack + hold + decay) {
			return Decay;
		}
		else {
			return Release;
		}
	}

	void ADSRWindow::setAttack(float v) {
		if (v < 0.0) {
			v = 0.0;
		}
		attack = v;
		sendChangeMessage();
	}

	void ADSRWindow::setHold(float v) {
		if (v < 0.0) {
			v = 0.0;
		}
		hold = v;
		sendChangeMessage();
	}

	void ADSRWindow::setDecay(float v) {
		if (v < 0.0) {
			v = 0.0;
		}
		decay = v;
		sendChangeMessage();
	}

	void ADSRWindow::setSustain(float v) {
		if (v > 1.0f) {
			v = 1.0f;
		}
		if (v < 0.0f) {
			v = 0.0f;
		}
		sustain = v;
		sendChangeMessage();
	}

	void ADSRWindow::setRelease(float v) {
		if (v < 0.0) {
			v = 0.0;
		}
		release = v;
		sendChangeMessage();
	}

	float ADSRWindow::getPoint(float x) {
		float totalLength = attack + hold + decay + release;
		float fpoint = x / totalLength * numPoints;
		if (fpoint >= numPoints) {
			return 0.0f;
		}
		if (fpoint < 0.0f) {
			return 0.0f;
		}
		int point = (int)(fpoint);
		float fraction = fpoint - point;
		float y1 = (*envelope)[point];
		float y2 = 0;
		if (point < numPoints - 1) {
			y2 = (*envelope)[point + 1];
		}
		return y1 + (y2 - y1) * fraction;
	}

	void ADSRWindow::getValue(float timeNow, bool isRelease, float timeAtRelease, float& v, bool& stop) {
		float totalLength = attack + hold + decay + release;
		float sustainEnd = attack + hold + decay;
		float pressedX;
		stop = false;
		if (!isRelease) {
			pressedX = timeNow < sustainEnd ? timeNow : sustainEnd;
		}
		else {
			pressedX = timeAtRelease < sustainEnd ? timeAtRelease : sustainEnd;
		}

		float pressedV = getPoint(pressedX);

		if (!isRelease) {
			v = pressedV;
		}
		else {
			float timeSinceRelease = timeNow - timeAtRelease;
			if (timeSinceRelease > release) {
				v = 0.0f;
				stop = true;
			}
			else {
				float x = sustainEnd + timeSinceRelease;
				float releaseV = getPoint(x);
				float ratio = releaseV / sustain;
				v = pressedV * ratio;
			}
		}
	}

	void ADSRWindow::setCurvature(float curvature, Stage stage) {
		switch (stage) {
		case Attack:
			attackCurvature = curvature;
			break;
		case Decay:
			deacyCurvature = curvature;
			break;
		case Release:
			releaseCurvature = curvature;
			break;
		default:
			break;
		}
	}

	SingleCurve::SingleCurve() {
	}

	SingleCurve::~SingleCurve() {
	}

	float SingleCurve::getPoint(float x) {
		if (x < 0.0f) {
			return startingValue;
		}
		else if (x > curveLength) {
			return endingValue;
		}

		x = x / curveLength;

		float adjustedX = 0.0f;
		if (curvature == 0.0f) {
			adjustedX = x;
		}
		else if (curvature > 0.0f) {
			adjustedX = powf(x, 1.0f + curvature);
		}
		else {
			adjustedX = 1.0f - powf(1.0f - x, 1.0f - curvature);
		}

		return startingValue + (endingValue - startingValue) * adjustedX;
	}

	ADSRWidget::ADSRWidget() {
		attackSlider.setSliderRange(0.0, 10.0);
		attackSlider.setSliderRatio(sliderKnobRatio);
		attackSlider.setTextWidth(sliderTextWidth);
		holdSlider.setSliderRange(0.0, 10.0);
		holdSlider.setSliderRatio(sliderKnobRatio);
		holdSlider.setTextWidth(sliderTextWidth);
		decaySlider.setSliderRange(0.0, 10.0);
		decaySlider.setSliderRatio(sliderKnobRatio);
		decaySlider.setTextWidth(sliderTextWidth);
		sustainSlider.setSliderRange(0.0, 1.0);
		sustainSlider.setSliderRatio(sliderKnobRatio);
		sustainSlider.setTextWidth(sliderTextWidth);
		releaseSlider.setSliderRange(0.0, 10.0);
		releaseSlider.setSliderRatio(sliderKnobRatio);
		releaseSlider.setTextWidth(sliderTextWidth);

		attackSlider.getSlider().onValueChange = [this]() {
			float value = attackSlider.getSlider().getValue();
			adsrWindow.setAttack(value);
			};
		holdSlider.getSlider().onValueChange = [this]() {
			float value = holdSlider.getSlider().getValue();
			adsrWindow.setHold(value);
			};
		decaySlider.getSlider().onValueChange = [this]() {
			float value = decaySlider.getSlider().getValue();
			adsrWindow.setDecay(value);
			};
		sustainSlider.getSlider().onValueChange = [this]() {
			float value = sustainSlider.getSlider().getValue();
			adsrWindow.setSustain(value);
			};
		releaseSlider.getSlider().onValueChange = [this]() {
			float value = releaseSlider.getSlider().getValue();
			adsrWindow.setRelease(value);
			};

		addAndMakeVisible(attackSlider);
		addAndMakeVisible(holdSlider);
		addAndMakeVisible(decaySlider);
		addAndMakeVisible(sustainSlider);
		addAndMakeVisible(releaseSlider);
		addAndMakeVisible(adsrWindow);
	}

	ADSRWidget::~ADSRWidget() {}

	void ADSRWidget::resized() {
		juce::Rectangle<int> bounds = getLocalBounds();
		int sliderHeight = 50; 
		juce::FlexBox flexBox;
		flexBox.flexDirection = juce::FlexBox::Direction::row; 
		flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
		flexBox.alignItems = juce::FlexBox::AlignItems::stretch;
		flexBox.items.add(juce::FlexItem(attackSlider).withHeight(sliderHeight).withFlex(1));
		flexBox.items.add(juce::FlexItem(holdSlider).withHeight(sliderHeight).withFlex(1));
		flexBox.items.add(juce::FlexItem(decaySlider).withHeight(sliderHeight).withFlex(1));
		flexBox.items.add(juce::FlexItem(sustainSlider).withHeight(sliderHeight).withFlex(1));
		flexBox.items.add(juce::FlexItem(releaseSlider).withHeight(sliderHeight).withFlex(1));
		auto sliderArea = bounds.removeFromBottom(sliderHeight);
		flexBox.performLayout(sliderArea);
		adsrWindow.setBounds(bounds);
	}
}