/*
  ==============================================================================

    macros.h
    Created: 22 Dec 2024 11:11:44pm
    Author:  ric

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../slider/slider.h"

namespace Akasha {
    class Macros : public juce::Component {
	public:
        Macros(juce::AudioProcessorValueTreeState& vts)
            : valueTreeState(vts) {
            for (int i = 0; i < 8; ++i) {
                auto* sliderWithLabel = new Akasha::SliderWithLabel("m" + juce::String(i));
                addAndMakeVisible(sliderWithLabel);
                sliders.add(sliderWithLabel);

                auto* attachment = new SliderAttachment(
                    valueTreeState,
                    "macro" + juce::String(i),
                    sliderWithLabel->getSlider()
                );
                sliderAttachments.add(attachment);
            }
        }

		~Macros() override {
            sliders.clear();
            sliderAttachments.clear();
		}

        void resized() override {
            juce::FlexBox macroSlidersBox;
            macroSlidersBox.flexDirection = juce::FlexBox::Direction::row;
            macroSlidersBox.flexWrap = juce::FlexBox::Wrap::wrap;
            macroSlidersBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;

            for (auto* slider : sliders) {
                macroSlidersBox.items.add(
                    juce::FlexItem(*slider).withMinWidth(slider->getTextWidth()).withMinHeight(60.0f)
                );
            }

            macroSlidersBox.performLayout(getLocalBounds());
        }

		void setMacroText(int index, const juce::String& newText) {
			sliders[index]->setLabelText(newText);
		}

		juce::String getMacroText(int index) {
			return sliders[index]->getLabelText();
		}

        typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
        // typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    private:
        juce::AudioProcessorValueTreeState& valueTreeState;
        juce::OwnedArray<Akasha::SliderWithLabel> sliders;
        juce::OwnedArray<SliderAttachment> sliderAttachments;
    };
}