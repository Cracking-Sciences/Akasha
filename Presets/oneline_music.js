// javascripts here
const { sin, cos, pow, tan, floor, random, abs, max, PI, min, cbrt, asin, atan} = Math;
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
K=t=>(dt="00020001"[t>>13&7],f=0.9995**(t&8191>>dt),asin(sin(100*f))*42*f+96+15*random()*f**9)*1.6,
K2=t=>(f=0.9995**(t&8191),asin(sin(100*f))*42*f+64+15*random()*f**9)*1.6,
C=t=>t+8192>>14&t>>12&t>>13&1?128*atan(sin(t/16)+sin(t/9))/1.0015**(t&2047):0,
H=t=>t+2048>>12&(~t>>13&3)>0?32*random()/1.001**(t&2047):0,
H2=t=>t+2048>>12&1?32*random()/1.001**(t&2047):0,
F=t=>(random()*(t>>8&255))/4,
T=p=>2**(p/12),
s=t>1<<19,s2=t>>13,
b=l=>"AAAMMMKMAAAMMMKMAAAMMMKM"+(l?"AAAMMMKM":t>>16&1?"RRPRPPMP":"AAMPRRPR"),
bt="00004421",
B=(t,q)=>((bs=t>>11&31,t*T(b(q).charCodeAt(bs)-65-(q?bt[t>>14&7]:0))|(~bs&24||q?((bs&7)<3?(t>>3&2047)/3:(bs&7)<5?t-2048>>4:t>>3):bs&2?t>>3:t>>4))&255)/2,
SB=t=>[(t%257|t%255)-192,~t%255|t%257],
BP=t=>(q=t%(1<<17),(t+(q>99999?8192:0)>>14&1)*t*(9+(t>>8&1))/(1+((q>99999)*(t>>12&3)))&31),
E=(Z,t,f,i=0)=>i<8?t<0?0:Z(t)+E(Z,t-8192,f,i+1)*f:0,
SE=(Z,e,i)=>i>16?0:e<0?0:Z(e)/2+SE(Z,e-16384+(i==0?8192:0),i+2)*0.6,
CM=(Z,t,...a)=>Z(t,...a)+Z(t+sin(t/3E3)*40,...a)/1.5,
m1="ACDFHHMKHHAAHFDCACDFHHFD"+(t>>17&1?"CCDDFFHH":"CACDCA@C"),
m2="<@ACDDHFDD<<DCA?=?ACDDCA"+(t>>17&1?"????@@@@":"?<?A@=<@"),
m3=~t>>17&3?"KMHFHHFHKMHFHHFHFDC?AA?ACDFHAAHK":"KMHFHHFHKMHFHHMOPOMKHHFHFDC?AA",
m4=~t>>17&3?"HIDCDDCDHIDCDDCDCA?:<<:<?ACD<<DH":"HIDCDDCDHIDCDDHKMKHFDDCDCA?:<<",
L=(t,a)=>t*T(a.charCodeAt(t>>12&31)-65)&63&(a[t>>12&31]==a[(t>>12&31)-1]||a[t>>12&31]==a[(t>>12&31)+1]?~t>>7:~t>>6),
n=(t*=(s2>446?1.0716:1),[s2<24?s2&8?5:21:s2<31?37:16,s2&8?138:154,394,3722,61450,61450,831489,831489,196634,196682,3786,3786,61450,61514,831489,s2<507?831489:655376,s2<515?655376:0][s2>>5]),
G=(Q,q,...a)=>n&1<<q?Q(t,...a):0,J=(v,q)=>n&1<<q?v:0,
bq=(r=t+(1<<16),~r>>14&7?r:r-(1<<14)),
SS=e=>(J(K(t)+SE(C,t,e),0)+G(K2,1)+G(H,2)+G(H2,3)+J(F(~t),4)+J(F(t),5)+J(F((~t&8191)+2048)*3,6)+G(B,7,s)+G(L,8,m1)+J(CM(L,t,m1),9)+J(CM(L,t,m2),10)+J(SB(t*T(-bt[t>>14&7]))[e]/4,11)+J(CM(L,t,m3),12)+J(CM(L,t,m4),13)+J(B(bq,s),14)+J(SB(t*T(-bt[bq>>14&7]))[e]/4,15)+G(B,16,0)+G(B,16,0)%32+J(SB(t)[e],17)/6*(1+(n&64?!(t>>12&3):0))+G(BP,18)-G(K,18)+J(96,19))/2,
[SS(0),SS(1)]

);
}

function main(args){
    var [m0,m1,m2,m3,m4,m5,m6,m7, tempo, beat, sampleRate, bufferLen, bufferPos, 
        time, note, velocity, justPressed, justReleased
        ] = args;

    var t = time*100000*m0;



    var output = onelinePlayer(t);// / 256.0;
    output = output.map(value => (value / 256) - 0.5);
    return output;
}
