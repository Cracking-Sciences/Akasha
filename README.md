# Akasha by Cracking Sciences

The ultimate answer to audio tools.

![snapshot](/Assets/snapshot_v0.1.0.png)
**v0.1.0**
## Overview

Focus on the dsp kernel to design your sounds (or patterns, or even tracks) directly in your DAW. Write down your idea in the handy javascrpit.
Akasha embeds [V8](https://v8.dev/) and offers high performance javascript execution.

The code editor is driven by JUCE WebBrowserComponent and holds a [CodeMirror 5](https://codemirror.net/5/) instance.

## How to Use

### Install
Open the standalone version directly or open the vst3 version in your DAW.


### Code Guide
The entry class is `Voice`. Up to 16 `Voice` instances handle polyphony notes, each with its own isolated members.

The entry function of `Voice` is `main`, called for each sample point whenever a key is holding.

**input args:**
|Default Name|Explanation|
|---|---|
|m0~m7|Macro knobs' values in float|
|tempo|Current tempo in bpm provided by your DAW |
|beat|Current beat (the xxx th beat in float) provided by your DAW |
|sampleRate|Sample Rate in float|
|bufferLen|Advanced feature. Ignore it if you don't know.|
|bufferPos|Advanced feature. Ignore it if you don't know.|
|time|The time elapsed since your have pressed down the note|
|note|The midi ID of the note you just pressed. 0~127|
|velocity|The velocity of the note you just pressed. 0.0~1.0 in float|
|justPressed|Whether this main call is the trigger frame of the note. 0 or 1|

**return:** 
- Return a float value between -1~1 for mono sound.
- Return an array of float value between -1~1 for stereo or multi-channel sound.

`main` is called for each sample point at the sample rate. Other functions, classes, and global variables stay there, memorizing their states.

Check `Presets/tutorials/*` for more tricks.

### Interact


To compile your code, defocus the editor.
Once your code is compiled, up to 16 notes can be pressed simutaneously, each with its own `Voice` instance.

Macro Knobs can be automatically controlled by your DAW.

### TODO

- 🔥Drawable LFO functions and their UI. Can be used as oscilator shape, modulation source, etc. 
- 🔥Support note-off sound by adsr envelope.
- 🔥MPE.
- 🔥Legato and glide.
- 🤓Inspector functions and their UI to track variables or arrays. 
- (DONE!✅)🤔Oversampling .