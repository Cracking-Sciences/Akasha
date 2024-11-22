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

function main(args){
    var macros = args.macros;
    var tempo = args.tempo;
    var sampleRate = args.sampleRate;
    var time = args.time;
    var beat = args.beat;
    var note = args.note;
    var velocity = args.velocity;
    // calc freq
    var freq = midiNoteToFreq(note) / 4;
    // unison
    const unisonVoices = 2;
    var detuneAmount = macros[3]*0.1;

    var harmonics = 1024 * exponentialMapping(macros[1]);
    var unisonOutput = 0;
    var rand = new Array(unisonVoices);
    for (var j = 0; j < unisonVoices; j++){
        rand[j] = (j-0.5)*detuneAmount;
    }

    for (var j = 0; j < unisonVoices; j++){
        var phaseOffset = rand[j]*Math.PI;
        var voiceOutput = 0;
        for (var i = 0; i < harmonics; i++) {
            var basic_freq = freq * (i + 1) * (1 + rand[j])
            voiceOutput += sinUnit(
                (mix(macros[2], freq_shift(basic_freq), basic_freq)
                * time), 
                1 / (i + 1), 
            phaseOffset);
        }
        unisonOutput += voiceOutput;
    }

    return unisonOutput*macros[0];
}

