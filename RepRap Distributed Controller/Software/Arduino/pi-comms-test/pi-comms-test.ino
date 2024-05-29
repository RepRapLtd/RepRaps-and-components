
#include <Wire.h>


void setup() {
  // put your setup code here, to run once:
  Wire.begin(8);
  Serial.begin(115200); 
  Serial.print("\nStart:\n");
  Wire.onRequest(requestEvent);
}

void loop() 
{
  char c;
  // put your main code here, to run repeatedly:
  if(Wire.available())
  {
    c = Wire.read();
    Serial.print(c);
  }

}

void requestEvent() {

}
