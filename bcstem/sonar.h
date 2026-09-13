#pragma once
//#include <Wire.h>
//#include "bcstem/util.h"
#include <VL6180X.h>
#include <VL53L0X.h>
#include "util.h"

namespace bcstem {

class SonarBase {
public:

  static const int TIMEOUT=-1;

  bool setup(int sensitivity_, int cmLimit_) {
    Serial.println("SonarBase::setup");
    _sensitivity = sensitivity_;
    _cmLimit = cmLimit_;

    return true;
  }

  virtual int ping() = 0;

  bool isBlocked() {
    for (int i=0;i<_sensitivity;i++)
    {
      if (!isBlockedOnce()) { return false; }
    }
    return true;
  }

  bool isFree() {
    int cm = ping();
    if (cm==TIMEOUT) { return true; }
    return cm > 30;
  }

  bool isBlockedOnce() {
    int cm = ping();
    if (cm==TIMEOUT) { return false; }
    //auto s = std::format("{} {} {}",dCM, _cmLimit, dCM < _cmLimit);
    //Serial.println(s.c_str());
    return cm < _cmLimit;
  }

protected:
  int _sensitivity;
  int _cmLimit;
};

class Sonar53L0 : public SonarBase {

public: 
  void setup(int sensitivity_, int cmLimit_) {
    SonarBase::setup(sensitivity_, cmLimit_);
    //Wire.begin(SDA, SLC);

    if(!bcstem::testI2CAddress(0x29)) {
      GlobalObject::get().setupPassed = false;
    }

    if (!_53L0.init()) {
      GlobalObject::get().setupPassed = false;
    }
  }  

  // return TIMEOUT if timeout
  // return distance in cm
  int ping() {
    int cm =  _53L0.readRangeSingleMillimeters() / 10;

    if (_53L0.timeoutOccurred()) { 
      return TIMEOUT; 
    }

    return cm;
  }

private:
  VL53L0X _53L0;

};

class Sonar6180 : public SonarBase {

public: 
  void setup(int sensitivity_, int cmLimit_) {
    SonarBase::setup(sensitivity_, cmLimit_);

    if(!bcstem::testI2CAddress(0x29)) {
      GlobalObject::get().setupPassed = false;
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

template<int ECHO,int TRIG>
class SonarT : public SonarBase {

public:

  void setup(int sensitivity_, int cmLimit_) {
    SonarBase::setup(sensitivity_, cmLimit_);
    Serial.println("SonarT::initialize");

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
