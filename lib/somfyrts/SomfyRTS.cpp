#include "SomfyRTS.h"

RTSRemoteDevice::RTSRemoteDevice(uint32_t remoteID, uint8_t txPin, bool debug)
{
  _debug = debug;
  _remoteId = remoteID;
  _txPin = txPin;
}

RTSRemoteDevice::RTSRemoteDevice(uint32_t remoteID, uint8_t txPin)
{
  _debug = false;
  _remoteId = remoteID;
  _txPin = txPin;
}

void RTSRemoteDevice::begin(uint32_t *rollingCode)
{
  _rollingCode = rollingCode;
  pinMode(_txPin, OUTPUT);
  digitalWrite(_txPin, LOW);

  if (Serial)
  {
    Serial.print("Simulated remote number : ");
    Serial.println(_remoteId, HEX);
    Serial.print("Current rolling code    : ");
    Serial.println(*rollingCode);
  }
}

void RTSRemoteDevice::_buildFrame(unsigned char *frame, unsigned char button)
{
  unsigned int code = *_rollingCode;
  frame[0] = 0xA7;            // Encryption key. Doesn't matter much
  frame[1] = button << 4;     // Which button did  you press? The 4 LSB will be the checksum
  frame[2] = code >> 8;       // Rolling code (big endian)
  frame[3] = code;            // Rolling code
  frame[4] = _remoteId >> 16; // Remote address
  frame[5] = _remoteId >> 8;  // Remote address
  frame[6] = _remoteId;       // Remote address

  if (Serial)
  {
    Serial.print("Frame         : ");
    for (byte i = 0; i < 7; i++)
    {
      if (frame[i] >> 4 == 0)
      {                    // Displays leading zero in case the most significant
        Serial.print("0"); // nibble is a 0.
      }
      Serial.print(frame[i], HEX);
      Serial.print(" ");
    }
  }

  // Checksum calculation: a XOR of all the nibbles
  checksum = 0;
  for (byte i = 0; i < 7; i++)
  {
    checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
  }
  checksum &= 0b1111; // We keep the last 4 bits only

  //Checksum integration
  frame[1] |= checksum; //  If a XOR of all the nibbles is equal to 0, the blinds will
                        // consider the checksum ok.

  if (Serial)
  {
    Serial.println("");
    Serial.print("With checksum : ");
    for (byte i = 0; i < 7; i++)
    {
      if (frame[i] >> 4 == 0)
      {
        Serial.print("0");
      }
      Serial.print(frame[i], HEX);
      Serial.print(" ");
    }
  }

  // Obfuscation: a XOR of all the bytes
  for (byte i = 1; i < 7; i++)
  {
    frame[i] ^= frame[i - 1];
  }

  if (Serial)
  {
    Serial.println("");
    Serial.print("Obfuscated    : ");
    for (byte i = 0; i < 7; i++)
    {
      if (frame[i] >> 4 == 0)
      {
        Serial.print("0");
      }
      Serial.print(frame[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");
    Serial.print("Rolling Code  : ");
    Serial.println(code);
  }

  //  We store the value of the rolling code in the FS
  *_rollingCode = code + 1;
  // _writeRemoteRollingCode(code + 1);
}

void RTSRemoteDevice::_sendCommand(unsigned char *frame, unsigned char sync)
{
  if (sync == 2)
  { // Only with the first frame.
    // Wake-up pulse & Silence
    digitalWrite(_txPin, HIGH);
    delayMicroseconds(9415);
    digitalWrite(_txPin, LOW);
    delayMicroseconds(89565);
  }

  // Hardware sync: two sync for the first frame, seven for the following ones.
  for (int i = 0; i < sync; i++)
  {
    digitalWrite(_txPin, HIGH);
    delayMicroseconds(4 * SYMBOL);
    digitalWrite(_txPin, LOW);
    delayMicroseconds(4 * SYMBOL);
  }

  // Software sync
  digitalWrite(_txPin, HIGH);
  delayMicroseconds(4550);
  digitalWrite(_txPin, LOW);
  delayMicroseconds(SYMBOL);

  //Data: bits are sent one by one, starting with the MSB.
  for (byte i = 0; i < 56; i++)
  {
    if (((frame[i / 8] >> (7 - (i % 8))) & 1) == 1)
    {
      digitalWrite(_txPin, LOW);
      delayMicroseconds(SYMBOL);
      // PORTD ^= 1<<5;
      digitalWrite(_txPin, HIGH);
      delayMicroseconds(SYMBOL);
    }
    else
    {
      digitalWrite(_txPin, HIGH);
      delayMicroseconds(SYMBOL);
      // PORTD ^= 1<<5;
      digitalWrite(_txPin, LOW);
      delayMicroseconds(SYMBOL);
    }
  }

  digitalWrite(_txPin, LOW);
  delayMicroseconds(30415); // Inter-frame silence
}

void RTSRemoteDevice::sendCommandUp()
{
  _buildFrame(_frame, HAUT);
  _sendCommand(_frame, 2);
  for (int i = 0; i < 2; i++)
  {
    _sendCommand(_frame, 7);
  }
}

void RTSRemoteDevice::sendCommandDown()
{
  _buildFrame(_frame, BAS);
  _sendCommand(_frame, 2);
  for (int i = 0; i < 2; i++)
  {
    _sendCommand(_frame, 7);
  }
}

void RTSRemoteDevice::sendCommandStop()
{
  _buildFrame(_frame, STOP);
  _sendCommand(_frame, 2);
  for (int i = 0; i < 2; i++)
  {
    _sendCommand(_frame, 7);
  }
}

void RTSRemoteDevice::sendCommandProg()
{
  _buildFrame(_frame, PROG);
  _sendCommand(_frame, 2);
  for (int i = 0; i < 2; i++)
  {
    _sendCommand(_frame, 7);
  }
}
