
#include "DDA.h"

DDA::DDA(DDA* n, I2CSlave* s)
{
  next = n;
  slave = s;
  moveDirection = FORWARDS;
  nextTick = micros();
  Clear();
  ClockListen();
}

bool DDA::Activate()
{
  // Are we ready to go?
  
  if(!set)
    return false;

  // Yes - get the show on the road
  
  SetDir();
  active = true;

  // Are we the master clock?
  
  if(interval < 0)
    return true;

  // Yes - set the next tick time and allow ourselves to write to the clock line
  
  nextTick = micros() + interval;
  ClockSend();
  return true;
}

void DDA::Spin()
{
  // Are we the master clock?
  
  if(interval < 0 || !active)
    return;

  // Yes - generate clock pulses for everyone else and step ourselves.
  
  long t = micros();

  if(t - nextTick < 0)
    return;
  
  digitalWrite(pinClock, LOW);
  digitalWrite(pinClock, HIGH);

  Step();

  nextTick = t + interval; 
}

// The Step() function is called by the clock-line interrupt if
// we are not the master clock function, or by our own timing if we are.

void DDA::Step()
{  
  if (stepCount < totalSteps) 
  {
    counter += delta;
    if (counter > 0)
    {
      digitalWrite(pinSTEP, LOW);
      digitalWrite(pinSTEP, HIGH);
      counter -= totalSteps;
      if(moveDirection == FORWARDS)
        slave->Increment();
      else
        slave->Decrement();
    }
    stepCount++;
  }

  if(stepCount >= totalSteps)
  {
    Clear();
    slave->DDAFinished();
  }
  
}

void DDA::SetDelta(long d)
{
  if(active)
  {
    Error("Setting delta while active!");
    return;
  }
  
  delta = d;
  if(delta < 0)
  {
    moveDirection = BACKWARDS;
    delta = -delta;
  } else
    moveDirection = FORWARDS;

  Clear();
}

// This must be called AFTER delta has been set.

void DDA::SetTotal(long t)
{
  if(active)
  {
    Error("Setting total while active!");
    return;
  }
  
  totalSteps = t;
  counter = -totalSteps/2;
  stepCount = 0;
  active = false;
  interval = -1;
  set = true;
}

void DDA::SetInterval(long i)
{
  if(active)
  {
    Error("Setting interval while active!");
    return;
  }
  
  interval = i;
}

void DDA::Error(char* e)
{
  Serial.println(e);
}


