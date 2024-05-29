/*
 * DDA stepping class for I2C control of distributed devices round a
 * RepRap machine.
 * 
 * Adrian Bowyer
 * 
 * 21 October 2016
 */
#ifndef DDA_H
#define DDA_H

#define FORWARDS 0
#define BACKWARDS (!FORWARDS)

const int pinDIR = A0;         // Digital output to set direction
const int pinSTEP = A1;        // Digital wave output to trigger stepping (A1 on shield)
const int pinClock = 2;

class I2CSlave;
void StepInterrupt();

class DDA
{
  public:
    DDA(DDA* next, I2CSlave* s);
    void Step();
    void SetDelta(long d);
    void SetTotal(long t);
    void SetInterval(long i);
    DDA* Next();
    void SetNext(DDA* n);
    bool Active();
    bool Activate();
    bool Set();
    void Spin();
    bool IAmTheClock();

  private:
    void SetDir();
    void Error(char* e);
    void Clear();
    void ClockListen();
    void ClockSend();
        
    I2CSlave* slave;
    DDA* next;
    long nextTick;
    volatile bool set;
    volatile long totalSteps;
    volatile long counter;
    volatile long interval;
    volatile long delta;
    volatile long stepCount;
    volatile bool moveDirection;
    volatile bool active;  
};

inline DDA* DDA::Next()
{
  return next;
}

inline bool DDA::IAmTheClock()
{
  return interval > 0;
}

inline void DDA::SetNext(DDA* n)
{
  next = n;
}

inline void DDA::SetDir()
{
  digitalWrite(pinDIR, moveDirection);
}

inline bool DDA::Active()
{
  return active;
}

inline bool DDA::Set()
{
  return set;
}

inline void DDA::Clear()
{
  active = false;
  set = false;
  interval = -1;
  ClockListen();  
}

inline void DDA::ClockListen()
{
  pinMode(pinClock, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinClock), StepInterrupt, FALLING);  
}

inline void DDA::ClockSend()
{
  detachInterrupt(digitalPinToInterrupt(pinClock));
  pinMode(pinClock, OUTPUT);
  digitalWrite(pinClock, HIGH); 
}
 
#endif

