//
// Created by nmwil on 15/09/2020.
//

#ifndef SIMHUB_READER_H
#define SIMHUB_READER_H

#include <arduino.h>

#define LAP_TIME_STRLEN 10
#define FLOAT_BUF_SIZE 6
#define MESSAGE_BUF_SIZE 32
#define TYRE_PRESSURE_BUF_SIZE 4

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
    
    char* getTyreTempFL();
    char* getTyreTempFR();
    char* getTyreTempRL();
    char* getTyreTempRR();

    int getSpeed();
    int getFuel();
    int getRpm();
    char getTcLevel();
    char getAbsLevel();
    char getMapLevel();
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
    void setPosition(unsigned long ms);
    void setLap();
    void setFlags();
    void setFuel();
    void setTyreTemps();

    void initProperties();

    char gear = 'n';
    int rpm = 0;
    int lapNumber = 0;
    int position = 0;
    int spd =0;
    int fuel;
    float lapDelta = -1.23;
    char lapTime[LAP_TIME_STRLEN] = {'0', '0', '.', '0', '0', '.', '0', '0', 0x0};
    char lastLapTime[LAP_TIME_STRLEN] = {'0', '0', '.', '0', '0', '.', '0', '0', 0x0};
    char bestLapTime[LAP_TIME_STRLEN] = {'0', '0', '.', '0', '0', '.', '0', '0', 0x0};
    char tyreTempFL[TYRE_PRESSURE_BUF_SIZE + 1];
    char tyreTempFR[TYRE_PRESSURE_BUF_SIZE + 1];
    char tyreTempRL[TYRE_PRESSURE_BUF_SIZE + 1];
    char tyreTempRR[TYRE_PRESSURE_BUF_SIZE + 1];
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
