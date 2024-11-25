// javascripts here
function midiNoteToFreq(note){
    if (note < 0 || note > 127) {
        return 0;
    }
    const A4 = 440;
    const A4_note = 69;
    return A4 * Math.pow(2, (note - A4_note)/12);
}

var dcBlockerLastInput = 0;
var dcBlockerLastOutput = 0;
function dcBlocker(input, alpha=0.995){
    const output = input - dcBlockerLastInput + alpha * dcBlockerLastOutput;
    dcBlockerLastInput = input;
    dcBlockerLastOutput = output;
    return output;
}

function decay(t) {
    const length = 2.0;
    const rate = 2.0;
    return Math.max(length * Math.exp(-rate * t), 0);
}

function square(phase, pulse){
    return phase < pulse? -1:1;
}

var phase = 0.0

function main(args){
    var [m0,m1,m2,m3,m4,m5,m6,m7, tempo, beat, sampleRate, bufferLen, bufferPos, 
        time, note, velocity, justPressed, justReleased
        ] = args;
    // calc freq
    var freq = midiNoteToFreq(note);
    phase += freq / sampleRate;
    phase %= 1.0;

    var output = square(phase, m1) * 0.5;
    output = dcBlocker(output);
    return output * decay(time) * m0;
}
