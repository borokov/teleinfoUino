#include "Teleinfo.h"
#include "Arduino.h"

#define STX 0x02 // Start Text. Start of frame.
#define ETX 0x03 // End Text. End of frame.
#define EOT 0x04 // End Of Text. Interuption.

#define LF 0x0A // Line Feed. Start of group.
#define SP 0x20 // SPace. Space between label and data.
#define CR 0x0D // Carriage Return. End of group.

//-------------------------------------------
Teleinfo::Teleinfo()
: m_cptSerial(9, 3)
{
}

//-------------------------------------------
void 
Teleinfo::setup() 
{
  Serial.begin(1200);     // opens serial port, sets data rate to 1200 bps
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  m_cptSerial.begin(1200);
}

//-------------------------------------------
const char* 
Teleinfo::readFrame()
{
  memset(m_frameBuff, '\0', FRAME_BUFF_LEN);

  bool success = false;
  do
  {
    success = tryReadFrame(m_frameBuff);
  } while ( !success );

  return m_frameBuff;
}

//-------------------------------------------
bool 
Teleinfo::tryReadFrame(char* buff)
{
  char value = 0;
  // wait for begining of frame
  do {
    value = readValue();
    if ( value == EOT )
    {
      Serial.write("ERROR tryRedFrame: EOT\n");
      return false;
    }
  } while(value != STX);
  // now value = STX

  char* ptr = buff;
  *ptr++ = value;

  while ( value != ETX )
  {
    // wait for begining of groupe
    while ( value != SP && value != ETX )
    {
      value = readValue();
      if ( value == EOT )
      {
        Serial.write("ERROR tryRedFrame: EOT\n");
        return false;
      }
      *ptr++ = value;
    }
    // now value = SP or ETX

    // begining of groupe 
    if ( value == SP )
    {
      if ( !readGroup(ptr) )
      {
        Serial.write("ERROR tryReadFrame: Arror while reading group.\n");
        return false;
      }
      if ( *ptr != CR )
      {
        Serial.write("ERROR tryReadFrame: Bad end of group\n");
        return false;
      }
      value = *ptr;
    }

  }

  return true;
}

//-------------------------------------------
char
Teleinfo::readValue()
{
  while(!m_cptSerial.available()) {}
  return m_cptSerial.read() & 0x7F;
}

//-------------------------------------------
bool 
Teleinfo::readGroup(char*& buff)
{
  if ( *buff != SP )
  {
    Serial.write("ERROR readGroup: Group does not start by SP\n");
    return false;
  }

  char value = 0;
  char* ptr = buff;
  int grpLen = 1;
  do
  {
    value = readValue();
    if ( value == EOT )
    {
      Serial.write("ERROR readGroup: EOT\n");
      return false;
    }
    *ptr++ = value;
    grpLen++;
  } while ( value != CR );

  if ( cksum(buff, grpLen - 2) != buff[grpLen - 2] )
  {
    Serial.write("ERROR readGroup: bad checksum\n");
    return false;
  }

  buff = ptr;
  return true;
}

//-------------------------------------------
char
Teleinfo::cksum(const char* buff, int buffLen)
{
  char sum = 0;
  for (int i = 1; i < buffLen; i++)
    sum = sum + buff[i];
  sum = (sum & 0x3F) + 0x20;
  return sum;
}

