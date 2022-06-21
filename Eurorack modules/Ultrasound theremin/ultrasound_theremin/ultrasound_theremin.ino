//ultrasound theremin V8/  Melvyn ROLLAND 
#include <Ultrasonic.h>
#include <toneAC.h>
Ultrasonic ultrasonic(12, 13); // pin for trigger(12) and echo(13)
#define TONE_VOLUME   40   // 1-20 //output sound pin(positive of speaker)
long distance;
void setup() {
  Serial.begin(9600);
  pinMode(10,OUTPUT); //audio positive
    pinMode(8,OUTPUT); // audio ground
} // end setup 
void loop() {
  delay(100);                    // Wait 100ms between pings 
  distance = ultrasonic.read(); //reads the distance from the sensor
if(distance <= 700 )             //play a sound if we are under this range
{
 int freq= 750-distance*10;//calculates the frequence
  toneAC(freq, TONE_VOLUME);    //play the sound of the theremin (pin10)
 // Serial.println(distance);     // for debuging
 // Serial.println(freq);         // for debuging
}
else
{
  digitalWrite(8,HIGH); //let the sound go throught
 noToneAC();  //out of range so theremin doesn't make a sound 
}
}
