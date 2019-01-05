#include "Teleinfo.h"
#include "Arduino.h"

#define STX 0x02 // Start Text. Start of frame.
#define ETX 0x03 // End Text. End of frame.
#define EOT 0x04 // End Of Text. Interuption.

#define LF 0x0A // Line Feed. Start of group.
#define SP 0x20 // SPace. Space between label and data.
#define CR 0x0D // Carriage Return. End of group.

//-------------------------------------------
Frame::Frame()
{
  memset(PTEC, 0, 3);
  HCHC = -1;
  HCHP = -1;
  IINST = -1;
  IMAX = -1;
  m_validBits = 0;
}

//-------------------------------------------
bool
Frame::isValid()
{
  return m_validBits == 0x1F; //b00011111
}

//-------------------------------------------
void
Frame::fill(char* buff, char buffLen)
{
  // buff = (label)SP(value)SP(checksum)CR

  // pointer to begining of label
  char* labelPtr = buff;
  // pointer to begining of value
  char* valuePtr = NULL;
  // volatil pointer
  char* ptr = buff;

  // Find label
  while ( *ptr != SP && ptr - buff < buffLen )
    *ptr++;

  if ( ptr - buff >= buffLen )
  {
    //printError("Frame::fill-1 buffer overflow");
    return;
  }

  // Replace SP char by \0 so that we can use strcmp function
  *ptr = '\0';
  ptr++;

  valuePtr = ptr;

  // Find value
  while ( *ptr != SP && ptr - buff < buffLen )
    *ptr++;

  if ( ptr - buff >= buffLen )
  {
    //printError("Frame::fill-2 buffer overflow");
    return;
  }

  // Replace SP char by \0 so that we can use strcmp function
  *ptr = '\0';

  // now buff = (label)\0(value)\0(checksum)CR
  //            |        |
  //         labelPtr  valuePtr

  if ( !HCHCValid() && strcmp(buff, "HCHC") == 0 )
  {
    HCHC = atol(valuePtr);
    setHCHCValid();
    // printLog("HCH Filled\n");
    return;
  }
  else if ( !HCHPValid() && strcmp(labelPtr, "HCHP") == 0 )
  {
    HCHP = atol(valuePtr);
    setHCHPValid();
    //printLog("HCHP Filled\n");
    return;
  }
  else if ( !IINSTValid() && strcmp(labelPtr, "IINST") == 0 )
  {
    IINST = atoi(valuePtr);
    setIINSTValid();
    //printLog("IINST Filled\n");
    return;
  }
  else if ( !IMAXValid() && strcmp(labelPtr, "IMAX") == 0 )
  {
    IMAX = atoi(valuePtr);
    setIMAXValid();
    //printLog("IMAX Filled\n");
    return;
  }
  else if ( !PTECValid() && strcmp(labelPtr, "PTEC") == 0 )
  {
    PTEC[0] = valuePtr[0];
    PTEC[1] = valuePtr[1];
    PTEC[2] = '\0';
    setPTECValid();
    //printLog("PTEC Filled\n");
    return;
  }

}

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
Teleinfo::getFrame()
{
  memset(m_lineBuff, '\0', LINE_BUFF_LEN);
  Frame frame;

  // Connexion is really poor and I hardly manage to have a complete
  // valid frame. So the idea is:
  // For each received line, try to fill information in Frame.
  // Once Frame is completly filled, return it
  // This may require several frame to fully fill Frame.
  while(!frame.isValid())
  {
    waitStartGroup();
    int buffLen = readGroup(m_lineBuff, LINE_BUFF_LEN);
    if ( buffLen > 0 )
    {
      frame.fill(m_lineBuff, buffLen);
    }
  }
  return frame;
}

//-------------------------------------------
void
Teleinfo::waitStartGroup()
{
  char value = 0;
  // wait for begining of frame
  do {
    value = readValue();
    printLog(value);
    if ( value == EOT )
    {
      printError("waitStartGroup: EOT");
    }
  } while(value != LF);
  printLog("LOG: New Group\n");
}

//-------------------------------------------
char
Teleinfo::readValue()
{
  while(!m_cptSerial.available()) {}
  return m_cptSerial.read() & 0x7F;
}

//-------------------------------------------
char
Teleinfo::readGroup(char* buff, char buffLen)
{
  char value = 0;
  char* ptr = buff;
  char grpLen = 0;
  char wantedChecksum = 0;
  char computedChecksum = 0;
  do
  {
    value = readValue();
    printLog(value);
    if ( value == EOT )
    {
      printError("ERROR readGroup: EOT\n");
      return 0;
    }
    *ptr++ = value;
    grpLen++;
  } while ( value != CR && grpLen < buffLen );

  if ( grpLen <= 3 )
  {
    printError("readGroup: Not enough character in group.\n");
    return 0;
  }

  wantedChecksum = buff[grpLen - 2];
  computedChecksum = cksum(buff, grpLen - 3);
  if ( computedChecksum != wantedChecksum )
  {
    printError("ERROR readGroup: bad checksum\n");
    return 0;
  }

  return grpLen;
}

//-------------------------------------------
char
Teleinfo::cksum(const char* buff, char buffLen)
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
  {
    Serial.write("ERROR: ");
    Serial.write(msg);
    Serial.write('\n');
  }

}

//-------------------------------------------
void
Teleinfo::printLog(const char* msg)
{
  if ( m_verbose )
  {
    Serial.write(msg);
  }
}

//-------------------------------------------
void
Teleinfo::printLog(const char msg)
{
  if ( m_verbose )
  {
    Serial.write(msg);
  }
}
