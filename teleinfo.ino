#include <Teleinfo.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>

SoftwareSerial cptSerial(9, 3);
Teleinfo teleinfo;

EthernetClient client;

void setup() 
{
  teleinfo.setup();

  // mac adress of server
  byte mac[] = {  0x90, 0xA2, 0xDA, 0x0E, 0xC0, 0x92 };
  Ethernet.begin(mac); // return 0 if failled
  
  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void loop() 
{
  const char* buff = teleinfo.readFrame();
  Frame frame = teleinfo.parse(buff);

  send(frame);

  // wait for some day
  delay(24UL * 60UL * 60UL * 1000UL);
}

void send(const Frame& frame)
{
  // connect to NAS
  client.connect("192.168.1.2", 80);
  
  // Make a HTTP request:
  client.println("POST /EDF/doAdd.php HTTP/1.1");
  client.println("Host: 192.168.1.2");
  client.println("Connection: close");
  client.println("Content-Type: application/x-www-form-urlencoded");

  char buff1[64];
  sprintf(buff1, "hc=%ld&hp=%ld", frame.HCHC, frame.HCHP);
  int buff1Len = strlen(buff1);
  char buff2[64];
  sprintf(buff2, "Content-Length: %d\r\n", buff1Len);

//  client.println("Content-Length: 10\r\n");
  client.println(buff2);
//  client.println("hc=5&hp=10");
  client.println(buff1);
  client.println();
  
  while(client.connected())
  {
    while(client.available())
    {
      int inChar = client.read();
      //Serial.write(inChar);
    }
  }
  
  client.stop();
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

