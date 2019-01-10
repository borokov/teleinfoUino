#include <Teleinfo.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>

SoftwareSerial cptSerial(9, 3);
Teleinfo teleinfo;

EthernetClient client;

void setup() 
{
  teleinfo.setup(false);
  // mac adress of server (the arduino board)
  byte mac[] = {  0x90, 0xA2, 0xDA, 0x0E, 0xC0, 0x92 };
  Ethernet.begin(mac); // return 0 if failled
  pinMode(LED_BUILTIN, OUTPUT);
  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void loop() 
{
  // Led is ON during frame reading and OFF while waiting 1h
  digitalWrite(LED_BUILTIN, HIGH);

  // try to read and send EDF conso until succeed
  bool ok = false;
  do
  {
    Frame frame = teleinfo.getFrame();
    ok = send(frame);
  }
  while ( !ok );

  digitalWrite(LED_BUILTIN, LOW);

}

bool send(const Frame& frame)
{
  // connect to server
  client.connect("192.168.1.2", 80);
  
  // Make a HTTP POST request:
  // serverIp/EDF/doAdd.php?hc=12345678&hp=12345678
  client.println("POST /EDF/doAdd.php HTTP/1.1");
  client.println("Host: 192.168.1.2");
  client.println("Connection: close");
  client.println("Content-Type: application/x-www-form-urlencoded");

  char buff1[64];
  sprintf(buff1, "hc=%ld&hp=%ld&iinst=%d", frame.HCHC, frame.HCHP, frame.IINST);
  Serial.write(buff1);
  int buff1Len = strlen(buff1);
  char buff2[32];
  sprintf(buff2, "Content-Length: %d\r\n", buff1Len);

  client.println(buff2);
  client.println(buff1);
  client.println();

  // now we will read client response. Response should contains a pattern to be valid.
  // if pattern if found, it means success, else something gone wrong.
  const char* pattern = "done";
  // current char in pattern.
  const char* patternChar = pattern;
  bool patternFound = false;
  while(client.connected())
  {
    while(client.available())
    {
      char inChar = client.read();
      // if current response char == current pattern char
      if ( inChar == *patternChar )
      {
        // go to next pattern char
        patternChar++;
        // if we reached end of pattern Bingo !
        if ( patternChar - pattern >= strlen(pattern) )
        {
          patternFound = true;
          break;
        }
      }
      // if current response char != current pattern char, go back to begining of patyern
      else
      {
        patternChar = pattern;
      }
    }
  }

  client.stop();

  return patternFound;
}

void print(const Frame& frame)
{
  Serial.write("\nPTEC: ");
  Serial.println(frame.PTEC);
  Serial.write("HC: ");
  Serial.println(frame.HCHC);
  Serial.write("HP: ");
  Serial.println(frame.HCHP);
  Serial.write("IINST: ");
  Serial.println(frame.IINST);
  Serial.write("IMAX: ");
  Serial.println(frame.IMAX);
}

