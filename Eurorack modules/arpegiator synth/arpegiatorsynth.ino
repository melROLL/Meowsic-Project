// synthe a sequence d'arperge avec biblioteue mozzi
// Melvyn ROLLAND et sutout github 
#include <MozziGuts.h>
#include <Oscil.h>

#include <tables/saw8192_int8.h>
#include <tables/smoothsquare8192_int8.h>

#include <EventDelay.h>
#include <mozzi_midi.h>
#include <LowPassFilter.h>

#define CONTROL_RATE 128 // seulemnt des puissance de deux sinon çça marhc e pas 

Oscil<SAW8192_NUM_CELLS, AUDIO_RATE> oscOne(SAW8192_DATA);       // premier ascilateur dent de scie
Oscil<SMOOTHSQUARE8192_NUM_CELLS, AUDIO_RATE> oscTwo(SMOOTHSQUARE8192_DATA); //second oscilateur caré
LowPassFilter lpf;                                                               //le filtre passe bas 

// le gain de notre sequence 
EventDelay kGainChangeDelay;
char gain = 1;

// definition des variables analogiques
const int OSC_ONE_PIN = 2;    // hauteur
const int OSC_TWO_PIN=3;      // phase
const int MODA_PIN=0;         // vitesse
const int MODB_PIN=1;         // legato
const int MODC_PIN=4;         // filtre passe bas

// definitions des variables pin numeriques
const int ledPin=4;           // LED
const int upButtonPin = 2;    // boutton haut pour changer de sequence
const int downButtonPin = 3;  // boutton bas

// les variables globales du code
int upButtonState;
int lastUpState = LOW;
int downButtonState;
int lastDownState = LOW;
unsigned long lastUpDebounceTime = 0;    
unsigned long lastDownDebounceTime = 0;  
unsigned long debounceDelay = 50;        // the debounce time; increase if the output flickers

// division constants
// on fait les calcules ici comme ça on gagne du temps d'intriction dans la boucle et les transition sont plus smooth
const float DIV1023 = 1.0 / 1023.0; // division constant for pot value range
const float DIV10 = 1.0 / 10.0;     // division constant for division by 10

// les variables pour le sequenceur/arpegiateur
int sequence = 0;               // la sequence actuelle
int note = 0;                   // la note axtuelle
const int numSequences = 21;    // le nombre de sequences
const int numNotes = 8;         // le nombre de note dans toutes les sequence ça ne marche pas d'en avoir de differentes longeur 

// les sequences a jouer 
const int NOTES[numSequences][numNotes] = {
                        {36,48,36,48,36,48,36,48},                    // up on two
                        {36,36,36,36,36,36,36,36},                    // flat quarters
                        {36,36,36,48,36,36,36,48},                    // up on four
                        {36,36,48,42,36,36,36,48},                    // dum da dum
                        {48,48,36,48,48,36,48,36},                    // two up one down
                        {36,48,36,48,36,48,36,48},                    // up down
                        {36,36,48,36,36,48,36,48},                    // up on three
                        {36,37,38,39,40,41,42,43},                    // chromatic up
                        {43,42,41,40,39,38,37,36},                    // chromatic down
                        {36,38,39,41,43,44,46,48},                    // major up
                        {48,46,44,43,41,39,38,36},                    // major down
                        {36,38,40,43,45,48,50,52},                    // natural minor up
                        {52,50,48,45,43,40,38,36},                    // natural minor down
                        {36,39,41,42,43,46,48,51},                    // major pentatonic up
                        {51,48,46,43,42,41,39,36},                    // major pentatonic down
                        {36,39,41,42,43,46,48,51},                    // blues up
                        {51,48,46,43,42,41,39,36},                    // blues down
                        {36,40,34,45,48,39,38,46},                    // random one
                        {36,38,35,45,37,43,50,41},                    // random two
                        {36,46,44,37,48,35,42,45},                    // random three
                        {36,44,48,38,39,43,44,38}};                   // random four               
                        // rajouter stranger sthing
                        // rajouter giorgio by moroder
                        
// les parameteres de controle global 
int OSC_ONE_OFFSET = 12;      // amount to offset the original midi note (12 is one octave)
int OSC_TWO_OFFSET = 2;       // amount to offset the second midi note from the osc one's midi note (5 is a fifth, two is a second, etc)
int ARP_RATE_MIN = 32;        // minimum arpeggiator rate (in millisecs)
int ARP_RATE_MAX = 1024;      // maximum arpeggiator rate (in millisecs)
int LEGATO_MIN = 32;          // minimum note length (capping at 32 to avoid rollover artifacts)
int LEGATO_MAX = 1024;        // maximum note length 
int LPF_CUTOFF_MIN = 10;      // low pass filter min cutoff frequency
int LPF_CUTOFF_MAX = 245;     // low pass filter max cutoff frequency

