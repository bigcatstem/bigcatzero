#pragma once

#include "mcu.h"

// Sensors
#include "sonar.h"
#include "motorset.h"
#include <ESP32Servo.h>

// App
#include "randomwalker.h"

namespace bcstem {

struct CFG {
  static const int sonarSensitivity = 1;
  static const int cmLimit = 20;
  //static const int cmFree = 30;
};

void testServo(Servo& servo_) {
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

  static const bool needI2C = true;
  static const bool needESPNOW = true;

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

    // ESPNOW
    //setupEspNow();

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
    _walker.loop();
  }

private:
  Sonar _sonar;
  MotorSet _motors;
  Servo _servo;

  RandomWalker<Sonar,MotorSet> _walker;

};


class ZeroRemote {

  struct PinMap4ESP32C3Zero {
    using MCU = ESP32C3Zero;
    static const int SDA = MCU::SDA; // 8
    static const int SLC = MCU::SLC; // 9
  };

public:

  using PinMap = PinMap4ESP32C3Zero;
  static constexpr bool needI2C = true;
  static const bool needESPNOW = true;

  void setup() {
    Serial.println("ZeroRemote::Remote");
    // ESPNOW
  }

  void loop() {

  }

};




} // bcstem