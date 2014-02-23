#include <Teleinfo.h>
#include <SoftwareSerial.h>

SoftwareSerial cptSerial(9, 3);
Teleinfo teleinfo;

void setup() {
  teleinfo.setup();
}

void loop() 
{
  const char* buff = teleinfo.readFrame();
  Serial.println("*****************************************");
  Serial.write(buff);
  Serial.println("*****************************************");

  delay(1000);
}

