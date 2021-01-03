//
// Created by nmwil on 15/09/2020.
//

#ifndef SIMHUB_READER_H
#define SIMHUB_READER_H

#include "BluetoothSerial.h"

#define LAP_TIME_STRLEN 9
#define FLOAT_BUF_SIZE 6
#define MESSAGE_BUF_SIZE 32

enum State {
  NoConnection,
  ConnectedToSimHub,
  Driving
};

class SimHubReader {
  public:
    SimHubReader();
    ~SimHubReader();

    void begin();
    void tick(unsigned long nowMs);

  private:
    void processMessage(unsigned long nowMs);
    void cleanBufferDigits(int startOffset);
    void cleanBufferLap(int startOffset);
    void cleanBufferGear(int startOffset);

    void setBrakeBiasLevel(unsigned long ms);
    void setAbsLevel(unsigned long ms);
    void setTcLevel(unsigned long ms);
    void setFlags();
    void setTcAndAbs();

    BluetoothSerial EEBlue;

    char gear = 'n';
    int rpm = 0;
    char spd[4] = {'0', '0', '0', 0x0};
    float lapDelta = -1.23;
    char lapTime[LAP_TIME_STRLEN] = {'0', '0', '.', '0', '0', '.', '0', '0', 0x0};
    float gapAhead = 2.0;
    float gapBehind = 1.9;
    bool yellowFlag = false;
    bool blueFlag = false;
    bool pitLimiter = false;
    bool tcActive = false;
    int tcLevel = -1;
    int absLevel = -1;
    bool absActive = false;
    bool drsAvailable = false;
    bool drsActive = false;
    float brakeBias = 0.0;

    unsigned long lastMessageTime;

    char buf[MESSAGE_BUF_SIZE];
    int bufIndex = 0;
    char floatBuf[FLOAT_BUF_SIZE];
    bool inBrightnessMode = false;
    State state = NoConnection;
    unsigned long stateChangeTime = 0;
};

#endif //SIMHUB_READER_H
