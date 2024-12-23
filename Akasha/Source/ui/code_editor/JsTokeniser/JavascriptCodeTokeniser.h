/*
  ==============================================================================

    JavascriptCodeTokeniser.h
    Created: 2 Nov 2017 10:22:47pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class JavascriptTokeniser   : public juce::CodeTokeniser
{
public:
    JavascriptTokeniser() {}
    ~JavascriptTokeniser() {}
    
    static juce::CodeEditorComponent::ColourScheme getDefaultEditorColourScheme();
    juce::CodeEditorComponent::ColourScheme getDefaultColourScheme() override;
    
    int readNextToken (juce::CodeDocument::Iterator& source) override;
    
    enum TokenType
    {
        tokenType_error = 0,
        tokenType_comment,
        tokenType_keyword,
        tokenType_operator,
        tokenType_ientifier,
        tokenType_integer,
        tokenType_float,
        tokenType_string,
        tokenType_bracket,
        tokenType_punctuation
    };
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JavascriptTokeniser)
};
