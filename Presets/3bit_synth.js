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

function shift8(a, bits) {
    if (bits > 0) {
        bits = bits % 8;
        return (a << bits) % 256 + (a >> (8 - bits)) % 256;
    } else {
        bits = -bits;
        bits = bits % 8;
        return (a >> bits) % 256 + (a << (8 - bits)) % 256;
    }
}

const ternary5 = 243;

function shift5Ternary(a, bits) {
    // ternary
    // bits = Math.floor(bits);
    if (bits > 0) {
        bits = bits % 5;
        return (a * 3 ** bits) % ternary5 + (a / 3 ** (5 - bits)) % ternary5;
    }else{
        bits = -bits;
        bits = bits % 5;
        return (a / 3 ** bits) % ternary5 + (a * 3 ** (5 - bits)) % ternary5;
    }
}

function andTernary(a, b) {
    let result = 0;
    let multiplier = 1;
    while (a > 0 || b > 0) {
        const digitA = a % 3;
        const digitB = b % 3;

        const maxDigit = Math.max(digitA, digitB);

        result += maxDigit * multiplier;

        a = Math.floor(a / 3);
        b = Math.floor(b / 3);
        multiplier *= 3;
    }
    return result;
}

function orTernary(a, b) {
    let result = 0;
    let multiplier = 1;
    while (a > 0 || b > 0) {
        const digitA = a % 3;
        const digitB = b % 3;

        const minDigit = Math.min(digitA, digitB);

        result += minDigit * multiplier;

        a = Math.floor(a / 3);
        b = Math.floor(b / 3);
        multiplier *= 3;
    }
    return result;
}

function xorTernary(a, b) {
    let result = 0;
    let multiplier = 1;
    while (a > 0 || b > 0) {
        const digitA = a % 3;
        const digitB = b % 3;

        const sum = digitA + digitB;

        result += (sum % 3) * multiplier;

        a = Math.floor(a / 3);
        b = Math.floor(b / 3);
        multiplier *= 3;
    }
    return result;
}

function notTernary(a) { 
    let result = 0;
    let multiplier = 1;
    while (a > 0) {
        const digitA = a % 3;

        result += ((2 - digitA) % 3) * multiplier;

        a = Math.floor(a / 3);
        multiplier *= 3;
    }
    return result;
}


function main(args) {
    var [m0,m1,m2,m3,m4,m5,m6,m7, tempo, beat, sampleRate, bufferLen, bufferPos, 
        time, note, velocity, justPressed, justReleased] = args;
    // Calculate frequency
    var freq = midiNoteToFreq(note);

    var t = (time * freq / 8 % 1.0) * (ternary5 - 2) | 0;
    var output = 
        xorTernary(
            xorTernary(
            shift5Ternary(t, 4 * m1),
            shift5Ternary(t, -1)
            ),
            t*m2*11
        );

    var slowAttack = time < 1.0 ? time : 1.0;

    output = dcBlocker(
        (output*1.0 / ternary5 -0.5)* m0 % 1.0 * slowAttack
    );
    return output;
}