/*  
  Autor: Cristhian Novais modified by Meowsic
*/


#include <MozziGuts.h>
#include <Oscil.h>
#include <EventDelay.h>
#include <ADSR.h> 
#include <mozzi_rand.h>
#include <mozzi_midi.h>
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();
#define CONTROL_RATE 64
//COSINE WAVE
#include <tables/cos8192_int8.h> 
Oscil <8192, AUDIO_RATE> aOscil(COS8192_DATA);
Oscil <8192, AUDIO_RATE> aOsci2(COS8192_DATA);
Oscil <8192, AUDIO_RATE> aOsci3(COS8192_DATA);
Oscil <8192, AUDIO_RATE> aOsci4(COS8192_DATA);


ADSR <CONTROL_RATE, AUDIO_RATE> envelope;
ADSR <CONTROL_RATE, AUDIO_RATE> envelope2;
ADSR <CONTROL_RATE, AUDIO_RATE> envelope3;
ADSR <CONTROL_RATE, AUDIO_RATE> envelope4;
boolean note_is_on = true;

int note=75;

/*
* Paramethers of note
*/
//Intensity of phase
byte attack_level = 255;
byte decay_level = 210;
byte notas[3];

//Duration of each phase
unsigned int attack=10;
unsigned int decay=10;
unsigned int sustain=40000;
unsigned int release_ms=393;

//Used to mute the speaker if no notes are being played.
boolean no_nota=true;
EventDelay noteDelay;


void handleNoteOn(byte channel, byte pitch, byte velocity)
{

    if(notas[0]==0)
    {
      aOscil.setFreq((int)mtof(pitch));
      notas[0]=pitch;
      envelope.noteOn();
      aOsci2.setFreq(((int)mtof(pitch))*2);
      envelope2.noteOn();
    }
    
    else if(notas[1]==0)
    {
      aOsci3.setFreq((int)mtof(pitch));
      notas[1]=pitch;
      envelope3.noteOn();
      aOsci4.setFreq(((int)mtof(pitch))*2);
      envelope4.noteOn();
    }
      
    envelope.update();
    envelope2.update();
    envelope3.update();
    envelope4.update();
    digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)

    
}


void handleNoteOff(byte channel, byte pitch, byte velocity)
{
    digitalWrite(13, LOW);   // turn the LED off

    if(notas[0]==pitch)
    {
       envelope.noteOff();
       envelope2.noteOff();
       notas[0]=0;
       noteDelay.start(release_ms+50);
     }
      
     else if(notas[1]==pitch)
     {
       envelope3.noteOff();
       envelope4.noteOff();
       notas[1]=0;
       noteDelay.start(release_ms+50);
     }
  
}

void setup(){

    pinMode(13, OUTPUT);     

    notas[0]=0;
    notas[1]=0;
    // choose envelope levels
    
    envelope.setADLevels(attack_level,decay_level);
    envelope2.setADLevels(attack_level-50,decay_level-50);
    envelope3.setADLevels(attack_level,decay_level);
    envelope4.setADLevels(attack_level-50,decay_level-50);
 
    // generate a random new adsr time parameter value in milliseconds
    envelope.setTimes(attack,decay,sustain,release_ms);   
    envelope2.setTimes(attack+50,decay+200,sustain-200,release_ms);
    envelope3.setTimes(attack,decay,sustain,release_ms);   
    envelope4.setTimes(attack+50,decay+200,sustain-200,release_ms);       
 

    byte midi_note = note;
    aOscil.setFreq((int)mtof(midi_note));
    aOsci2.setFreq(2*((int)mtof(midi_note)));
    aOsci3.setFreq((int)mtof(midi_note));
    aOsci4.setFreq(2*((int)mtof(midi_note)));
  
    startMozzi(CONTROL_RATE);
    MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function

    // Do the same for NoteOffs
    MIDI.setHandleNoteOff(handleNoteOff);

    // Initiate MIDI communications, listen to all channels
    MIDI.begin(MIDI_CHANNEL_OMNI);
}



void updateControl(){    
  
    if(noteDelay.ready() && notas[0]==0 && notas[1]==0)
      no_nota=true;
    else
      no_nota=false;
  
    envelope.update();
    envelope2.update();
    envelope3.update();
    envelope4.update();
} 


int updateAudio(){
    if(no_nota)
      return 0;
    else{
      return ((long)envelope.next() * aOscil.next())
              +((int)envelope2.next() * aOsci2.next())
              +((int)envelope3.next() * aOsci3.next())
              +((int)envelope4.next() * aOsci4.next())
        >>8;
    }
}


void loop(){
  
    MIDI.read();
    audioHook(); 
    
}



