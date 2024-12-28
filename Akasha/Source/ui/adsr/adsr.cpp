#include "adsr.h"

namespace Akasha {
	ADSRKernel::ADSRKernel() {
		envelope = std::make_unique<std::vector<float>>(numPoints);
		curves = std::make_unique<std::vector<SingleCurve>>(4);
		calcPoints();
		idToUseVector.resize(8);
	}

	ADSRKernel::~ADSRKernel(){
		// if (vts_ptr != nullptr) {
		// 	for (auto& id : idToUseVector) {
		// 		vts_ptr->removeParameterListener(id, this);
		// 	}
		// }
	}

	float ADSRKernel::getTarget(int target) {
		return targets[target];
	}

	void ADSRKernel::setTarget(int target, float v) {
		if (vts_ptr != nullptr) {
			if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(vts_ptr->getParameter(idToUseVector[target]))) {
				floatParam->setValueNotifyingHost(floatParam->convertTo0to1(v));
			}
		}
	}

	void ADSRKernel::calcPoints() {
		(*curves)[Attack].startingValue = 0.0f;
		(*curves)[Attack].endingValue = 1.0f;
		(*curves)[Attack].curveLength = targets[0];
		(*curves)[Attack].curvature = targets[1];
		(*curves)[Hold].startingValue = 1.0f;
		(*curves)[Hold].endingValue = 1.0f;
		(*curves)[Hold].curvature = 0.0f;
		(*curves)[Hold].curveLength = targets[2];
		(*curves)[Decay].startingValue = 1.0f;
		(*curves)[Decay].curveLength = targets[3];
		(*curves)[Decay].curvature = targets[4];
		(*curves)[Decay].endingValue = targets[5];
		(*curves)[Release].startingValue = targets[5];
		(*curves)[Release].endingValue = 0.0f;
		(*curves)[Release].curveLength = targets[6];
		(*curves)[Release].curvature = targets[7];

		float totalLength = (*curves)[Attack].curveLength + (*curves)[Hold].curveLength + (*curves)[Decay].curveLength + (*curves)[Release].curveLength;
		for (int i = 0; i < numPoints; i++) {
			float x = (float)i / (float)numPoints * totalLength;
			if (x <= (*curves)[Attack].curveLength) {
				(*envelope)[i] = (*curves)[Attack].getPoint(x);
			}
			else if (x <= (*curves)[Attack].curveLength + (*curves)[Hold].curveLength) {
				(*envelope)[i] = (*curves)[Hold].getPoint(x - (*curves)[Attack].curveLength);
			}
			else if (x <= (*curves)[Attack].curveLength + (*curves)[Hold].curveLength + (*curves)[Decay].curveLength) {
				(*envelope)[i] = (*curves)[Decay].getPoint(x - (*curves)[Attack].curveLength - (*curves)[Hold].curveLength);
			}
			else {
				(*envelope)[i] = (*curves)[Release].getPoint(x - (*curves)[Attack].curveLength - (*curves)[Hold].curveLength - (*curves)[Decay].curveLength);
			}
		}
	}

	void ADSRKernel::getValue(float timeNow, bool isRelease, float timeAtRelease, float& v, bool& stop) {
		float sustainEnd = (*curves)[Attack].curveLength + (*curves)[Hold].curveLength + (*curves)[Decay].curveLength;
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
			if (timeSinceRelease > (*curves)[Release].curveLength) {
				v = 0.0f;
				stop = true;
			}
			else {
				float x = sustainEnd + timeSinceRelease;
				float releaseV = getPoint(x);
				float ratio = releaseV / (*curves)[Decay].endingValue;
				v = pressedV * ratio;
			}
		}
	}

	float ADSRKernel::getPoint(float x) {
		float totalLength = (*curves)[Attack].curveLength + (*curves)[Hold].curveLength + (*curves)[Decay].curveLength + (*curves)[Release].curveLength;
		if (totalLength <= 0.0f) {
			return 1.0f;
		}
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

	void ADSRKernel::setVTS(juce::AudioProcessorValueTreeState* vts, juce::String idToUse, int target) {
		if (target < 0 || target >= 8) {
			return;
		}
		if (vts_ptr != nullptr) {
			vts_ptr->removeParameterListener(idToUse, this);
		}
		vts_ptr = vts;
		idToUseVector[target] = idToUse;
		vts_ptr->addParameterListener(idToUse, this);
		if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(vts_ptr->getParameter(idToUse))) {
			targets[target] = floatParam->get();
		}
	}

	void ADSRKernel::parameterChanged(const juce::String& parameterID, float newValue) {
		int target = std::find(idToUseVector.begin(), idToUseVector.end(), parameterID) - idToUseVector.begin();
		if (target < 0 || target >= 8) {
			return;
		}
		targets[target] = newValue;
		calcPoints();
	}

	ADSRWindow::ADSRWindow(ADSRKernel& adsrKernel):
		adsrKernel(adsrKernel)
	{
		setInterceptsMouseClicks(true, true);
		addChangeListener(this);
	}

	ADSRWindow::~ADSRWindow() {
		removeChangeListener(this);
	}


	void ADSRWindow::changeListenerCallback(juce::ChangeBroadcaster* source) {
		if (source == this) {
			adsrKernel.calcPoints();
			repaint();
		}
	}

	void ADSRWindow::mouseDown(const juce::MouseEvent& event) {
		int x = event.x;
		int y = event.y;
		juce::Rectangle<int> screenBounds = getLocalBounds();
		int screenBoundsWidth = screenBounds.getWidth();
		int screenBoundsHeight = screenBounds.getHeight();
		int sampleIndex = x * adsrKernel.numPoints / screenBoundsWidth;
		if (sampleIndex < 0) {
			sampleIndex = 0;
		}
		if (sampleIndex >= adsrKernel.numPoints) {
			sampleIndex = adsrKernel.numPoints - 1;
		}
		currentHoldStage = getSegmentIndex(sampleIndex);
		currentCurvature = (*adsrKernel.curves)[currentHoldStage].curvature;
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
		if (currentHoldStage == ADSRKernel::Stage::Decay || currentHoldStage == ADSRKernel::Stage::Release) {
			fdy = -fdy;
		}
		float curvature = currentCurvature + fdy * 5;
		switch (actionType) {
		case RemoveCurvature:
			break;
		case DragCurvature:
			if (curvature < adsrKernel.minCurvature) curvature = adsrKernel.minCurvature;
			if (curvature > adsrKernel.maxCurvature) curvature = adsrKernel.maxCurvature;
			setCurvature(curvature, currentHoldStage);
			sendChangeMessage();
			break;
		default:
			return;
		}
	}

	void ADSRWindow::paint(juce::Graphics& g) {
		juce::Colour outlineColour = findColour(juce::TextEditor::outlineColourId);
		g.setColour(outlineColour);
		juce::Rectangle<int> bounds = getLocalBounds();
		g.drawLine(bounds.getRight() - 1, bounds.getY(), bounds.getRight() - 1, bounds.getBottom(), 1.0f); 
		g.drawLine(bounds.getX(), bounds.getBottom() - 1, bounds.getRight(), bounds.getBottom() - 1, 1.0f); 


		int height = getHeight();
		int width = getWidth();
		juce::Path p;
		p.startNewSubPath(0, height);
		float totalLength = adsrKernel.getTarget(0) + adsrKernel.getTarget(2) + adsrKernel.getTarget(3) + adsrKernel.getTarget(6);
		float intervalHint = 5.0f;
		if (totalLength <= 1.5f) {
			intervalHint = 0.2f;
		}
		else if (totalLength <= 3.0f) {
			intervalHint = 0.5f;
		}else if (totalLength <= 10.0f) {
			intervalHint = 1.0f;
		}
		float nextHintTime = intervalHint;
		g.setColour(outlineColour.withAlpha(0.5f));
		for (int x = 0; x < width; x++) {
			float time = (float)x / (float)width * totalLength;
			float fy = adsrKernel.getPoint(time);
			int y = height * (1.0f - fy);
			p.lineTo(x, y);
			if (time >= nextHintTime) {
				g.drawLine(x, 0, x, height, 1.0f);
				juce::String timeLabel = juce::String(nextHintTime, 1);
				int textX = x + 5;
				int textY = height - 15;
				g.drawText(timeLabel, textX, textY, 50, 15, juce::Justification::left);
				nextHintTime += intervalHint;
			}
		}
		g.setColour(outlineColour);
		g.strokePath(p, juce::PathStrokeType(2.0f));
	}

	void ADSRWindow::resized() {
		adsrKernel.calcPoints();
		repaint();
	}

	ADSRKernel::Stage ADSRWindow::getSegmentIndex(int sampleIndex) {
		float attack = adsrKernel.getTarget(0);
		float hold = adsrKernel.getTarget(2);
		float decay = adsrKernel.getTarget(3);
		float release = adsrKernel.getTarget(6);
		float totalLength = attack + hold + decay + release;
		float x = (float)sampleIndex / (float)adsrKernel.numPoints * totalLength;
		if (x < attack) {
			return ADSRKernel::Stage::Attack;
		}
		else if (x < attack + hold) {
			return ADSRKernel::Stage::Hold;
		}
		else if (x < attack + hold + decay) {
			return ADSRKernel::Stage::Decay;
		}
		else {
			return ADSRKernel::Stage::Release;
		}
	}

	void ADSRWindow::setAttack(float v) {
		if (v < 0.0) {
			v = 0.0;
		}
		adsrKernel.setTarget(0, v);
		sendChangeMessage();
	}

	void ADSRWindow::setHold(float v) {
		if (v < 0.0) {
			v = 0.0;
		}
		adsrKernel.setTarget(2, v);
		sendChangeMessage();
	}

	void ADSRWindow::setDecay(float v) {
		if (v < 0.0) {
			v = 0.0;
		}
		adsrKernel.setTarget(3, v);
		sendChangeMessage();
	}

	void ADSRWindow::setSustain(float v) {
		if (v > 1.0f) {
			v = 1.0f;
		}
		if (v < 0.0f) {
			v = 0.0f;
		}
		adsrKernel.setTarget(5, v);
		sendChangeMessage();
	}

	void ADSRWindow::setRelease(float v) {
		if (v < 0.0) {
			v = 0.0;
		}
		adsrKernel.setTarget(6, v);
		sendChangeMessage();
	}


	void ADSRWindow::setCurvature(float curvature, ADSRKernel::Stage stage) {
		switch (stage) {
		case ADSRKernel::Stage::Attack:
			adsrKernel.setTarget(1, curvature);
			break;
		case ADSRKernel::Stage::Decay:
			adsrKernel.setTarget(4, curvature);
			break;
		case ADSRKernel::Stage::Release:
			adsrKernel.setTarget(7, curvature);
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
		if (curveLength <= 0.0f) {
			return endingValue;
		}
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

	ADSRWidget::ADSRWidget(ADSRKernel& adsrKernel) :
		adsrKernel(adsrKernel),
		adsrWindow(std::make_unique<ADSRWindow>(adsrKernel)) 
	{
		attackSlider.getSlider().setRange(0.0, 10.0, 0.001);
		holdSlider.getSlider().setRange(0.0, 10.0, 0.001);
		decaySlider.getSlider().setRange(0.0, 10.0, 0.001);
		sustainSlider.getSlider().setRange(0.0, 1.0, 0.001);
		releaseSlider.getSlider().setRange(0.0, 10.0, 0.001);

		attackSlider.getLabel().setEditable(false);
		holdSlider.getLabel().setEditable(false);
		decaySlider.getLabel().setEditable(false);
		sustainSlider.getLabel().setEditable(false);
		releaseSlider.getLabel().setEditable(false);

		attackSlider.getSlider().setSkewFactor(0.25);
		holdSlider.getSlider().setSkewFactor(0.25);
		decaySlider.getSlider().setSkewFactor(0.25);
		releaseSlider.getSlider().setSkewFactor(0.25);

		attackSlider.getSlider().onValueChange = [this]() {
			float value = attackSlider.getSlider().getValue();
			adsrWindow->setAttack(value);
			};
		holdSlider.getSlider().onValueChange = [this]() {
			float value = holdSlider.getSlider().getValue();
			adsrWindow->setHold(value);
			};
		decaySlider.getSlider().onValueChange = [this]() {
			float value = decaySlider.getSlider().getValue();
			adsrWindow->setDecay(value);
			};
		sustainSlider.getSlider().onValueChange = [this]() {
			float value = sustainSlider.getSlider().getValue();
			adsrWindow->setSustain(value);
			};
		releaseSlider.getSlider().onValueChange = [this]() {
			float value = releaseSlider.getSlider().getValue();
			adsrWindow->setRelease(value);
			};
		addAndMakeVisible(attackSlider);
		addAndMakeVisible(holdSlider);
		addAndMakeVisible(decaySlider);
		addAndMakeVisible(sustainSlider);
		addAndMakeVisible(releaseSlider);
		addAndMakeVisible(adsrWindow.get());
	}

	ADSRWidget::~ADSRWidget() {}

	void ADSRWidget::paint(juce::Graphics& g) {
		// when a new preset is loaded, the sliders should be updated.
		// The processor will call editor to repaint everything.
		// So this will work.
		attackSlider.getSlider().setValue(adsrKernel.getTarget(0));
		holdSlider.getSlider().setValue(adsrKernel.getTarget(2));
		decaySlider.getSlider().setValue(adsrKernel.getTarget(3));
		sustainSlider.getSlider().setValue(adsrKernel.getTarget(5));
		releaseSlider.getSlider().setValue(adsrKernel.getTarget(6));
	}

	void ADSRWidget::resized() {
		juce::Rectangle<int> bounds = getLocalBounds();
		int sliderHeight = 80; 
		int height = bounds.getHeight();
		int width = bounds.getWidth();

		juce::FlexBox flexBox;
		flexBox.flexDirection = juce::FlexBox::Direction::row; 
		flexBox.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
		flexBox.alignItems = juce::FlexBox::AlignItems::stretch;
		flexBox.items.add(juce::FlexItem(attackSlider).withHeight(sliderHeight).withWidth(width/5));
		flexBox.items.add(juce::FlexItem(holdSlider).withHeight(sliderHeight).withWidth(width/5));
		flexBox.items.add(juce::FlexItem(decaySlider).withHeight(sliderHeight).withWidth(width/5));
		flexBox.items.add(juce::FlexItem(sustainSlider).withHeight(sliderHeight).withWidth(width/5));
		flexBox.items.add(juce::FlexItem(releaseSlider).withHeight(sliderHeight).withWidth(width/5));
		auto sliderArea = bounds.removeFromBottom(sliderHeight);
		flexBox.performLayout(sliderArea);
		adsrWindow->setBounds(bounds);
	}
}