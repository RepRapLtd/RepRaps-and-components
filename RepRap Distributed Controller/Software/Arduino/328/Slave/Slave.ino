#include "WirePacket.h"

char data[DATA_LENGTH];
char oldData[DATA_LENGTH];
bool stepRun = false;
bool stepForward = true;

const float PWMMod = 2.5;
int voltagePWM;

/* OUTPUT */
  const int pinREF = 3;         // PWM for setting REF voltage
  const int pinDIR = A0;         // Digital output to set direction
  const int pinSTEP = 13;        // Digital wave output to trigger stepping (A1 on shield)

/* INPUT */
  const int pinClock = 2; 

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
  Serial.println("");
  Serial.print("REFVoltage: "); Serial.println(REFVoltage);
  Serial.print("voltagePWM: "); Serial.println(voltagePWM);
}

void PutVoltageInData()
{
  for(int i = 0; i < DATA_LENGTH; i++)
     data[i] = 0;
  float v = voltagePWM*PWMMod;
  if(v < 10.0)
    data[3] = (char)v;
  else if(v < 100.0)
  {
    data[2] = (char)(v/10.0);
    data[3] = (char)(v - data[2]*10);
  } else
  {
    data[1] = (char)(v/100.0);
    data[2] = (char)((v - data[1]*100)/10.0);
    data[3] = (char)(v - data[1]*100 - data[2]*10);
  }
}

void Act()
{
  char tempString[20];
  int v;
  
  switch(data[0])
  {
    case 0:
      stepRun = false;
      //Serial.print("0");
      break;

    case 1:
      stepRun = true;
      //Serial.print("1");
      break;

    case 2:
      v = (int)data[1]*100 + (int)data[2]*10 + (int)data[3];
      setREF((float)v);
      //Serial.print("2");
      break;

    case 3:
      PutVoltageInData();
      WirePacket->Send(data, success);
      //Serial.print("3");
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
  Serial.begin(115200);
/*
// High Speed PWM settings
  TCCR0A = 2<<COM0A0 | 2<<COM0B0 | 3<<WGM00;
  TCCR0B = 0<<WGM02 | 1<<CS00;
  TCCR1 = 0<<PWM1A | 0<<COM1A0 | 1<<CS10;
  GTCCR = 1<<PWM1B | 2<<COM1B0;
    
// Interrupt pin setting
  GIMSK = 0b00100000;
  PCMSK = 1 << pinClock;
*/
  for(int i = 0; i < DATA_LENGTH; i++)
    data[i] = 0;
  Act();
  setREF(50);
 // sei();         
}

void loop() 
{
  WirePacket->Spin();
  if(WirePacket->Get(data))
  {
    WirePacket->Print(data, success);
    Act();
    for(int i = 0; i < DATA_LENGTH; i++)
      oldData[i] = data[i];
  }
}




