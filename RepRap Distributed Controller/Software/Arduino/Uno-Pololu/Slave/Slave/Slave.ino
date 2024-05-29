
#include "WirePacket.h"
#include "DDA.h"
#include "I2CSlave.h"

#define SLAVE_ADDRESS 18

I2CSlave* slave;
 
void setup() 
{
  slave = new I2CSlave(SLAVE_ADDRESS);     
}

void loop() 
{
  slave->Spin();
}




