#ifndef _TELEINFO_
#define _TELEINFO_

#include <SoftwareSerial.h>

#define FRAME_BUFF_LEN 512

typedef struct Frame 
{
  char PTEC[3]; // tarif courant (HC\0, HP\0)
  long HCHC; // conso heure creuse
  long HCHP; // conso heure pleine
  int IINST;// intensite instantanee
  int IMAX; // intensite max
} Frame;

class Teleinfo
{
public:
  Teleinfo();

  void setup();

  // read a frame
  const char* readFrame();


private:
 
  // try to read creent frame. Return success.
  bool tryReadFrame(char* buff);

  // read a single value
  char readValue();

  // read a group. ptr must point to LF
  // return success
  bool readGroup(char*& ptr);

  // return checksum of buffer
  char cksum(const char* buff, int buffLen);


  SoftwareSerial m_cptSerial;
  char m_frameBuff[FRAME_BUFF_LEN];
};

#endif
