#ifndef _TELEINFO_
#define _TELEINFO_

#include <SoftwareSerial.h>

// max len of a frame
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

  // must be called in setup()
  // If verbose is true, will print information in serial.
  // Serial shoulf br at 1200 baud
  void setup(bool verbose);

  // read a frame. This call is blocking until a frame is properly read
  const char* readFrame();

  // create a frame from framebuffer
  Frame parse(const char* frameBuff);

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

  void parseGroupe(const char*& buff, char* label, char* value);

  // print error on serial if m_verbose is true
  void printError(const char* msg);

  SoftwareSerial m_cptSerial;
  char m_frameBuff[FRAME_BUFF_LEN];
  bool m_verbose;
};

#endif
