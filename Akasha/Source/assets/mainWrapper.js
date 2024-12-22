function createMainWrapper() {
    // Private variables to store previous Macros and time
    var prevMacros = null;
    var time = 0.0;
    var liveBeat = 0.0;
    var interpolatedMacros = [];
    var sharedResultArray = []; // Shared result array for single-channel output

    return {
        mainWrapper: function (args1, args2, buffer) {
            var currentMacros = args1;
            let [
                numSamples, numChannels, sampleRate, tempo, beat, justPressed, voiceId, note, velocity
            ] = args2;

            // Ensure interpolatedMacros matches the size of currentMacros
            if (!prevMacros) {
                prevMacros = currentMacros.slice(); // Initialize only once
                interpolatedMacros = new Array(currentMacros.length).fill(0);
            }

            if (justPressed) {
                time = 0.0;
            }

            liveBeat = beat;

            // Precompute constant factor
            const tempoFactor = tempo / 60.0 / sampleRate;
            const delta = 1.0 / sampleRate;

            for (var i = 0; i < numSamples; i++) {
                // Precompute interpolation factor
                const t = (i + 1) / numSamples;

                // Interpolate macros
                for (var j = 0; j < currentMacros.length; j++) {
                    interpolatedMacros[j] = t * currentMacros[j] + (1.0 - t) * prevMacros[j];
                }

                // Calculate time since release

                // Call main and handle the result
                let result = main([
                    ...interpolatedMacros, tempo, liveBeat, sampleRate, numSamples,
                    i, time, note, velocity, justPressed
                ]);

                // Normalize single number result to array
                if (typeof result === "number") {
                    sharedResultArray[0] = result;
                    result = sharedResultArray;
                }

                // Fill the buffer directly
                for (var channel = 0; channel < numChannels; channel++) {
                    const channelBuffer = buffer[channel]; // Cache channel reference
                    channelBuffer[i] = channel < result.length ? result[channel] : result[result.length - 1];
                }

                // Update time and beat
                time += delta;
                liveBeat += tempoFactor;
                justPressed = false;
            }

            // Update previous macros
            for (var j = 0; j < currentMacros.length; j++) {
                prevMacros[j] = currentMacros[j];
            }
        }
    };
}

// Create an instance of mainWrapper with its own private state
var mainWrapperInstance = createMainWrapper();
var mainWrapper = mainWrapperInstance.mainWrapper;
