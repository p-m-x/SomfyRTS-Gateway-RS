#ifndef somfyrts_h
#define somfyrts_h
#include <Arduino.h>

#define SYMBOL 640
#define HAUT 0x2
#define STOP 0x1
#define BAS 0x4
#define PROG 0x8

class RTSRemoteDevice
{

private:
  bool _debug;
  uint8_t _txPin;
  uint32_t _remoteId;
  unsigned char _frame[7];
  char checksum;
  uint32_t *_rollingCode;

  void _buildFrame(unsigned char *frame, unsigned char button);
  void _sendCommand(unsigned char *frame, unsigned char sync);

public:
  RTSRemoteDevice(uint32_t remoteID, uint8_t txPin, bool debug);
  RTSRemoteDevice(uint32_t remoteID, uint8_t txPin);
  void begin(uint32_t *rollingCode);
  void sendCommandUp();
  void sendCommandDown();
  void sendCommandStop();
  void sendCommandProg();
};

#endif