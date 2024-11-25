# Akasha by Cracking Sciences

The ultimate answer to audio tools.

![snapshot](/Assets/snapshot_v0.0.3.png)

## Overview

Focus on the dsp kernel to design your sounds (or patterns, or even tracks) directly in your DAW. Write down your idea in the handy javascrpit.

Akasha embeds [V8](https://v8.dev/) and offers high performance javascript execution.


## How to Use

### Install
Use the standalone version or open the vst3 version in your DAW.

### Use

#### Code Guide
The entry of the execution is `function main(args)`, where `args` provides all the information you need for a single note hit.

**input args**

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
|justReleased|TODO feature|

**return**

Return a float value between -1~1 for mono sound.

Return an array of float value between -1~1 for stereo or multi-channel sound.

#### Interact


To compile your code, press `shift` + `enter` or defocus the editor.

Once your code is compiled, up to 16 notes can be pressed simutaneously, each with its own code context and isolated global variables.


### TODO

- Drawable LFO functions and its UI. Can be used as oscilator shape, modulation source, etc.
- Support release sound by the setting "process window after releasing a note".
- MPE.
- Inspector functions to track variables or arrays.
