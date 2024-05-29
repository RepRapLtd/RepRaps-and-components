/*
 * Slave class for I2C control of distributed devices round a
 * RepRap machine.
 * 
 * Adrian Bowyer
 * 
 * 21 October 2016
 */

void StepInterrupt()
{
  slave->Step();
}

I2CSlave::I2CSlave(int address)
{
  myAddress = address;

  // Set PWM to work at high frequency
  
  TCCR2B = TCCR2B & B11111000 | B00000001;  

  pinMode(pinREF, OUTPUT);
  pinMode(pinSTEP, OUTPUT);
  digitalWrite(pinSTEP, 1);
  pinMode(pinDIR, OUTPUT);
  pinMode(pinClock, INPUT_PULLUP);

  wirePacket = new WPac(myAddress);

  // Build the DDA ring
  
  nextDDA = 0;
  for(int i = 0; i < DDA_COUNT; i++)
  {
    nextDDA = new DDA(nextDDA, this);
    if(!i)
      activeDDA = nextDDA;
  }
  activeDDA->SetNext(nextDDA);

  // Clear the data
  
  for(int i = 0; i < DATA_LENGTH; i++)
    data[i] = 0;
  coordinate = 0;
  SetRef(50);
  
  Serial.begin(115200);
  Serial.print("\n\nSlave address ");
  Serial.print(myAddress);
  Serial.println(" started.");
  
  sei();      
}


void I2CSlave::SetRef(float REFVoltage)
{
  voltagePWM = round(REFVoltage/PWMMod);
  analogWrite(pinREF, voltagePWM);   
  Serial.print("REFVoltage set to: "); Serial.println(REFVoltage);
}

void I2CSlave::PutData(unsigned char d[], long v)
{
  for (int i = 3; i >= 0; i--)
  {
    d[i] = v & 0xff;
    v = v >> 8;
  }  
}

long I2CSlave::GetData(unsigned char d[])
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



void I2CSlave::Act()
{
  long value;
   
  switch(data[0])
  {
    case deltaC:
      value = GetData(&data[1]);
      nextDDA->SetDelta(value);
      break;

    case stepsC:
      value = GetData(&data[1]);
      nextDDA->SetTotal(value);
      break;

    case intervalC:
      value = GetData(&data[1]);
      nextDDA->SetInterval(value);
      break;

    case currentC:
      value = GetData(&data[1]);
      SetRef((float)value);
      break;

    case lastC:
      wirePacket->Send(oldData, success);
      break;

    case getC:
      PutData(&data[1], coordinate);
      wirePacket->Send(data, success);
      break;
      
    default:
      Serial.print(data[0], HEX);
      Serial.println(" is not a valid action");
  }
}

void I2CSlave::Spin()
{
  wirePacket->Spin();

  activeDDA->Spin();

  // Deal with the case where we are starting a new sequence and we're the clock

  if(!activeDDA->Set())
  {
    if(nextDDA->IAmTheClock())  // Rest of nextDDA must be set if this is true
    {
      activeDDA = nextDDA;
      nextDDA = activeDDA->Next();
      if(!activeDDA->Activate())
        Error("Attempting to activate unready DDA 3!");
    }
  }
  
  if(wirePacket->Get(data))
  {
    wirePacket->Print(data, success);
    Act();
    for(int i = 0; i < DATA_LENGTH; i++)
      oldData[i] = data[i];
  }  
}




