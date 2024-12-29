#pragma once
#include <JuceHeader.h>

namespace Akasha {
	inline juce::File getDependencyPath() {
		auto baseDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
		auto crackingSciencesDir = baseDir.getChildFile("CrackingSciences");
		if (!crackingSciencesDir.exists())
			crackingSciencesDir.createDirectory();
		auto akashaDir = crackingSciencesDir.getChildFile("Akasha");
		if (!akashaDir.exists())
			akashaDir.createDirectory();
		return akashaDir;
	}
}