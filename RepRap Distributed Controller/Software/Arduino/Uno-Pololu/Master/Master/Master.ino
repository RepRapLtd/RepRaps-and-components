#include "WirePacket.h"

#define COMMAND_LENGTH 50

#define MAX_AX 4

char axes[4] = {'X', 'Y', 'Z', 'E'};

enum commands
{
  deltaC = 'D',
  stepsC = 'S',
  currentC = 'c',
  slaveC = 's',
  helpC = '?',
  getC = 'g',
  moveC = 'm',
  lastC = 'l',
  timeC = 't',
  circleC = 'o',
  feedC = 'f',
  accelerationC = 'a',
  zeroC = 'z',
  intervalC = 'i'
};

float feedRate = 10;
float acceleration = 3;
float coordinates[MAX_AX];
float oldCoordinates[MAX_AX];
float stepsPerMM[MAX_AX] = {87.4890, 87.4890, 4000.0, 420.0};

long oldPosition[MAX_AX];
long newPosition[MAX_AX];
int addresses[MAX_AX];
volatile bool ddaActive = false;
volatile long totalSteps = -1;

unsigned char data[DATA_LENGTH];

unsigned char command[COMMAND_LENGTH];
int cp;

int slave = 0;

const int clockPin = 3;
int tickTime = 2000; // us

void StepInterrupt()
{
  if(totalSteps < 0)
    Serial.println("Received interrupt while totalSteps not set!");

  totalSteps--;

  if(totalSteps <= 0)
    ddaActive = false; 
}

void MoveAxes()
{
  if(ddaActive)
  {
    Serial.println("Trying to start a dda when one is already running!");
    return;
  }
    long nextTick;
    totalSteps = -1;
    long delta;
    int bigDirection;
    
    for(int axis = 0; axis < MAX_AX; axis ++)
    {
        newPosition[axis] = round(coordinates[axis]*stepsPerMM[axis]);
        
        delta = newPosition[axis] - oldPosition[axis];
        
        if(abs(delta) > totalSteps)
        {
            totalSteps = abs(delta);
            bigDirection = axis;
        }
        PutData(&data[1], delta);
        data[0] = deltaC;
        WirePacket->Send(addresses[axis], data, success);
    }

    if(totalSteps <= 0)
        return; // nothing to do

    float d = 0.0;
    float dd;
    
    for(int axis = 0; axis < MAX_AX; axis ++)
    {
        PutData(&data[1], totalSteps);
        data[0] = stepsC;
        WirePacket->Send(addresses[axis], data, success);
        oldPosition[axis] = newPosition[axis];

        dd = coordinates[axis] - oldCoordinates[axis];
        d += dd*dd;
        oldCoordinates[axis] = coordinates[axis];
    }

//    Serial.print("totalSteps = ");
//    Serial.println(totalSteps);
    

    d = sqrt(d)/(float)totalSteps;
    long minTickTime = round(1000000.0*d/feedRate);
    PutData(&data[1], minTickTime);
    data[0] = intervalC;
    ddaActive = true;
    long moveTimeOut = millis() + 5000;
    WirePacket->Send(addresses[bigDirection], data, success);

    while(ddaActive)
    {
      if(moveTimeOut - millis() < 0)
      {
        Serial.print("Move timed out.  totalSteps = ");
        Serial.println(totalSteps);
        ddaActive = false;
      }
    }
    
    totalSteps = -1;
}

void circle(float r)
{
  for(int axis = 0; axis < MAX_AX; axis++)
    coordinates[axis] = oldCoordinates[axis];
  float angle;
  float x0 = oldCoordinates[0];
  float y0 = oldCoordinates[1];
  for(int theta = 0; theta < 100; theta++)
  {
    angle = (float)theta*0.02*3.1415926;
    coordinates[0] = x0 + r*cos(angle);
    coordinates[1] = y0 + r*sin(angle);
    MoveAxes();
  }
  coordinates[0] = x0;
  coordinates[1] = y0;
  MoveAxes();
}


void PutData(unsigned char d[], long v)
{
  for (int i = 3; i >= 0; i--)
  {
    d[i] = v & 0xff;
    v = v >> 8;
  }  
}

long GetData(unsigned char d[])
{
  long v = 0;
  for(int i = 0; i < 3; i++)
  {
    v = v | (d[i] & 0xff);
    v = v << 8;
  }
  v = v | (d[3] & 0xff);
  return v;
}

void Clear()
{
  for(int i = 0; i < DATA_LENGTH; i++)
    data[i] = 0;
  for(int i = 0; i < COMMAND_LENGTH; i++)
    command[i] = 0;
  cp = 0;     
}

void Zero()
{
  for(int axis = 0; axis < MAX_AX; axis++)
  {
    oldPosition[axis] = 0;
    oldCoordinates[axis] = 0.0;
  }
}

