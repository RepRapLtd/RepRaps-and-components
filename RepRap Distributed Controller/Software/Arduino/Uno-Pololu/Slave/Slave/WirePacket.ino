/*
  Class to send and recieve fixed length data packets via I2C
  
  Adrian Bowyer
  RepRapLtd
  
  http://reprapltd.com
  
  7 September 2016
  
 
 
 The bytes in a packet looks like this:
   
      *Sdnnnn
      
 where * is the special start character, S is a status byte for communicating
 data types, errors etc, dd is some data and 00 is padding the data out to 
 PACKET_LENGTH.
 
*/

#include "WirePacket.h"

void ReceiveInterrupt(int numBytes)
{
  slave->WirePacket()->ReceiveEvent(numBytes);
}

void RequestInterrupt()
{
  slave->WirePacket()->RequestEvent();
}


WPac::WPac()
{
  lastSlave = -1;
  myAddress = -1;
  startTime = -1;
  debug = false;
  Begin();
}

WPac::WPac(int address)
{
  myAddress = address;
  sendStatus = success;
  errorStatus = success;
  sendReady = false;
  requestPending = false;
  dataAvailable = false;
  startTime = -1;
  debug = false;
  Begin(myAddress); 
}


bool WPac::AvailableWithTimeOut(int flag)
{
    if(startTime < 0)
      startTime = millis();
    while(!Available())
    {
      if(millis() - startTime >= LONG_WAIT)
      {
        DealWithError(receivewait, flag, false);
        return false;
      }
    }
    startTime = -1;
    return true;  
}


void WPac::Send(unsigned char* str, Status s)
{
  if(sendReady)
  {
    DealWithError(blocked, 1, true);
    return;
  }
  for(int i = 0; i < DATA_LENGTH; i++)
   dataToSend[i] = str[i];
  sendStatus = s;
  sendReady = true;    
}

void WPac::Send(int toAddress, unsigned char* str, Status s)
{
  lastSlave = toAddress;
  BeginTransmission(toAddress);
  Write(START_CHAR);
  Write((unsigned char)s);
  for(int i = 0; i < DATA_LENGTH; i++)
  {
    Write(str[i]);
  }
  EndTransmission();
}

bool WPac::Get(unsigned char* str)
{
  if(!dataAvailable)
    return false;
  for(int i = 0; i < DATA_LENGTH; i++)
    str[i] = dataReceived[i];
  dataAvailable = false;
  return true;         
}


bool WPac::Get(int fromAddress, unsigned char* str)
{
    lastSlave = fromAddress;
    delayMicroseconds(500);
    RequestFrom(fromAddress);
    if(!AvailableWithTimeOut(2))
      return false;
    int dudCount = 0;
    int c = Read();
    while(c != START_CHAR && dudCount < MAX_DUD_BYTES)
    {
        dudCount++;
        if(!AvailableWithTimeOut(3))
          return false;
        c = Read();
    }
  
    if(dudCount >= MAX_DUD_BYTES)
    {
          DealWithError(dudchars, 4, false);
          return false;
    }

    Status receivedStatus = (Status)Read();
    if(receivedStatus != success)
         DealWithError(receivedStatus, 5, true);
    for(int i = 0; i < DATA_LENGTH; i++)
    {
          if(!AvailableWithTimeOut(6))
            return false;
          str[i] = Read();
    }
    if(debug)
      Print(str, receivedStatus);
    return true;
}

void WPac::DealWithError(Status s, int flag, bool slave)
{
  if(s == success)
    return;
    
  if(slave)
  {
    Serial.print("\nSlave ");
    Serial.print(lastSlave);
    Serial.print(" error: ");
  } else
  {
    Serial.print("\nMaster error: ");
  }
  
  switch(s)
  {  
    case receivewait:
      Serial.print("time out waiting for input");
      break;

    case dudchars:
      Serial.print("dud input characters");
      break;

    case blocked:
      Serial.print("blocked");
      break;

    default:
      Serial.print("unknown error");
  }
  Serial.print(", flag: ");
  Serial.println(flag);
}

void  WPac::Spin()
{
  if(myAddress < 0)
    return;
  if(requestPending)
    RequestEvent();
}

void WPac::Print(unsigned char* str, Status s)
{
  Serial.print("*");
  Serial.print(s, HEX);
  for(int i = 0; i < DATA_LENGTH; i++)
  {
    Serial.print(" ");
    Serial.print(str[i], HEX);
  }
  //Serial.print(":    ");
  Serial.println();
}


void WPac::RequestEvent()
{

  requestPending = true;
  if(!sendReady)
    return;

  Write(START_CHAR);
  Status s = sendStatus;
  if(errorStatus != success)
  {
    s = errorStatus;
    errorStatus = success;
  }
  Write((char)s);
  for(int i = 0; i < DATA_LENGTH; i++)
    Write(dataToSend[i]);
  requestPending = false;
  sendReady = false;
  Print(dataToSend, s); 
}

void WPac::ReceiveEvent(int numBytes)
{
  if(!AvailableWithTimeOut(7))
    return;
       
  if(Read() != START_CHAR)
  {
    DealWithError(dudchars, 8, true);
    return;
  }
      
  if(!AvailableWithTimeOut(10))
    return;
       
  Status receivedStatus = (Status)Read();
    
  for(int i = 0; i < DATA_LENGTH; i++)
  {
    if(!AvailableWithTimeOut(9))
      return;
    dataReceived[i] = Read();    
  }
  dataAvailable = true; 
}






    


