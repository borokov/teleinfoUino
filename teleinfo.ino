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

  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void loop() 
{
  // try to read and send EDF conso until succeed
  bool ok = false;
  do
  {
    const char* buff = teleinfo.readFrame();
    Frame frame = teleinfo.parse(buff);
    ok = send(frame);
  }
  while ( !ok );

  // Wait for 1h.
  // For some reason my Arduino board seems 3 times slower than expected.
  // I didn't find any solution, so wait for 20min to have 1h real life.
  delay(20UL * 60UL * 1000UL);
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

  char buff1[32];
  sprintf(buff1, "hc=%ld&hp=%ld", frame.HCHC, frame.HCHP);
  int buff1Len = strlen(buff1);
  char buff2[32];
  sprintf(buff2, "Content-Length: %d\r\n", buff1Len);

  client.println(buff2);
  client.println(buff1);
  client.println();

  // now we will read client response. Response should contains a pattern to be valid.
  // if pattern if found, it means success, else something gone wrong.
  char* pattern = "done";
  // current char in pattern.
  char* patternChar = pattern;
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

