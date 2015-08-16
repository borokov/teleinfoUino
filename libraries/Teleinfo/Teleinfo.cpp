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
, m_verbose(false)
{
}

//-------------------------------------------
void
Teleinfo::setup(bool verbose)
{
  m_verbose = verbose;
  if ( m_verbose )
  {
    Serial.begin(1200);     // opens serial port, sets data rate to 1200 bps
    while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo only
    }
  }
  m_cptSerial.begin(1200);
}

//-------------------------------------------
Frame
Teleinfo::parse(const char* frameBuff)
{
  const char* ptr = frameBuff;
  Frame frame;

  while ( *ptr != ETX )
  {
    // got to begining of group
    while ( *ptr != LF && *ptr != ETX )
    {
      ptr++;
    }

    if ( *ptr == LF )
    {
      ptr++;
      char label[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
      char value[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
      parseGroupe(ptr, label, value);

      if ( strcmp(label, "HCHC") == 0 )
      {
        frame.HCHC = atol(value);
      }
      else if ( strcmp(label, "HCHP") == 0 )
      {
        frame.HCHP = atol(value);
      }
      else if ( strcmp(label, "IINST") == 0 )
      {
        frame.IINST = atoi(value);
      }
      else if ( strcmp(label, "IMAX") == 0 )
      {
        frame.IMAX = atoi(value);
      }
      else if ( strcmp(label, "PTEC") == 0 )
      {
        frame.PTEC[0] = value[0];
        frame.PTEC[1] = value[1];
        frame.PTEC[2] = '\0';
      }
    }

  }
  return frame;
}

//-------------------------------------------
void
Teleinfo::parseGroupe(const char*& ptr, char* label, char* value)
{
  if ( ptr[-1] != LF )
  {
    printError("ERROR parseGroupe: Group does not begin with LF\n");
    return;
  }
  char* ptrLabel = label;
  char* ptrValue = value;
  while ( *ptr != SP )
    *ptrLabel++ = *ptr++;

  ptr++;
  while ( *ptr != SP )
    *ptrValue++ = *ptr++;
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
      printError("ERROR tryReadFrame: EOT\n");
      return false;
    }
  } while(value != STX);
  // now value = STX

  char* ptr = buff;
  *ptr++ = value;

  while ( value != ETX )
  {
    // wait for begining of groupe
    while ( value != LF && value != ETX )
    {
      value = readValue();
      if ( value == EOT )
      {
        printError("ERROR tryReadFrame: EOT\n");
        return false;
      }
      *ptr++ = value;
    }
    // now value = LF or ETX

    // begining of groupe
    if ( value == LF )
    {
      if ( !readGroup(ptr) )
      {
        printError("ERROR tryReadFrame: Error while reading group.\n");
        return false;
      }
      if ( *ptr != CR )
      {
        printError("ERROR tryReadFrame: Bad end of group\n");
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
  if ( buff[-1] != LF )
  {
    printError("ERROR readGroup: Group does not start by SP\n");
    return false;
  }

  char value = 0;
  char* ptr = buff;
  int grpLen = 0;
  do
  {
    value = readValue();
    if ( value == EOT )
    {
      printError("ERROR readGroup: EOT\n");
      return false;
    }
    *ptr++ = value;
    grpLen++;
  } while ( value != CR );

  if ( cksum(buff, grpLen - 3) != buff[grpLen - 2] )
  {
    printError("ERROR readGroup: bad checksum\n");
    return false;
  }

  buff = ptr-1;
  return true;
}

//-------------------------------------------
char
Teleinfo::cksum(const char* buff, int buffLen)
{
  char sum = 0;
  for (int i = 0; i < buffLen; i++)
    sum = sum + buff[i];
  sum = (sum & 0x3F) + 0x20;

  return sum;
}

//-------------------------------------------
void
Teleinfo::printError(const char* msg)
{
  if ( m_verbose )
    Serial.write("ERROR readGroup: bad checksum\n");
}
