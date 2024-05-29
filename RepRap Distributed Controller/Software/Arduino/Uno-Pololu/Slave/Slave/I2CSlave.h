
#ifndef I2CSLAVE_H
#define I2CSLAVE_H

const float PWMMod = 2.5;

/* OUTPUT */

const int pinREF = 3;         // PWM for setting REF voltage

#define DDA_COUNT 3

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

class I2CSlave
{
  public:

    I2CSlave(int address);
    void Spin();
    void Step();
    void Increment();
    void Decrement();
    void DDAFinished();
    WPac* WirePacket();
        
  private:

    void SetRef(float REFVoltage);
    void PutData(unsigned char d[], long v);
    long GetData(unsigned char d[]);
    void Act();
    void Error(char* e);

    WPac* wirePacket;
    DDA* activeDDA;
    DDA* nextDDA;
    unsigned char data[DATA_LENGTH];
    unsigned char oldData[DATA_LENGTH];

    int myAddress;
    volatile long coordinate;

    int voltagePWM;

};


inline void I2CSlave::Step()
{
  // Deal with the case when a new sequence is starting and we are not the clock
  
  if(!activeDDA->Set())
  {
    if(!nextDDA->Set())
    {
      Error("nextDDA used before set!");
      return;
    }
    activeDDA = nextDDA;
    nextDDA = activeDDA->Next();
    if(!activeDDA->Activate())
      Error("Attempting to activate unready DDA 1!");

    // If we arrive at this point someone else must be the clock and
    // have sent us a pulse to get us here, so complain if we think we 
    // are the clock.

    if(activeDDA->IAmTheClock())
    {
      Error("The clock-generating slave has received a clock pulse!");
      return;
    }
  }

  // Now take the step
  
  activeDDA->Step();
}

inline void I2CSlave::DDAFinished()
{
  if(nextDDA->Set())
  {
    // If the next one is good to go, Queue it up
    
    activeDDA = nextDDA;
    nextDDA = activeDDA->Next();
    if(!activeDDA->Activate())
      Error("Attempting to activate unready DDA 2!");
  }
}

inline void I2CSlave::Increment()
{
  coordinate++;
}

inline void I2CSlave::Decrement()
{
  coordinate--;
}

inline WPac* I2CSlave::WirePacket()
{
  return wirePacket;
}

inline void I2CSlave::Error(char* e)
{
  Serial.println(e);
}

#endif

