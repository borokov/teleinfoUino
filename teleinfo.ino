//#include <Teleinfo.h>
//#include <SoftwareSerial.h>

SoftwareSerial cptSerial(9, 3);
Teleinfo teleinfo;
dfezgfez
void setup() {
  //teleinfo.setup();
  cptSerial.begin(1200);
}

void loop() 
{
 teleinfo.readFrame();
  delay(1000);
}

