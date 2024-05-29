/*
  Class to send and recieve fixed length data packets via I2C
  
  Adrian Bowyer
  RepRapLtd
  
  http://reprapltd.com
  
  7 September 2016
  
 
 
 The bytes in a packet looks like this:
   
      *Sdd00
      
 where * is the special start character, S is a status byte for communicating
 data types, errors etc, dd is some data and 00 is padding the data out to 
 PACKET_LENGTH.
 
*/

#ifndef WIRE_PACKET_H
#define WIRE_PACKET_H


#define PACKET_LENGTH 7
#define DATA_LENGTH 5
#define MAX_DUD_BYTES 8
#define START_CHAR '*'
#define LONG_WAIT 5 // milliseconds
#define I2C_CLOCK 100000L 

enum Status
{
  success,
  dudchars,
  blocked,
  receivewait
};
 
class WPac
{

 public:
    void Spin();

    WPac();
    WPac(int address);
    void Send(unsigned char* str, Status s);
    void Send(int address, unsigned char* str, Status s);
    bool Get(unsigned char* str);
    bool Get(int address, unsigned char* str);
    void Print(unsigned char* str, Status s);

  private:
  
    void BeginTransmission(int toAddress);
    void EndTransmission();
    void RequestFrom(int address);
    
    int lastSlave;
    static void RequestEvent();
    static void ReceiveEvent(int numBytes);
    bool SendReadyWithTimeOut(); 
       
    int myAddress;

    unsigned char Read();
    void Write(unsigned char c);
    void Begin();
    void Begin(int address);
    bool Available();
    bool AvailableWithTimeOut(int flag);
    void DealWithError(Status s, int flag, bool slave);

    long startTime;
  
};


extern WPac* WirePacket;

#include <Wire.h>

inline void WPac::Begin()
{
  Wire.begin();
  Wire.setClock(I2C_CLOCK);
}

inline void WPac::Begin(int address)
{
  Wire.begin(address);
  Wire.setClock(I2C_CLOCK);
  Wire.onRequest(WirePacket->RequestEvent);
  Wire.onReceive(WirePacket->ReceiveEvent);  
}

inline void WPac::BeginTransmission(int toAddress)
{
  Wire.beginTransmission(toAddress);  
}

inline void WPac::Write(unsigned char c)
{
  Wire.write(c);  
}

inline void WPac::EndTransmission()
{
  Wire.endTransmission();  
}

inline bool WPac::Available()
{
  return (Wire.available() > 0);  
}

inline unsigned char WPac::Read()
{
  return Wire.read();
}

inline void WPac::RequestFrom(int address)
{
  Wire.requestFrom(address, PACKET_LENGTH);
}



#endif

