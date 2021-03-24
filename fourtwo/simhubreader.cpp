#include "simhubreader.h"

SimHubReader::SimHubReader() {
}

SimHubReader::~SimHubReader() {
}

void SimHubReader::tick(unsigned long  nowMs) {
  if (Serial.available() > 0) {
    int readCount = Serial.readBytesUntil('#', buf, MESSAGE_BUF_SIZE);
    buf[min(readCount, MESSAGE_BUF_SIZE - 1)] = 0x0;
    //            Serial.println(buf);
    processMessage(nowMs);
    memset(buf, 0x0, MESSAGE_BUF_SIZE);
  }
}

void SimHubReader::begin() {
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
    case 'm':
      setMapLevel(nowMs);
      break;
    case 'f':
      setFuel();
      break;
    case 'l':
      setLap();
      break;
    case 'p':
      setPosition(nowMs);
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
      cleanBufferDigits(2);
      sscanf(&buf[1], "%d", &spd);
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
      strncpy(lapTime, &buf[1], min(LAP_TIME_STRLEN - 1, (int)strlen(&buf[1])));
      break;
    case 'J':
      memset(lastLapTime, 0, LAP_TIME_STRLEN);
      strncpy(lastLapTime, &buf[1], min(LAP_TIME_STRLEN - 1, (int)strlen(&buf[1])));
      break;
    case 'K':
      memset(bestLapTime, 0, LAP_TIME_STRLEN);
      strncpy(bestLapTime, &buf[1], min(LAP_TIME_STRLEN - 1, (int)strlen(&buf[1])));
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

void SimHubReader::setFuel() {
  if (strlen(&buf[1]) > 1) {
    fuel = atoi(&buf[1]);
  }
}

void SimHubReader::setLap() {
  lapNumber = atoi(&buf[1]);
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

void SimHubReader::setPosition(unsigned long ms) {
  position = atoi(&buf[1]);
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
  spd = 227;
  fuel = 0;
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
int SimHubReader::getSpeed() {
  return spd;
}

int SimHubReader::getFuel() {
  return fuel;
}
int SimHubReader::getRpm() {
  return rpm;
}
char SimHubReader::getTcLevel() {
  return tcLevel > 0 ? tcLevel : '-';
}
char SimHubReader::getAbsLevel() {
  return absLevel > 0 ? absLevel : '-';
}
char SimHubReader::getMapLevel() {
  return mapLevel > 0 ? mapLevel : '-';
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
int SimHubReader::getBrakeBias() {
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
