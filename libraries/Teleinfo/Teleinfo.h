#ifndef _TELEINFO_
#define _TELEINFO_

#include <SoftwareSerial.h>

// max len of a frame
#define FRAME_BUFF_LEN 512

// max len of a line (<=>group) in frame
#define LINE_BUFF_LEN 32

class Frame
{
public:
  Frame();

  bool isValid();

  void fill(char* buff, char buffLen);

  char PTEC[3]; // tarif courant (HC\0, HP\0)
  long HCHC; // conso heure creuse
  long HCHP; // conso heure pleine
  int IINST;// intensite instantanee
  int IMAX; // intensite max

private:

  bool PTECValid() { return m_validBits & 0x01; }
  setPTECValid() { m_validBits |= 0x01; }

  bool HCHCValid() { return m_validBits & 0x02; }
  setHCHCValid() { m_validBits |= 0x02; }

  bool HCHPValid() { return m_validBits & 0x04; }
  setHCHPValid() { m_validBits |= 0x04; }

  bool IINSTValid() { return m_validBits & 0x08; }
  setIINSTValid() { m_validBits |= 0x08; }

  bool IMAXValid() { return m_validBits & 0x10; }
  setIMAXValid() { m_validBits |= 0x10; }

  char m_validBits;


};

class Teleinfo
{
public:
  Teleinfo();

  // must be called in setup()
  // If verbose is true, will print information in serial.
  // Serial shoulf br at 1200 baud
  void setup(bool verbose);

  Frame getFrame();

private:

  void waitStartGroup();

  // try to read creent frame. Return success.
  bool tryReadFrame(char* buff);

  // read a single value
  char readValue();

  // read a group. ptr must point to LF
  // nb of char written to buff
  char readGroup(char* buff, char buffLen);

  // return checksum of buffer
  char cksum(const char* buff, char buffLen);

  // print error on serial if m_verbose is true
  void printError(const char* msg);
  void printLog(const char* msg);
  void printLog(const char msg);

  SoftwareSerial m_cptSerial;
  char m_lineBuff[LINE_BUFF_LEN];
  bool m_verbose;
};

#endif
