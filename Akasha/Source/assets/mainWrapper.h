#pragma once

inline constexpr const char* mainWrapperScript = R"(
function createMainWrapper() {
    // Private variables to store previous Macros and time
    var prevMacros = null;
    var time = 0.0;
    var timeAtReleased = 0.0;
    var liveBeat = 0.0;
    return {
        mainWrapper: function (args1, args2, buffer) {
            // args1: Array of macros
            // args2: Array of other parameters
            // buffer: A fixed 2D floating-point array representing the audio buffer for all channels

            var currentMacros = args1;
            var [
                numSamples, numChannels, sampleRate, tempo, beat, justPressed, justReleased, note, velocity
            ] = args2;

            var interpolatedMacros;
            var timeSinceReleased;

            if (!prevMacros) {
                prevMacros = currentMacros.slice();
            }

            if (justPressed) {
                time = 0.0;
                timeAtReleased = 0.0;
            }
            if (justReleased) {
                timeAtReleased = time;
            }

            liveBeat = beat;

            for (var i = 0; i < numSamples; i++) {
                // temp var
                var t = (i + 1)/ (numSamples);
                interpolatedMacros = currentMacros.map((current, index) => 
                    t * current + (1.0 - t) * prevMacros[index]
                );
                timeSinceReleased = time - timeAtReleased;

                // Call main and handle the result
                var result = main([
                    ...interpolatedMacros, tempo, liveBeat, sampleRate, numSamples, 
                    i, time, note, velocity, justPressed, justReleased, timeSinceReleased
                ]);

                // Fill the buffer directly
                if (typeof result === "number") {
                    for (var channel = 0; channel < numChannels; channel++) {
                        buffer[channel][i] = result;
                    }
                } else if (Array.isArray(result)) {
                    for (var channel = 0; channel < numChannels; channel++) {
                        if (channel < result.length) {
                            buffer[channel][i] = result[channel];
                        } else {
                            buffer[channel][i] = result[result.length - 1];
                        }
                    }
                }

                // Update
                time += 1.0 / sampleRate;
                liveBeat += tempo / 60.0 / sampleRate;
                justPressed = false;
                justReleased = false;
            }
            prevMacros = currentMacros.slice();
        }
    };
}

// Create an instance of mainWrapper with its own private state
var mainWrapperInstance = createMainWrapper();
var mainWrapper = mainWrapperInstance.mainWrapper;


)";