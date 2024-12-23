#pragma once
const char* const defaultJavascriptCode = R"(// javascripts here
function midiNoteToFreq(note) {
    if (note < 0 || note > 127) {
        return 0;
    }
    const A4 = 440;
    const A4_note = 69;
    return A4 * Math.pow(2, (note - A4_note) / 12);
}

class Voice {
    constructor() {
        this.phase = 0.0;
        this.freq = 0.0;
    }

    main({
        m0, m1, m2, m3, m4, m5, m6, m7,
        tempo,
        beat,
        sampleRate,
        numSamples,
        bufferPos,
        time,
        note,
        velocity,
        justPressed
    }) {
        if (justPressed) {
            this.freq = midiNoteToFreq(note);
            this.phase = 0.0;
        }

        this.phase += this.freq / sampleRate * (1 + m1);
        this.phase %= 1.0;

        let output = Math.sin(this.phase * 2 * Math.PI);
        return output * m0;
    }
}

)";