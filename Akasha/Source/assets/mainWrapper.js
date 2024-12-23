function createMainWrapper() {

    var sharedResultArray = []; // Shared result array for single-channel output

    var interpolatedMacros = new Array(8).fill(0);

    // 16 Voice instances
    const voices = Array.from({ length: 16 }, () => new Voice());
    const times = new Array(16).fill(0.0);
    const liveBeats = new Array(16).fill(0.0);
    var prevMacrosArray = Array.from({ length: 16 }, () => null);;
 
    return {
        mainWrapper: function (args1, args2, buffer) {
            var currentMacros = args1;
            let [
                numSamples, numChannels, sampleRate, tempo, beat, justPressed, voiceId, note, velocity
            ] = args2;

            const voiceIndex = voiceId % voices.length;
            const voice = voices[voiceIndex];
            let time = times[voiceIndex];
            let liveBeat = liveBeats[voiceIndex];
            let prevMacros = prevMacrosArray[voiceIndex];

            // Initialize prevMacros and interpolatedMacros for the voice
            if (!prevMacros) {
                prevMacros = currentMacros.slice(); // Initialize only once
                prevMacrosArray[voiceIndex] = prevMacros;
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

                // Call main and handle the result
                
                let result = voice.main({
                    m0: interpolatedMacros[0],
                    m1: interpolatedMacros[1],
                    m2: interpolatedMacros[2],
                    m3: interpolatedMacros[3],
                    m4: interpolatedMacros[4],
                    m5: interpolatedMacros[5],
                    m6: interpolatedMacros[6],
                    m7: interpolatedMacros[7],
                    tempo,
                    beat: liveBeat,
                    sampleRate,
                    numSamples,
                    bufferPos: i,
                    time,
                    note,
                    velocity,
                    justPressed
                });

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

            // Update time and beat
            times[voiceId % 16] = time;
            liveBeats[voiceId % 16] = liveBeat;
        }
    };
}

// Create an instance of mainWrapper with its own private state
var mainWrapperInstance = createMainWrapper();
var mainWrapper = mainWrapperInstance.mainWrapper;
