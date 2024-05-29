#include "WirePacket.h"

char data[DATA_LENGTH];
char oldData[DATA_LENGTH];
bool stepRun = false;
bool stepForward = true;

const float PWMMod = 2.67;
int voltagePWM;

const int pinREF = 1;         // PWM for setting REF voltage
const int pinDIR = 5;         // Digital output to set direction
const int pinSTEP = 4;        // Digital wave output to trigger stepping
const int pinClock = 3; 

ISR(PCINT0_vect)
{
  if(!stepRun || digitalRead(pinClock))
    return;
    
  // Create step pulse only on falling edge
  
  digitalWrite(pinSTEP, LOW);
  digitalWrite(pinSTEP, HIGH);
}

void setDIR()
{
  if (stepForward) 
  {
    digitalWrite(pinDIR, HIGH);
  } else 
  {
    digitalWrite(pinDIR, LOW);
  }
}

void setREF(float REFVoltage)
{
  voltagePWM = round(REFVoltage/PWMMod);
  analogWrite(pinREF, voltagePWM);   
}

void Act()
{
  switch(data[0])
  {
    case 0:
      stepRun = false;
      break;

    case 1:
      stepRun = true;
      break;

    case 2:
      setREF((float)data[1]);
      break;

    case 3:
      data[0] = voltagePWM;
      WirePacket->Send(data, success);
      break;

    case 4:
      stepForward = true;
      setDIR();
      break;

    case 5:
      stepForward = false;
      setDIR();
      break;

    case 6:
      WirePacket->Send(oldData, success);
      break;  

    default:
       ;
  }
}
  
void setup() 
{
  WirePacket = new WPac(8);

  pinMode(pinREF, OUTPUT);
  pinMode(pinSTEP, OUTPUT);
  pinMode(pinDIR, OUTPUT);
  pinMode(pinClock, INPUT);

// High Speed PWM settings
  TCCR0A = 2<<COM0A0 | 2<<COM0B0 | 3<<WGM00;
  TCCR0B = 0<<WGM02 | 1<<CS00;
  TCCR1 = 0<<PWM1A | 0<<COM1A0 | 1<<CS10;
  GTCCR = 1<<PWM1B | 2<<COM1B0;
    
// Interrupt pin setting
  GIMSK = 0b00100000;
  PCMSK = 1 << pinClock;

  for(int i = 0; i < DATA_LENGTH; i++)
    data[i] = 0;
  Act();
  setREF(25.0);
  sei();         
}

void loop() 
{
  WirePacket->Spin();
  if(WirePacket->Get(data))
  {
    Act();
    for(int i = 0; i < DATA_LENGTH; i++)
      oldData[i] = data[i];
  }
}




