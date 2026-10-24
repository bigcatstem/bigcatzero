#pragma once
//#include <Wire.h>
//#include "bcstem/util.h"
//#include <VL6180X.h>
#include <VL53L0X.h>
#include "util.h"

namespace bcstem {

class SonarBase {
public:

  static const int TIMEOUT=-1;

  bool setup(int sensitivity_, int mmLimit_) {
    Serial.println("SonarBase::setup");
    _sensitivity = sensitivity_;
    _mmLimit = mmLimit_;

    return true;
  }

  virtual int ping() = 0;

  enum class Result {
    TOO_CLOSE,
    IN_RANGE,
    TOO_FAR,
    OUT_OF_REACH
  };

  Result check(int mmClose_, int mmFar_, int mmReachLimit_) {
    int mm = ping();
    if (mm==TIMEOUT) { return Result::OUT_OF_REACH; }
    if (mm > mmReachLimit_) { return Result::OUT_OF_REACH; }
    if (mm > mmFar_) { return Result::TOO_FAR; }
    if (mm < mmClose_) { return Result::TOO_CLOSE; }
    return Result::IN_RANGE;
  }

  bool isBlocked() {
    for (int i=0;i<_sensitivity;i++)
    {
      if (!isBlockedOnce()) { return false; }
    }
    return true;
  }

  bool isFree() {
    int mm = ping();
    if (mm==TIMEOUT) { return true; }
    return mm > 300;
  }

  bool isBlockedOnce() {
    int mm = ping();
    if (mm==TIMEOUT) { return false; }
    //auto s = std::format("{} {} {}",dCM, _mmLimit, dCM < _mmLimit);
    //Serial.println(s.c_str());
    return mm < _mmLimit;
  }

protected:
  int _sensitivity;
  int _mmLimit;
};

template <typename CFG>
class Sonar53L0 : public SonarBase {

public: 
  void setup(int sensitivity_, int mmLimit_) {
    SonarBase::setup(sensitivity_, mmLimit_);
    //Wire.begin(SDA, SLC);

    if constexpr (CFG::Sonar::enable) {

      if(!bcstem::testI2CAddress(0x29)) {
        Serial.println("Sonar53L0::setup testI2CAddress failed");
        GlobalObject::failSetup();
      }

      if (!_53L0.init()) {
        Serial.println("Sonar53L0::setup init failed");
        GlobalObject::failSetup();
      }
    }
    Serial.println("Sonar53L0::setup done");
  }  

  // return TIMEOUT if timeout
  // return distance in mm
  int ping() {
//Serial.println("Sonar53L0::ping");

    if constexpr (!CFG::Sonar::enable) {
      return TIMEOUT;
    }

    int mm =  _53L0.readRangeSingleMillimeters();

    if (_53L0.timeoutOccurred()) { 
//Serial.println("Sonar53L0::ping TIMNEOUT");
      return TIMEOUT; 
    }

//Serial.print("Sonar53L0::ping cm:");
//Serial.println(cm);
    return mm;
  }

private:
  VL53L0X _53L0;

};

/*
class Sonar6180 : public SonarBase {

public: 
  void setup(int sensitivity_, int mmLimit_) {
    SonarBase::setup(sensitivity_, mmLimit_);

    if(!bcstem::testI2CAddress(0x29)) {
      GlobalObject::failSetup();
    }

    //Wire.begin(SDA, SLC);
    _6180.init();
    _6180.configureDefault();
    _6180.setTimeout(100);  // 1ms
  }  

  // return TIMEOUT if timeout
  // return distance in cm
  int ping() {
    int cm =  _6180.readRangeSingleMillimeters() / 10;

    if (_6180.timeoutOccurred()) { 
      return TIMEOUT; 
    }

    return cm;
  }

private:
  VL6180X _6180;

};
*/

template<int ECHO,int TRIG>
class SonarT : public SonarBase {

public:

  void setup(int sensitivity_, int mmLimit_) {
    SonarBase::setup(sensitivity_, mmLimit_);
    Serial.println("SonarT::setup");

    pinMode(ECHO, INPUT);
    pinMode(TRIG, OUTPUT);

    // initalize TRIG pin
    digitalWrite(TRIG, LOW);
  }

  // return TIMEOUT if timeout
  // return distance in cm
  int ping() {

    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    unsigned long microSec = pulseIn(ECHO, HIGH, 10000L);

    if (microSec==0) { return TIMEOUT; }

    unsigned long d = microSec / 58; //* 0.017;
    //if (microSec!=0) {
      //auto s = std::format("{} {}", microSec, d);
      //Serial.println(s.c_str());

    //}
    return static_cast<int>(d);
  }

};

} // namespace bcstem
