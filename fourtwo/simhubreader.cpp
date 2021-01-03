#include "simhubreader.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

SimHubReader::SimHubReader() {
}

SimHubReader::~SimHubReader() {
}

void SimHubReader::tick(unsigned long  nowMs) {
  if (EEBlue.available()) {
    int readCount = EEBlue.readBytesUntil('#', buf, MESSAGE_BUF_SIZE);
    buf[min(readCount, MESSAGE_BUF_SIZE - 1)] = 0x0;
    //            Serial.println(buf);
    processMessage(nowMs);
    memset(buf, 0x0, MESSAGE_BUF_SIZE);
  }
}

void SimHubReader::begin() {
  EEBlue.begin("W7N-DASH");

  initProperties();
}

void SimHubReader::processMessage(unsigned long  nowMs) {
  char msgType = buf[0];

  State newState = state;

  switch (msgType) {
    case 's':
      newState = ConnectedToSimHub;
      break;
    case 'e':
      newState = NoConnection;
      break;
    case 'F':
      cleanBufferDigits(2);
      setFlags();
      break;
    case 'T':
      setTcLevel(nowMs);
      break;
    case 'Q':
      setAbsLevel(nowMs);
      break;
    case 'M':
      setMapLevel(nowMs);
      break;
    case 'I':
      setBrakeBiasLevel(nowMs);
      break;
    case 'P':
      cleanBufferDigits(2);
      newState = buf[1] == '0' ? ConnectedToSimHub : Driving;
      break;
    case 'R':
      cleanBufferDigits(2);
      sscanf(&buf[1], "%d", &rpm);
      break;
    case 'G':
      cleanBufferGear(2);
      newState = Driving;
      gear = buf[1];
      break;
    case 'S':
      memset(spd, 0x0, 4);
      cleanBufferDigits(2);
      memcpy(&spd, &buf[1], 3);
      break;
    case 'D':
      strncpy(floatBuf, &buf[1], FLOAT_BUF_SIZE);
      lapDelta = atof(floatBuf);
      break;
    case 'A':
      strncpy(floatBuf, &buf[1], FLOAT_BUF_SIZE);
      gapAhead = atof(floatBuf);
      break;
    case 'B':
      strncpy(floatBuf, &buf[1], FLOAT_BUF_SIZE);
      gapBehind = atof(floatBuf);
      break;
    case 'L':
      memset(lapTime, 0, LAP_TIME_STRLEN);
      strncpy(lapTime, &buf[4], min(LAP_TIME_STRLEN - 1, (int)strlen(&buf[4])));
      break;
    default:
      break;
  }

  if (newState != state) {
    state = newState;
    stateChangeTime = nowMs;
  }
  lastMessageTime = nowMs;
}


void SimHubReader::cleanBufferDigits(int startOffset) {
  for (int i = startOffset; i < MESSAGE_BUF_SIZE; i++) {
    if (!isdigit(buf[i])) {
      buf[i] = 0x0;
      break;
    }
  }
}

void SimHubReader::cleanBufferLap(int startOffset) {
  for (int i = startOffset; i < MESSAGE_BUF_SIZE; i++) {
    if (!isdigit(buf[i]) || buf[i] != ':') {
      buf[i] = 0x0;
      break;
    }
  }
}

void SimHubReader::cleanBufferGear(int startOffset) {
  for (int i = startOffset; i < MESSAGE_BUF_SIZE; i++) {
    if (!isdigit(buf[i]) || buf[i] != 'n' || buf[i] != 'N' || buf[i] != 'r' || buf[i] != 'R') {
      buf[i] = 0x0;
      break;
    }
  }
}

void SimHubReader::setFlags() {
  yellowFlag = buf[1] != '0';
  blueFlag = buf[2] != '0';
  pitLimiter = buf[3] != '0';
}

void SimHubReader::setTcLevel(unsigned long ms) {
  tcLevel = atoi(&buf[1]);
}

void SimHubReader::setMapLevel(unsigned long ms) {
  mapLevel = atoi(&buf[1]);
}
void SimHubReader::setAbsLevel(unsigned long ms) {
  absLevel = abs(atoi(&buf[1]));
}

void SimHubReader::setBrakeBiasLevel(unsigned long ms) {
  brakeBias = atof(&buf[1]);
}

void SimHubReader::initProperties() {
  gear = '7';
  rpm = 0;
  strcpy(spd, "227");
  lapDelta = -10;
  strcpy(lapTime, "18:88.888");
  strcpy(lastLapTime, "--:--.--");
  strcpy(bestLapTime, "18:88.888");
  gapAhead = -1.0;
  gapBehind = 2.0;
  yellowFlag = false;
  blueFlag = false;
  pitLimiter = false;
  tcLevel = 2;
  absLevel = 3;
  mapLevel = 3;
  drsAvailable = false;
  drsActive = false;
  brakeBias = 52;
}

char SimHubReader::getGear() {
  return gear;
}

char* SimHubReader::getLapTime() {
  return lapTime;
}

char* SimHubReader::getLastLapTime() {
  return lastLapTime;
}

char* SimHubReader::getBestLapTime() {
  return bestLapTime;
}
char* SimHubReader::getSpeed() {
  return spd;
}

int SimHubReader::getRpm() {
  return rpm;
}
int SimHubReader::getTcLevel() {
  return tcLevel;
}
int SimHubReader::getAbsLevel() {
  return absLevel;
}
int SimHubReader::getMapLevel() {
  return mapLevel;
}

int SimHubReader::getPosition() {
  return position;
}
int SimHubReader::getLapNumber() {
  return lapNumber;
}
bool SimHubReader::isYellowFlag() {
  return yellowFlag;
}
bool SimHubReader::isBlueFlag() {
  return blueFlag;
}
bool SimHubReader::isPitLimiter() {
  return pitLimiter;
}
bool SimHubReader::isDrsAvailable() {
  return drsAvailable;
}
bool SimHubReader::isDrsActive() {
  return drsActive;
}

float SimHubReader::getLapDelta() {
  return lapDelta;
}
float SimHubReader::getGapAhead() {
  return gapAhead;
}
float SimHubReader::getGapBehind() {
  return gapBehind;
}
float SimHubReader::getBrakeBias() {
  return brakeBias;
}
float SimHubReader::getFuelPercentage() {
  return fuelPercentage;
}
float SimHubReader::getTyreTempFL() {
  return tyreTempFL;
}
float SimHubReader::getTyreTempFR() {
  return tyreTempFR;
}
float SimHubReader::getTyreTempRL() {
  return tyreTempRL;
}
float SimHubReader::getTyreTempRR() {
  return tyreTempRR;
}
float SimHubReader::getTyrePressureFL() {
  return tyrePressureFL;
}
float SimHubReader::getTyrePressureFR() {
  return tyrePressureFR;
}
float SimHubReader::getTyrePressureRL() {
  return tyrePressureRL;
}
float SimHubReader::getTyrePressureRR() {
  return tyrePressureRR;
}
