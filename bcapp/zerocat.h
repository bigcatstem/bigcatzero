#pragma once

#include "../bcstem/mcu.h"

// ESP
#include <esp_wifi.h>
#include <esp_now.h>

// Sensors
#include "../bcstem/sonar.h"
#include "../bcstem/motorset.h"
#include <ESP32Servo.h>

// App
#include "randomwalker.h"

namespace bcstem {

struct CFG {
  static const int sonarSensitivity = 1;
  static const int cmLimit = 20;
  //static const int cmFree = 30;
};

inline void testServo(Servo& servo_) {
  for (int i=0;i<179;i+=10) {
    servo_.write(i);
    delay(10);
  }
  servo_.write(90);
}

class ZeroCat {

  struct PinMap4ESP32S {
    using MCU = ESP32S;

  /*
    GND BLACK
    VCC
    D21 GREY      SDA
    D22 PURPLE    SLC
    D15 BLUE      ECHO
    D2  GREEN     L-IN1
    D4  YELLOW    L-IN2 
    D16 ORANGE    R-IN4
    D17 RED       R-IN3
    D5  BROWN     // Pin 5 is already attached to LEDC
    D18 BLACK     // Pin 18 is already attached to LEDC (channel 0, resolution 10)
    D19 WHITE     TRIG
    D21 GREY      SDA 
    RX0
    TX0
    D22 GREEN (DUPLICTED) SCL
    D23 YELLOW    SERVO
  */

    static const int FL2 = 2;        // L-IN2 YELLOW  4->2 Green
    static const int FL1 = 4;        // L-IN1 GREEN   2->15->4 BLUE  
    static const int MotorL1 = FL1;
    static const int MotorL2 = FL2;
    //static const int BL1 = 22;       // L-IN3 PURPLE
    //static const int BL2 = 21;       // L-IN4 GREY
    //static const int FR1 = 18;       // R-IN2 BLACK
    //static const int FR2 = 19;       // R-IN1 WHITE
    static const int BR1 = 16;       // R-IN4 ORANGE
    static const int BR2 = 17;       // R-IN3 RED
    static const int MotorR1 = BR1;
    static const int MotorR2 = BR2;
    static const int ECHO = 15; // BLUE       //       YELLOW     15->4->15
    static const int TRIG = 19; // WHITE //23;      //       YELLOW
    static const int SERVO = 23; // YELLOW //21;     // GREY
    static const int SDA   = MCU::SDA; // 21;  // GREY 
    static const int SLC   = MCU::SLC; // 22;  // PURPLE


    // MAC Address: b0:cb:d8:c6:52:04
  };


public:

  using PinMap = PinMap4ESP32S;
  using MCU = PinMap::MCU;

  static constexpr const uint8_t espNowAddr[] = {0xB0, 0xCB, 0xD8, 0xC6, 0x52, 0x04};  // ESP32S
  static constexpr const uint8_t espNowTargetAddr[] = {0, 0, 0, 0, 0, 0};

  static constexpr bool needI2C = true;
  static constexpr bool isEspNowReceiver = true;
  static constexpr bool isEspNowSender = false;

  //using Sonar = SonarT<PinMap::ECHO, PinMap::TRIG>;
  //using Sonar = Sonar6180;
  using Sonar = Sonar53L0;

  //using MotorSet = MotorSetT<PinMap::FL1, PinMap::FL2, PinMap::FR1, PinMap::FR2,
  //                          PinMap::BL1, PinMap::BL2, PinMap::BR1, PinMap::BR2>;
  using MotorSet = MotorSetT<PinMap::MotorL1, PinMap::MotorL2,
        PinMap::MotorR1, PinMap::MotorR2>;

  ZeroCat() : _walker(_sonar, _motors, _servo)
  {
  }

  void setup() {

    Serial.println("ZeroCat::setup()");

    // Sensors
    _servo.attach(PinMap::SERVO);
    _servo.write(90);
    _motors.initialize();
    _sonar.setup(CFG::sonarSensitivity, CFG::cmLimit);
    _walker.initialize();

    testServo(_servo);
    //scanI2C();
    //g_timerServo.start();
  }

  void loop() {
    //_walker.loop();
  }

  void onEspNowSent(const uint8_t * macAddr_, esp_now_send_status_t status) {}

  // callback function that will be executed when data is received
  void onEspNowReceived(const uint8_t * macAddr_, const uint8_t *incomingData, int len) {

    XY16 xy;
    memcpy(&xy, incomingData, sizeof(xy));
    Serial.print(len);
    Serial.print(" , ");
    Serial.print(xy.x);
    Serial.print(" , ");
    Serial.println(xy.y);

    if (isBetween((xy.y)/8,-100,100)) {
      if (xy.x/8 > 100) {
        _motors.backward();
      } else if (xy.x/8 < -100) {
        _motors.forward();
      } else {
        _motors.stop();
      }
      //motors.moveAnalog((xy.x-2048)/8, (xy.x-2048)/8);
    } else if (xy.y>0) {
      _motors.rotateLeft();
      //motors.moveAnalog(0, (xy.x-2048)/8);
    } else {
      _motors.rotateRight();
      //motors.moveAnalog((xy.x-2048)/8, 0);
    }

  }

  //MotorSet& motors() { return _motors; }

private:
  Sonar _sonar;
  MotorSet _motors;
  Servo _servo;

  RandomWalker<Sonar,MotorSet> _walker;

};




} // bcstem