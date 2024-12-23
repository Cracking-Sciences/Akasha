/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#include <cstring>

namespace BinaryData
{

//================== mainWrapper.js ==================
static const unsigned char temp_binary_data_0[] =
"function createMainWrapper() {\r\n"
"\r\n"
"    var sharedResultArray = []; // Shared result array for single-channel output\r\n"
"\r\n"
"    var interpolatedMacros = new Array(8).fill(0);\r\n"
"\r\n"
"    // 16 Voice instances\r\n"
"    const voices = Array.from({ length: 16 }, () => new Voice());\r\n"
"    const times = new Array(16).fill(0.0);\r\n"
"    const liveBeats = new Array(16).fill(0.0);\r\n"
"    var prevMacrosArray = Array.from({ length: 16 }, () => null);;\r\n"
" \r\n"
"    return {\r\n"
"        mainWrapper: function (args1, args2, buffer) {\r\n"
"            var currentMacros = args1;\r\n"
"            let [\r\n"
"                numSamples, numChannels, sampleRate, tempo, beat, justPressed, voiceId, note, velocity\r\n"
"            ] = args2;\r\n"
"\r\n"
"            const voiceIndex = voiceId % voices.length;\r\n"
"            const voice = voices[voiceIndex];\r\n"
"            let time = times[voiceIndex];\r\n"
"            let liveBeat = liveBeats[voiceIndex];\r\n"
"            let prevMacros = prevMacrosArray[voiceIndex];\r\n"
"\r\n"
"            // Initialize prevMacros and interpolatedMacros for the voice\r\n"
"            if (!prevMacros) {\r\n"
"                prevMacros = currentMacros.slice(); // Initialize only once\r\n"
"                prevMacrosArray[voiceIndex] = prevMacros;\r\n"
"            }\r\n"
"\r\n"
"            if (justPressed) {\r\n"
"                time = 0.0;\r\n"
"            }\r\n"
"\r\n"
"            liveBeat = beat;\r\n"
"\r\n"
"            // Precompute constant factor\r\n"
"            const tempoFactor = tempo / 60.0 / sampleRate;\r\n"
"            const delta = 1.0 / sampleRate;\r\n"
"\r\n"
"\r\n"
"            for (var i = 0; i < numSamples; i++) {\r\n"
"                // Precompute interpolation factor\r\n"
"                const t = (i + 1) / numSamples;\r\n"
"\r\n"
"                // Interpolate macros\r\n"
"                for (var j = 0; j < currentMacros.length; j++) {\r\n"
"                    interpolatedMacros[j] = t * currentMacros[j] + (1.0 - t) * prevMacros[j];\r\n"
"                }\r\n"
"\r\n"
"                // Call main and handle the result\r\n"
"                \r\n"
"                let result = voice.main({\r\n"
"                    m0: interpolatedMacros[0],\r\n"
"                    m1: interpolatedMacros[1],\r\n"
"                    m2: interpolatedMacros[2],\r\n"
"                    m3: interpolatedMacros[3],\r\n"
"                    m4: interpolatedMacros[4],\r\n"
"                    m5: interpolatedMacros[5],\r\n"
"                    m6: interpolatedMacros[6],\r\n"
"                    m7: interpolatedMacros[7],\r\n"
"                    tempo,\r\n"
"                    beat: liveBeat,\r\n"
"                    sampleRate,\r\n"
"                    numSamples,\r\n"
"                    bufferPos: i,\r\n"
"                    time,\r\n"
"                    note,\r\n"
"                    velocity,\r\n"
"                    justPressed\r\n"
"                });\r\n"
"\r\n"
"                // Normalize single number result to array\r\n"
"                if (typeof result === \"number\") {\r\n"
"                    sharedResultArray[0] = result;\r\n"
"                    result = sharedResultArray;\r\n"
"                }\r\n"
"\r\n"
"                // Fill the buffer directly\r\n"
"                for (var channel = 0; channel < numChannels; channel++) {\r\n"
"                    const channelBuffer = buffer[channel]; // Cache channel reference\r\n"
"                    channelBuffer[i] = channel < result.length ? result[channel] : result[result.length - 1];\r\n"
"                }\r\n"
"\r\n"
"                // Update time and beat\r\n"
"                time += delta;\r\n"
"                liveBeat += tempoFactor;\r\n"
"                justPressed = false;\r\n"
"            }\r\n"
"\r\n"
"            // Update previous macros\r\n"
"            for (var j = 0; j < currentMacros.length; j++) {\r\n"
"                prevMacros[j] = currentMacros[j];\r\n"
"            }\r\n"
"\r\n"
"            // Update time and beat\r\n"
"            times[voiceId % 16] = time;\r\n"
"            liveBeats[voiceId % 16] = liveBeat;\r\n"
"        }\r\n"
"    };\r\n"
"}\r\n"
"\r\n"
"// Create an instance of mainWrapper with its own private state\r\n"
"var mainWrapperInstance = createMainWrapper();\r\n"
"var mainWrapper = mainWrapperInstance.mainWrapper;\r\n";

const char* mainWrapper_js = (const char*) temp_binary_data_0;


const char* getNamedResource (const char* resourceNameUTF8, int& numBytes);
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0xfe940f0e:  numBytes = 3876; return mainWrapper_js;
        default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "mainWrapper_js"
};

const char* originalFilenames[] =
{
    "mainWrapper.js"
};

const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)
        if (strcmp (namedResourceList[i], resourceNameUTF8) == 0)
            return originalFilenames[i];

    return nullptr;
}

}
