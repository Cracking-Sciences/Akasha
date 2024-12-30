#pragma once
const char* const defaultJavascriptCode = 
R"(

// sampleRate is a global constant
// generate a sine wave
function midiNoteToFreq(note) {
    if (note < 0 || note > 127) {
        return 0;
    }
    const A4 = 440;
    const A4_note = 69;
    return A4 * Math.pow(2, (note - A4_note) / 12);
}

function sin(phase){
	return Math.sin(phase*2.00*Math.PI);
}


class DCBlocker{
	constructor(cutoffFrequency = 5.0){
		this.xp = 0;
		this.yp = 0;
		this.alpha = 1 - (2 * Math.PI * cutoffFrequency) / sampleRate;
		if (this.alpha < 0) this.alpha = 0;
	}
	process(x){
		let y = x - this.xp + this.alpha * this.yp;
		this.yp = y;
		this.xp = x;
		return y;
	}
}

class Voice {
	constructor(){
		this.dcBlocker = new DCBlocker();
	}
    main({
        m0, m1, m2, m3, m4, m5, m6, m7, // macro knobs
        tempo, // bpm
        beat, // the current beat xx.yy
        numSamples, // please ignore it
        bufferPos, // please ignore it
        time, // the time elapsed since note pressed
		delta, // the time elapsed since last main call (normally = 1/sampleRate)
        note, // midi key 0~127
        velocity, // velocity 0.0~1.0
        justPressed // the note is just triggered
    }) {
        if (justPressed) {
            this.freq = midiNoteToFreq(note);
            this.phase = 0.0;
        }
        this.phase += this.freq * delta;
        this.phase %= 1.0;
        let output = sin(this.phase);
		output = this.dcBlocker.process(output);
        return output*m0;
    }
}

)";