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
            sliderAttachments.clear();
            sliders.clear();
		}

        void resized() override {
            juce::FlexBox macroSlidersBox;
            juce::FlexBox row1;
			juce::FlexBox row2;
            macroSlidersBox.flexDirection = juce::FlexBox::Direction::column;
            macroSlidersBox.flexWrap = juce::FlexBox::Wrap::noWrap;
            macroSlidersBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;

			float width = getLocalBounds().getWidth();
			float height = getLocalBounds().getHeight();

            for (int i = 0; i < 4;i++) {
                row1.items.add(
                    juce::FlexItem(*sliders[i]).withMinWidth(width / 4).withMinHeight(height / 2)
                );
            }
			for (int i = 4; i < 8; i++) {
				row2.items.add(
					juce::FlexItem(*sliders[i]).withMinWidth(width / 4).withMinHeight(height / 2)
				);
			}
			macroSlidersBox.items.add(juce::FlexItem(row1).withFlex(1.0));
			macroSlidersBox.items.add(juce::FlexItem(row2).withFlex(1.0));
            macroSlidersBox.performLayout(getLocalBounds());
        }

		void setMacroText(int index, const juce::String& newText) {
			sliders[index]->setLabelText(newText);
		}

		juce::String getMacroText(int index) {
			return sliders[index]->getLabelText();
		}

        typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    private:
        juce::AudioProcessorValueTreeState& valueTreeState;
        juce::OwnedArray<Akasha::SliderWithLabel> sliders;
        juce::OwnedArray<SliderAttachment> sliderAttachments;
    };
}