void Act()
{
  data[0] = command[0];
  long v;
  float r;
  char* pch;
  int axis;
      
  switch(command[0])
  {
    case currentC:
      sscanf(&command[1], "%ld", &v);
      PutData(&data[1], v);
      WirePacket->Send(addresses[slave], data, success);
      Serial.print("Motor current set to ");
      Serial.println(v);
      break;

    case lastC:
      WirePacket->Send(addresses[slave], data, success);
      if(WirePacket->Get(addresses[slave], data))
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

    case getC:
      for(int axis = 0; axis < MAX_AX; axis++)
      {
        Serial.print(axes[axis]);
        Serial.print(": ");
        WirePacket->Send(addresses[axis], data, success);
        if(WirePacket->Get(addresses[axis], data))
        {
          v = GetData(&data[1]);
          Serial.print(v);
          v = oldPosition[axis] - v;
          if(v)
          {
            Serial.print(" (which is ");
            Serial.print(v);
            Serial.print(" steps different from the recorded value of ");
            Serial.print(oldPosition[axis]);
            Serial.print(" steps)");
          }
        } else
          Serial.print("No reply");
        delay(200);
        if(axis < MAX_AX - 1)
          Serial.print(", ");
        else
          Serial.println();
      }
      break;

    case moveC:
      pch = strtok(&command[1], " \n");
      axis = 0;
      Serial.print("Moving to: ");
      while (pch != NULL)
      {
        coordinates[axis] = atof(pch);
        Serial.print(coordinates[axis]);
        Serial.print(" ");
        axis++;
        pch = strtok(NULL, " \n");
      }
      Serial.println();
      MoveAxes();
      break;

    case slaveC:
      slave = -1;
      for(int axis = 0; axis < MAX_AX; axis++)
      {
        if(command[1] == axes[axis])
        {
          slave = axis;
          break;
        }
      }
      if(slave < 0)
      {
        slave = 0;
        Serial.print("No axis named \"");
        Serial.print((char)command[1]);
        Serial.println("\" exists.");
      }
      Serial.print("Talking to slave: ");
      Serial.println(axes[slave]);
      break;

    case feedC:
      feedRate = atof(&command[1]);
      Serial.print("Feedrate set to ");
      Serial.print(feedRate);
      Serial.println(" mm/sec.");
      break;

  case accelerationC:
      acceleration = atof(&command[1]);
      Serial.print("Acceleration set to ");
      Serial.print(acceleration);
      Serial.println(" mm/sec^2.");
      break;  

    case timeC:
      sscanf(&command[1], "%ld", &tickTime);
      Serial.print("Step time set to ");
      Serial.print(tickTime);
      Serial.println(" microseconds.");
      break;

    case circleC:
      circle(atof(&command[1]));
      break;

    case zeroC:
      Zero();
      break;
      
    default:
      Serial.println("Commands:");
      Serial.print(' '); Serial.print((char)currentC); Serial.println("nnn - set motor current");
      Serial.print(' '); Serial.print((char)getC); Serial.println(" - get, check and print the current coordinates");
      Serial.print(' '); Serial.print((char)moveC); Serial.println("X Y Z E - move to a position in mm");
      Serial.print(' '); Serial.print((char)slaveC); Serial.println("X|Y|Z|E - set which slave to talk to");
      Serial.print(' '); Serial.print((char)feedC); Serial.println("nn.n - set feedrate in mm/sec");
      Serial.print(' '); Serial.print((char)accelerationC); Serial.println("nn.n - set acceleration in mm/sec^2");
      Serial.print(' '); Serial.print((char)lastC); Serial.println(" - print the last packet sent to the slave");
      Serial.print(' '); Serial.print((char)circleC); Serial.println("nnn - draw a circle radius nnn centred at the current position");
      Serial.print(' '); Serial.print((char)zeroC); Serial.println(" - set the current position to be the origin");    
      Serial.print(' '); Serial.print((char)helpC); Serial.println(" - print this list");
      
  }
  Clear();
}

void Init()
{
  command[0] = slaveC;
  command[1] = 'X';
  command[2] = 0;
  Act();  
  command[0] = currentC;
  sprintf(&command[1], "%d", 200);
  Act();
  
  command[0] = slaveC;
  command[1] = 'Y';
  command[2] = 0;
  Act();
  command[0] = currentC;
  sprintf(&command[1], "%d", 300);
  Act(); 

  command[0] = slaveC;
  command[1] = 'Z';
  command[2] = 0;
  Act();
  command[0] = currentC;
  sprintf(&command[1], "%d", 200);
  Act(); 

  command[0] = feedC;
  sprintf(&command[1], "%d", 30);
  Act(); 
}
  
void setup() 
{
  pinMode(clockPin, INPUT_PULLUP);
  //digitalWrite(clockPin, 1);
  WirePacket = new WPac();
  Serial.begin(115200);
  Clear();
  Zero();
  ddaActive = false;
  attachInterrupt(digitalPinToInterrupt(clockPin), StepInterrupt, FALLING);
  addresses[0] = 15;
  addresses[1] = 16;
  addresses[2] = 17;
  addresses[3] = 18;
  Init();
  command[0] = '?';
  command[1] = 0;
  Act();
  sei();
}

void loop() 
{
  WirePacket->Spin();
  
  if(Serial.available())
  {
    unsigned char c = Serial.read();
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

