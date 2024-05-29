
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
int v = 0;
void setup() 
{
  pinMode(ledPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin,1); //*** OFF
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
}

void loop() {
  v = 1-v;
  digitalWrite(ledPin, v);
  delay(100);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  Wire.write("hello "); // respond with message of 6 bytes
  // as expected by master
}


