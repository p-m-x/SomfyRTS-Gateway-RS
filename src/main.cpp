#include <Arduino.h>
#include <util/crc16.h>
#include <SomfyRTS.h>
#include <EEPROM.h>

// CLEAR_DEVICES_ROLLING_CODES - set to 1 if want to clear all rollingCodes.
#define CLEAR_DEVICES_ROLLING_CODES 0

#define SERIAL_BAUND_RATE 9600
#define SERIAL_WAIT_FOR_CMD_TIME 1000
#define PREAMBULE 0xF0F0
#define TX_PIN 11
#define DEVICES_FIRST_ADDRESS 0x121300
#define DEVICES_COUNT 10
#define EEPROM_ROLLING_CODE_START_ADDRESS 0

struct DeviceCmd
{
  int deviceIdx;
  String cmd;
};

String serialInputString = "";
bool serialInputComplete = false;
unsigned long serialLastCharTime = 0;
RTSRemoteDevice devices[DEVICES_COUNT] = {
    RTSRemoteDevice(DEVICES_FIRST_ADDRESS, TX_PIN),
    RTSRemoteDevice(DEVICES_FIRST_ADDRESS + 1, TX_PIN),
    RTSRemoteDevice(DEVICES_FIRST_ADDRESS + 2, TX_PIN),
    RTSRemoteDevice(DEVICES_FIRST_ADDRESS + 3, TX_PIN),
    RTSRemoteDevice(DEVICES_FIRST_ADDRESS + 4, TX_PIN),
    RTSRemoteDevice(DEVICES_FIRST_ADDRESS + 5, TX_PIN),
    RTSRemoteDevice(DEVICES_FIRST_ADDRESS + 6, TX_PIN),
    RTSRemoteDevice(DEVICES_FIRST_ADDRESS + 7, TX_PIN),
    RTSRemoteDevice(DEVICES_FIRST_ADDRESS + 8, TX_PIN),
    RTSRemoteDevice(DEVICES_FIRST_ADDRESS + 9, TX_PIN),
};
uint32_t rollingCodes[DEVICES_COUNT];

// initialRollingCodes - change it to your need to simulate existed device
uint32_t initialRollingCodes[DEVICES_COUNT] = {237, 255, 253, 320, 552, 314, 284, 474, 1, 1};

void storeRollingCode(uint8_t deviceIdx, uint32_t rollingCode);
void initializeRollingCodes();
void printRollingCodes();
void parseCommandString(String str, int &deviceIdx, String &cmd);

void setup()
{
  Serial.begin(SERIAL_BAUND_RATE);
  EEPROM.begin();

  initializeRollingCodes();

  // devices init
  for (uint8_t i = 0; i < DEVICES_COUNT; i++)
  {
    devices[i].begin(&rollingCodes[i]);
  }
}

void loop()
{
  if (serialLastCharTime > 0 && serialLastCharTime + SERIAL_WAIT_FOR_CMD_TIME < millis())
  {
    serialInputComplete = true;
  }

  if (serialInputComplete)
  {
    serialLastCharTime = 0;
    serialInputComplete = false;
    int deviceId = -1;
    String cmd = "";
    parseCommandString(serialInputString, deviceId, cmd);
    serialInputString = "";

    if (deviceId < 0)
    {
      return;
    }
    if (cmd.equals("up"))
    {
      devices[deviceId].sendCommandUp();
    }
    else if (cmd.equals("stop"))
    {
      devices[deviceId].sendCommandStop();
    }
    else if (cmd.equals("down"))
    {
      devices[deviceId].sendCommandDown();
    }
    else if (cmd.equals("prog"))
    {
      devices[deviceId].sendCommandProg();
    }
    else
    {
      return;
    }
    storeRollingCode(deviceId, rollingCodes[deviceId]);
  }
}

void initializeRollingCodes()
{
  if (CLEAR_DEVICES_ROLLING_CODES)
  {
    for (uint8_t i = 0; i < DEVICES_COUNT; i++)
    {
      storeRollingCode(i, 0);
    }
  }
  EEPROM.get(EEPROM_ROLLING_CODE_START_ADDRESS, rollingCodes);
  // printRollingCodes();
  for (uint8_t i = 0; i < DEVICES_COUNT; i++)
  {
    if (rollingCodes[i] < initialRollingCodes[i])
    {
      storeRollingCode(i, initialRollingCodes[i]);
    }
  }
  EEPROM.get(EEPROM_ROLLING_CODE_START_ADDRESS, rollingCodes);
  // printRollingCodes();
}

void storeRollingCode(uint8_t deviceIdx, uint32_t rollingCode)
{
  //Decomposition from a 4byte var to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (rollingCode & 0xFF);
  byte three = ((rollingCode >> 8) & 0xFF);
  byte two = ((rollingCode >> 16) & 0xFF);
  byte one = ((rollingCode >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  int address = EEPROM_ROLLING_CODE_START_ADDRESS + (deviceIdx * sizeof(uint32_t));
  EEPROM.update(address, four);
  EEPROM.update(address + 1, three);
  EEPROM.update(address + 2, two);
  EEPROM.update(address + 3, one);
}

void parseCommandString(String str, int &deviceIdx, String &cmd)
{
  int sepIdx = str.indexOf('|');
  if (sepIdx <= 0)
  {
    return;
  }

  deviceIdx = atoi(str.substring(0, sepIdx).c_str());
  cmd = str.substring(sepIdx + 1, str.length());
}

void printRollingCodes()
{
  for (uint8_t i = 0; i < DEVICES_COUNT; i++)
  {
    Serial.print("Device #");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(rollingCodes[i]);
  }
}

void serialEvent()
{
  while (Serial.available())
  {
    char inChar = (char)Serial.read();
    if (inChar == '\n')
    {
      serialInputComplete = true;
      return;
    }
    serialLastCharTime = millis();
    serialInputString += inChar;
  }
}