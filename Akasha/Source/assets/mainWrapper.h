#pragma once

const char* mainWrapperScript = R"(
function createMainWrapper() {
    // Private variables to store previous Macros and time
    var prevMacrosStatic = null;
    var time = 0.0;
    var globalJustReleased = false; // Shared flag for noteOff signals

    // Function to set justReleased flag
    function setJustReleased() {
        globalJustReleased = true;
    }

    return {
        mainWrapper: function (args1, args2) {
            // args1: Array of macros (currentMacros)
            // args2: Array of other parameters [tempo, beat, sampleRate, bufferLen, numChannels, note, velocity, justPressed, justReleased]

            if (!Array.isArray(args1) || !Array.isArray(args2)) {
                throw new Error("Invalid arguments passed to mainWrapper. Expected two arrays: args1 (macros) and args2 (parameters).");
            }

            const currentMacros = args1; // Extract macros
            const [
                tempo, beat, sampleRate, bufferLen, numChannels,
                note, velocity, justPressed, justReleased
            ] = args2; // Extract other parameters

            // Initialize prevMacrosStatic if it's the first call
            if (!prevMacrosStatic) {
                prevMacrosStatic = currentMacros.slice();
            }

            // Reset time if justPressed is true
            if (justPressed) {
                time = 0.0;
            }

            // Allocate static buffer if needed
            if (!mainWrapperStaticBuffer || mainWrapperStaticBuffer.length < bufferLen) {
                mainWrapperStaticBuffer = new Array(bufferLen);
                for (var i = 0; i < bufferLen; i++) {
                    mainWrapperStaticBuffer[i] = new Array(numChannels).fill(0);
                }
            }

            // Perform macro interpolation for each sample
            for (var i = 0; i < bufferLen; i++) {
                var t = i / (bufferLen - 1); // Interpolation factor from 0 to 1
                var interpolatedMacros = currentMacros.map((current, index) => 
                    t * current + (1.0 - t) * prevMacrosStatic[index]
                );

                // Handle global justReleased signal
                if (globalJustReleased) {
                    globalJustReleased = false; // Reset the global flag
                }

                // Call main and handle the result
                var result = main([
                    ...interpolatedMacros, tempo, beat, sampleRate, bufferLen, 
                    i, time, note, velocity, justPressed, globalJustReleased
                ]);

                // Handle the result
                if (typeof result === "number") {
                    for (var channel = 0; channel < numChannels; channel++) {
                        mainWrapperStaticBuffer[i][channel] = result;
                    }
                } else if (Array.isArray(result)) {
                    for (var channel = 0; channel < numChannels; channel++) {
                        if (channel < result.length) {
                            mainWrapperStaticBuffer[i][channel] = result[channel];
                        } else {
                            mainWrapperStaticBuffer[i][channel] = result[result.length - 1];
                        }
                    }
                }

                // Increment time per sample
                time += 1.0 / sampleRate;
                justPressed = false; // Reset after the first sample in this buffer
            }

            // Update prevMacrosStatic with currentMacros for the next call
            prevMacrosStatic = currentMacros.slice();

            return mainWrapperStaticBuffer;
        },
        setJustReleased: setJustReleased
    };
}

// Create an instance of mainWrapper with its own private state
var mainWrapperInstance = createMainWrapper();
var mainWrapper = mainWrapperInstance.mainWrapper;
var setJustReleased = mainWrapperInstance.setJustReleased;

)";