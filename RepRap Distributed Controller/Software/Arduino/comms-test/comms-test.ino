
// Wire Slave Sender
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Sends data as an I2C/TWI slave device
// Refer to the "Wire Master Reader" example for use with this

// Created 29 March 2006

// This example code is in the public domain.



#include <Wire.h>

// constants won't change. Used here to set a pin number :
const int ledPin =  13;      // the number of the LED pin

const int dirPin =  5;      // the number of the LED pin
const int stepPin =  6;      // the number of the LED pin
const int enablePin =  7;      // the number of the LED pin
const int myAddress = 8;
char buf = 0;
int v = 0;
void setup() 
{
  pinMode(ledPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin,1); //*** OFF
  Wire.begin(myAddress);                // join i2c bus with address #8
  TWAR = (myAddress << 1) | 1;  // enable broadcasts to be received
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent);
  Serial.begin(115200);
}

void loop() {
  v = 1-v;
  digitalWrite(ledPin, v);
  delay(100);
  if(Serial.available() > 0)
    buf = Serial.read();
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  if(buf)
  {
    Wire.write(&buf, 1);
  }
  buf = 0;
}

// called by interrupt service routine when incoming data arrives
void receiveEvent (int howMany)
{
  for (int i = 0; i < howMany; i++)
  {
    Serial.write(Wire.read());
  } 
}


