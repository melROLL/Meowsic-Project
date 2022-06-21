#include <MIDI.h>

#define redLed 13
#define pot A7
#define fourth 7
#define seventh 8
#define eighth 9
#define switchOne 11
#define switchTwo 10

MIDI_CREATE_DEFAULT_INSTANCE();

int notes[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int scale[] = {0, 2, 3, 5, 7, 9, 10, 12, 14, 15, 17, 19, 21, 22, 24, 26, 27, 29, 31, 33, 34, 36};
int seqLength;
int range = 21;
int bpm;
int rootNote = 33;
int potValue;
int four;
int seven;
int eight;
int hold;
int loops = 4;
int count = 1;

void setup() {
  
  pinMode(redLed, OUTPUT);
  pinMode(pot, INPUT);
  pinMode(fourth, INPUT);
  pinMode(seventh, INPUT);
  pinMode(eighth, INPUT);
  pinMode(switchOne, INPUT);
  
  digitalWrite(0, LOW);

  //Serial.begin(115200);
  MIDI.begin(MIDI_CHANNEL_OFF);

  potValue = analogRead(pot);
  randomSeed(potValue);

}

void loop() {
  //Serial.println(count);
  
  hold = digitalRead(switchOne);
  
  if (count == 1){

    digitalWrite(redLed, HIGH);
    
    four = digitalRead(fourth);
    seven = digitalRead(seventh);
    eight = digitalRead(eighth);
    hold = digitalRead(switchOne);
    

      seqLength = 16;

    for (int i = 0; i < seqLength; i++){
      notes[i] = scale[random(0,range)];
    }

    digitalWrite(redLed, LOW);
    
  }
  
  if (count <= loops){
    
    for (int i = 0; i < seqLength; i++){
    
      potValue = analogRead(pot);
      bpm = 60 + potValue * 0.72265625;         // 60 - 800 bpm quarter notes

      int pause = digitalRead(switchTwo);
      while (pause == 1){
        delay(50);
        pause = digitalRead(switchTwo);
      }
      
      MIDI.sendNoteOn(rootNote + notes[i], 127, 1);
      digitalWrite(redLed, HIGH);
      //Serial.println(notes[i]);
      //Serial.println(potValue);
      //Serial.println(bpm);
      //Serial.println("");
      
      delay(60000/bpm*0.75);
      MIDI.sendNoteOff(rootNote + notes[i], 0, 1);
      digitalWrite(redLed, LOW);
      delay(60000/bpm*0.25);
      
    }      
    
    //Serial.println("");

    hold = digitalRead(switchOne);
    if (count == loops && hold == 1){
      count = loops;
    }
    else{
      count += 1;
    }

  }



  if (count > loops && hold == 0){
    
    count = 1;

  }
}
