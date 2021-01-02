#include "simhubreader.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

SimHubReader::SimHubReader() {
}

SimHubReader::~SimHubReader() {
}

void SimHubReader::tick(unsigned long  nowMs) {
  int readCount = EEBlue.readBytesUntil('#', buf, MESSAGE_BUF_SIZE);
  buf[min(readCount, MESSAGE_BUF_SIZE - 1)] = 0x0;
  //            Serial.println(buf);
  processMessage(nowMs);
  memset(buf, 0x0, MESSAGE_BUF_SIZE);
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
    case 'I':
      setBrakeBiasLevel(nowMs);
      break;
    case 't':
      cleanBufferDigits(2);
      setTcAndAbs();
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

void SimHubReader::setTcAndAbs() {
  tcActive = buf[1] != '0';
  absActive = buf[2] != '0';
}

void SimHubReader::setTcLevel(unsigned long ms) {
  int newTcLevel = atoi(&buf[1]);

  if (newTcLevel != tcLevel) {
    tcLevel = newTcLevel;
  }
}

void SimHubReader::setAbsLevel(unsigned long ms) {
  int newAbsLevel = abs(atoi(&buf[1]));

  if (newAbsLevel != absLevel) {
    absLevel = newAbsLevel;
  }
}

void SimHubReader::setBrakeBiasLevel(unsigned long ms) {
  float newBrakeBias = atof(&buf[1]);

  if (newBrakeBias != brakeBias) {
    brakeBias = newBrakeBias;
  }
}
