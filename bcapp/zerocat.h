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
#include "remotewalker.h"

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

    static const uint8_t FL2 = 2;        // L-IN2 YELLOW  4->2 Green
    static const uint8_t FL1 = 4;        // L-IN1 GREEN   2->15->4 BLUE  
    static const uint8_t MotorL1 = FL1;
    static const uint8_t MotorL2 = FL2;
    //static const int BL1 = 22;       // L-IN3 PURPLE
    //static const int BL2 = 21;       // L-IN4 GREY
    //static const int FR1 = 18;       // R-IN2 BLACK
    //static const int FR2 = 19;       // R-IN1 WHITE
    static const uint8_t BR1 = 16;       // R-IN4 ORANGE
    static const uint8_t BR2 = 17;       // R-IN3 RED
    static const uint8_t MotorR1 = BR1;
    static const uint8_t MotorR2 = BR2;
    static const uint8_t ECHO = 15; // BLUE       //       YELLOW     15->4->15
    static const uint8_t TRIG = 19; // WHITE //23;      //       YELLOW
    static const uint8_t SERVO = 23; // YELLOW //21;     // GREY
    static const uint8_t SDA   = MCU::SDA; // 21;  // GREY 
    static const uint8_t SLC   = MCU::SLC; // 22;  // PURPLE


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

  enum WalkerMode {
    RANDOM_WALKER = 0,
    REMOTE_WALKER
  };

  ZeroCat() : 
    _randomWalker(_sonar, _motors, _servo),
    _remoteWalker(_sonar, _motors, _servo)
  {
    _walkerMode = REMOTE_WALKER;
  }

  void setup() {

    Serial.println("ZeroCat::setup()");

    // Sensors
    _servo.attach(PinMap::SERVO);
    _servo.write(90);
    _motors.setup();
    _sonar.setup(CFG::sonarSensitivity, CFG::cmLimit);
    _randomWalker.setup();
    _remoteWalker.setup();

    testServo(_servo);
    //scanI2C();
    //g_timerServo.start();
  }

  void loop() {

    switch(_walkerMode) {
      case REMOTE_WALKER:
        _remoteWalker.loop();
      break;
      case RANDOM_WALKER:
        _randomWalker.loop();
      break;
    }

  }

  void onEspNowSent(const uint8_t * macAddr_, esp_now_send_status_t status) {}

  // callback function that will be executed when data is received
  void onEspNowReceived(const uint8_t * macAddr_, const uint8_t *incomingData, int len) {

    XY16 xy; // -2048 to 2048
    memcpy(&xy, incomingData, sizeof(xy));

    Serial.print(xy.x);
    Serial.print(" , ");
    Serial.println(xy.y);

    _remoteWalker.onControl(xy.x, xy.y);


  }

private:
  Sonar _sonar;
  MotorSet _motors;
  Servo _servo;

  WalkerMode _walkerMode = REMOTE_WALKER; 

  RandomWalker<Sonar,MotorSet> _randomWalker;
  RemoteWalker<Sonar,MotorSet> _remoteWalker;

};




} // bcstem