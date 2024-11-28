/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#include <cstring>

namespace BinaryData
{

//================== mainWrapper.js ==================
static const unsigned char temp_binary_data_0[] =
"function createMainWrapper() {\r\n"
"    // Private variables to store previous Macros and time\r\n"
"    var prevMacros = null;\r\n"
"    var time = 0.0;\r\n"
"    var timeAtReleased = 0.0;\r\n"
"    var liveBeat = 0.0;\r\n"
"    var interpolatedMacros = [];\r\n"
"    var sharedResultArray = []; // Shared result array for single-channel output\r\n"
"\r\n"
"    return {\r\n"
"        mainWrapper: function (args1, args2, buffer) {\r\n"
"            var currentMacros = args1;\r\n"
"            let [\r\n"
"                numSamples, numChannels, sampleRate, tempo, beat, justPressed, justReleased, note, velocity\r\n"
"            ] = args2;\r\n"
"\r\n"
"            // Ensure interpolatedMacros matches the size of currentMacros\r\n"
"            if (!prevMacros) {\r\n"
"                prevMacros = currentMacros.slice(); // Initialize only once\r\n"
"                interpolatedMacros = new Array(currentMacros.length).fill(0);\r\n"
"            }\r\n"
"\r\n"
"            if (justPressed) {\r\n"
"                time = 0.0;\r\n"
"                timeAtReleased = 0.0;\r\n"
"            }\r\n"
"            if (justReleased) {\r\n"
"                timeAtReleased = time;\r\n"
"            }\r\n"
"\r\n"
"            liveBeat = beat;\r\n"
"\r\n"
"            // Precompute constant factor\r\n"
"            const tempoFactor = tempo / 60.0 / sampleRate;\r\n"
"            const delta = 1.0 / sampleRate;\r\n"
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
"                // Calculate time since release\r\n"
"                const timeSinceReleased = time - timeAtReleased;\r\n"
"\r\n"
"                // Call main and handle the result\r\n"
"                let result = main([\r\n"
"                    ...interpolatedMacros, tempo, liveBeat, sampleRate, numSamples,\r\n"
"                    i, time, note, velocity, justPressed, justReleased, timeSinceReleased\r\n"
"                ]);\r\n"
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
"                justReleased = false;\r\n"
"            }\r\n"
"\r\n"
"            // Update previous macros\r\n"
"            for (var j = 0; j < currentMacros.length; j++) {\r\n"
"                prevMacros[j] = currentMacros[j];\r\n"
"            }\r\n"
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
        case 0xfe940f0e:  numBytes = 3165; return mainWrapper_js;
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
