//
// Created by nmwil on 15/09/2020.
//

#ifndef SIMHUB_READER_H
#define SIMHUB_READER_H

#include "BluetoothSerial.h"

#define LAP_TIME_STRLEN 10
#define FLOAT_BUF_SIZE 6
#define MESSAGE_BUF_SIZE 32

#define NO_INT -1
#define NO_FLOAT 0

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

    char getGear();
    char* getLapTime();
    char* getLastLapTime();
    char* getBestLapTime();
    
    int getSpeed();
    int getRpm();
    int getTcLevel();
    int getAbsLevel();
    int getMapLevel();
    int getPosition();
    int getLapNumber();
    int getBrakeBias();

    bool isYellowFlag();
    bool isBlueFlag();
    bool isPitLimiter();
    bool isDrsAvailable();
    bool isDrsActive();

    float getLapDelta();
    float getGapAhead();
    float getGapBehind();
    float getFuelPercentage();
    float getTyreTempFL();
    float getTyreTempFR();
    float getTyreTempRL();
    float getTyreTempRR();
    float getTyrePressureFL();
    float getTyrePressureFR();
    float getTyrePressureRL();
    float getTyrePressureRR();

  private:
    void processMessage(unsigned long nowMs);
    void cleanBufferDigits(int startOffset);
    void cleanBufferLap(int startOffset);
    void cleanBufferGear(int startOffset);

    void setBrakeBiasLevel(unsigned long ms);
    void setAbsLevel(unsigned long ms);
    void setMapLevel(unsigned long ms);
    void setTcLevel(unsigned long ms);
    void setFlags();

    void initProperties();

    BluetoothSerial EEBlue;

    char gear = 'n';
    int rpm = 0;
    int lapNumber = 0;
    int position = 0;
    int spd =0;
    float lapDelta = -1.23;
    char lapTime[LAP_TIME_STRLEN] = {'0', '0', '.', '0', '0', '.', '0', '0', 0x0};
    char lastLapTime[LAP_TIME_STRLEN] = {'0', '0', '.', '0', '0', '.', '0', '0', 0x0};
    char bestLapTime[LAP_TIME_STRLEN] = {'0', '0', '.', '0', '0', '.', '0', '0', 0x0};
    float gapAhead = 2.0;
    float gapBehind = 1.9;
    bool yellowFlag = false;
    bool blueFlag = false;
    bool pitLimiter = false;
    int tcLevel = -1;
    int absLevel = -1;
    int mapLevel = -1;
    bool drsAvailable = false;
    bool drsActive = false;
    float brakeBias = 0.0;
    float fuelPercentage;
    float tyreTempFL;
    float tyreTempFR;
    float tyreTempRL;
    float tyreTempRR;
    float tyrePressureFL;
    float tyrePressureFR;
    float tyrePressureRL;
    float tyrePressureRR;
    
    unsigned long lastMessageTime;

    char buf[MESSAGE_BUF_SIZE];
    int bufIndex = 0;
    char floatBuf[FLOAT_BUF_SIZE];
    bool inBrightnessMode = false;
    State state = NoConnection;
    unsigned long stateChangeTime = 0;
};

#endif //SIMHUB_READER_H
