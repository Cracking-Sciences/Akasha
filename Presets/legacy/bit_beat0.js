// JavaScript code here
function midiNoteToFreq(note) {
    if (note < 0 || note > 127) {
        return 0;
    }
    const A4 = 440;
    const A4_note = 69;
    return A4 * Math.pow(2, (note - A4_note) / 12);
}

var dcBlockerLastInput = 0;
var dcBlockerLastOutput = 0;
function dcBlocker(input, alpha = 0.995) {
    const output = input - dcBlockerLastInput + alpha * dcBlockerLastOutput;
    dcBlockerLastInput = input;
    dcBlockerLastOutput = output;
    return output;
}

function shift16(a, bits) {
    if (bits > 0) {
        bits = bits % 16;
        return (a << bits) % 256 + (a >> (16 - bits)) % 256;
    } else {
        bits = -bits;
        bits = bits % 16;
        return (a >> bits) % 256 + (a << (16 - bits)) % 256;
    }
}

function main(args) {
    var [m0,m1,m2,m3,m4,m5,m6,m7, tempo, beat, sampleRate, bufferLen, bufferPos, 
        time, note, velocity, justPressed, justReleased] = args;

    // Calculate frequency
    var freq = midiNoteToFreq(note);

    var t = (time * freq / 64 % 1.0) * 255 | 0;
    var output = shift16(t, 7 * m1) & shift16(t, -2) | (t*113);

    output = dcBlocker(output / 256.0 * 31 * m0 % 1.0);
    return output;
}