void setup(){
  //initialisation de nos boutons 
  pinMode(upButtonPin, INPUT);
  pinMode(downButtonPin, INPUT);
  pinMode(ledPin,OUTPUT);

  //initialize Mozzi objects
  startMozzi(CONTROL_RATE);
  oscOne.setFreq(48);
  oscTwo.setFreq(51);
  lpf.setResonance(128u);
  kGainChangeDelay.set(1000);
  //Serial.begin(9600); //ça plante donc c'est en commentaire
}

void updateControl(){

  // on lit la sequence 
  // using mozziMicros() instead of millis() for timing calls because Mozzi disables millis() timer
  int upreading = digitalRead(upButtonPin);
  int downreading = digitalRead(downButtonPin);
  if (upreading != lastUpState){
    lastUpState = mozziMicros(); 
  }
  if ((mozziMicros() - lastUpDebounceTime) > debounceDelay){
    if (upreading != upButtonState){
      lastUpDebounceTime = mozziMicros();
      upButtonState = upreading;
      if (upButtonState == HIGH){
        sequence += 1;
        if (sequence >= numSequences){
          // pour paser de la derniere à la premiere sequence
          sequence = 0;
        }
      }
    }
  }
  if (downreading != lastDownState){
    lastDownState = mozziMicros();
  }
  if ((mozziMicros() - lastDownDebounceTime) > debounceDelay){
    if (downreading != downButtonState){
      lastDownDebounceTime = mozziMicros();
      downButtonState = downreading;
      if (downButtonState == HIGH){
        sequence -= 1;
        if (sequence < 0){
          // pour boucler de la premiere à la derniere sequence 
          sequence = numSequences - 1;
        }
      }
    }
  }
  
  // pon lit les valeurs des potentiometres
  int oscOne_val = mozziAnalogRead(OSC_ONE_PIN);
  int oscTwo_val = mozziAnalogRead(OSC_TWO_PIN);
  int modA_val = mozziAnalogRead(MODA_PIN);
  int modB_val = mozziAnalogRead(MODB_PIN);
  int modC_val = mozziAnalogRead(MODC_PIN);

  // map pot vals
  // These formulas set the range of values coming from the pots to value ranges that work well with the various control functionality.
  // You'll probably only need to mess with these if you want to expand or offset the ranges to suit your own project's needs
  int oscOne_offset = (OSC_ONE_OFFSET*2) * ((oscOne_val * DIV1023)-0.5);                  // offset of original midi note number +/- 1 octave
  float oscTwo_offset = ((oscTwo_val * DIV1023) * DIV10) * OSC_TWO_OFFSET;                // frequency offset for second oscillator +/- 0.2 oscOne freq
  float modA_freq = ARP_RATE_MIN + (ARP_RATE_MAX * (1-(modA_val * DIV1023)));             // arpeggiator rate from 32 millisecs to ~= 1 sec
  float modB_freq = 1-(modB_val * DIV1023);                                               // legato from 32 millisecs to full on (1 sec)
  int modC_freq = LPF_CUTOFF_MIN + (LPF_CUTOFF_MAX *(modC_val * DIV1023));                // lo pass filter cutoff freq ~=100Hz-8k

  // using an EventDelay to cycle through the sequence and play each note
  kGainChangeDelay.set(modA_freq);                                        // la frequence du  delay                                        
  if(kGainChangeDelay.ready()){                                           
      if(gain==0){                                                        // we'll make changes to the oscillator freq when the note is off
        if(note >= numNotes){                                             // if we've reached the end of the sequence, loop back to the beginning
            note = 0;
        }
        // turn on the LED on first note of sequence only
        if (note==0){
          digitalWrite(ledPin,HIGH);  
        }else{
          digitalWrite(ledPin,LOW);
        }
        // set oscillator notes based on current note in sequence
        float noteOne = mtof(NOTES[sequence][note] + oscOne_offset);      // osc one's freq = note plus any offset from user
        float noteTwo = noteOne + oscTwo_offset;                          // osc two's freq = osc one's freq plus user offset
        oscOne.setFreq(noteOne);                                          
        oscTwo.setFreq(noteTwo);
        note += 1;
        
        // setting length of note
        gain = 1;
        kGainChangeDelay.set(modA_freq*(1-modB_freq));                    // set length that note is on based on user legato settings
      }
      else{
          gain = 0;
          kGainChangeDelay.set(modA_freq*modB_freq);                      // set length that note is off based on user legato settings
      }
    kGainChangeDelay.start();                                             // execute the delay specified above
  }
  // setting lo pass cutoff freq
  lpf.setCutoffFreq(modC_freq);                                           // set the lo pass filter cutoff freq per user settings
  
}

int updateAudio(){
  // calculating the output audio signal as follows:
  // 1. Summing the waveforms from the two oscillators
  // 2. Shifting their bits by 1 to keep them in a valid output range
  // 3. Multiplying the combined waveform by the gain (volume)
  // 4. Passing the signal through the low pass filter
  return (char)lpf.next((((oscOne.next() + oscTwo.next())>>2) * gain));
}

void loop(){
  audioHook();
}
