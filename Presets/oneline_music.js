// javascripts here
const { sin, cos, pow, tan, floor } = Math;
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


function onelinePlayer(t){
    return (
// one line music here:
SS=(s,o,r,p)=>(c=s.charCodeAt((t>>r)%p),32==c?0:31&t*2**(c/12-o)),3*SS('0 0     7 7     037:<<',6,10,32)+5*(t&4096?SS('037',4,8,3)*(4096-(t&4095))>>12:0)

);
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
    // var freq = midiNoteToFreq(note);

    var t = time*10000;
    var output = onelinePlayer(t) / 256.0;

    return dcBlocker(output);
}



