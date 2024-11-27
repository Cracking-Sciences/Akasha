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
"    return {\r\n"
"        mainWrapper: function (args1, args2, buffer) {\r\n"
"            // args1: Array of macros\r\n"
"            // args2: Array of other parameters\r\n"
"            // buffer: A fixed 2D floating-point array representing the audio buffer for all channels\r\n"
"\r\n"
"            var currentMacros = args1;\r\n"
"            var [\r\n"
"                numSamples, numChannels, sampleRate, tempo, beat, justPressed, justReleased, note, velocity\r\n"
"            ] = args2;\r\n"
"\r\n"
"            var interpolatedMacros;\r\n"
"            var timeSinceReleased;\r\n"
"\r\n"
"            if (!prevMacros) {\r\n"
"                prevMacros = currentMacros.slice();\r\n"
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
"            for (var i = 0; i < numSamples; i++) {\r\n"
"                // temp var\r\n"
"                var t = (i + 1)/ (numSamples);\r\n"
"                interpolatedMacros = currentMacros.map((current, index) => \r\n"
"                    t * current + (1.0 - t) * prevMacros[index]\r\n"
"                );\r\n"
"                timeSinceReleased = time - timeAtReleased;\r\n"
"\r\n"
"                // Call main and handle the result\r\n"
"                var result = main([\r\n"
"                    ...interpolatedMacros, tempo, liveBeat, sampleRate, numSamples, \r\n"
"                    i, time, note, velocity, justPressed, justReleased, timeSinceReleased\r\n"
"                ]);\r\n"
"\r\n"
"                // Fill the buffer directly\r\n"
"                if (typeof result === \"number\") {\r\n"
"                    for (var channel = 0; channel < numChannels; channel++) {\r\n"
"                        buffer[channel][i] = result;\r\n"
"                    }\r\n"
"                } else if (Array.isArray(result)) {\r\n"
"                    for (var channel = 0; channel < numChannels; channel++) {\r\n"
"                        if (channel < result.length) {\r\n"
"                            buffer[channel][i] = result[channel];\r\n"
"                        } else {\r\n"
"                            buffer[channel][i] = result[result.length - 1];\r\n"
"                        }\r\n"
"                    }\r\n"
"                }\r\n"
"\r\n"
"                // Update\r\n"
"                time += 1.0 / sampleRate;\r\n"
"                liveBeat += tempo / 60.0 / sampleRate;\r\n"
"                justPressed = false;\r\n"
"                justReleased = false;\r\n"
"            }\r\n"
"            prevMacros = currentMacros.slice();\r\n"
"        },\r\n"
"        setJustReleased: setJustReleased\r\n"
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
        case 0xfe940f0e:  numBytes = 2898; return mainWrapper_js;
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
