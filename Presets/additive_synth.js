// javascripts here
const { sin, cos, pow, tan, floor, random } = Math;
function int(a){
    return floor(a);
}

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

function sinUnit(freq, amp, phase) {
    return amp * sin(2 * Math.PI * freq + phase);
}

function logarithmicMapping(x, c = 5) {
    return Math.log(1 + c * x) / Math.log(1 + c);
}

function exponentialMapping(x, k = 5) {
    return (Math.exp(k * x) - 1) / (Math.exp(k) - 1);
}

function mix(ratio, a, b){
    return ratio*a + (1-ratio)*b;
}

function freq_shift(freq){
    return exponentialMapping((freq / 20000), 1)*freq;
}

var phase = 0.0

function main(args){
    var [m0,m1,m2,m3,m4,m5,m6,m7, tempo, beat, sampleRate, bufferLen, bufferPos, 
        time, note, velocity, justPressed, justReleased
        ] = args;

    var freq = midiNoteToFreq(note) / 4;

    var harmonics = 512 * exponentialMapping(m1);
    var output = 0;
    for (var i = 0; i < harmonics; i++) {
        var basic_freq = freq * (i + 1)
        output += sinUnit(
            (mix(m2, freq_shift(basic_freq), basic_freq)
            * time), 
            1 / (i + 1), 
        0);
    }

    return output*m0;
}

