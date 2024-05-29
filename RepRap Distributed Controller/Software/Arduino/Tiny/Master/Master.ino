#include "WirePacket.h"

char data[DATA_LENGTH];
char command[10];
int cp;

const int slave1 = 8;

void Act()
{
  data[0] = command[0] - '0';
     
  switch(command[0])
  {
    case '0':
    case '1':
    case '2':
    case '4':
    case '5':
      WirePacket->Send(slave1, data, success);
      break;


    case '3':
      data[1] = atoi(&command[1]);
      WirePacket->Send(slave1, data, success);
      if(WirePacket->Get(slave1, data))
      {
        Serial.print("PWM: ");
        Serial.println((int)data[0]);
      } else
        Serial.println("No reply.");
      break;

    case '6':
      WirePacket->Send(slave1, data, success);
      if(WirePacket->Get(slave1, data))
      {
        Serial.print("data was: ");
        for(int i = 0; i < DATA_LENGTH; i++)
        {
          Serial.print(data[i], HEX);
          Serial.print(' ');
        }
        Serial.println();
      } else
        Serial.println("No reply.");
      break;

    default:
      Serial.println("Commands:");
      Serial.println(" 0 - motor off");
      Serial.println(" 1 - motor on"); 
      Serial.println(" 2nnn - set pwm to nnn mv"); 
      Serial.println(" 3 - return pwm"); 
      Serial.println(" 4 - motor forwards"); 
      Serial.println(" 5 - motor backwards");
      Serial.println(" 6 - retrieve last data sent");      
      Serial.println(" ? - print this list");      
  }
}
  
void setup() 
{
  WirePacket = new WPac();
  Serial.begin(115200);
  for(int i = 0; i < DATA_LENGTH; i++)
    data[i] = 0;
  cp = 0;
  command[0] = '?';
  command[1] = 0;
  Act();
}



void loop() 
{
  WirePacket->Spin();
  
  if(Serial.available())
  {
    char c = Serial.read();
    if(c == '\n')
    {
      command[cp] = 0;
      cp = 0;
      Act();
    } else
    {
      command[cp] = c;
      cp++;
    }
  } 
}
