#pragma once

#include "mcu.h"

// ESP
#include <esp_wifi.h>
#include <esp_now.h>

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

inline void testServo(Servo& servo_) {
  for (int i=0;i<179;i+=10) {
    servo_.write(i);
    delay(10);
  }
  servo_.write(90);
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) ;


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

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Sensors
    _servo.attach(PinMap::SERVO);
    _servo.write(90);
    _motors.initialize();
    _sonar.setup(CFG::sonarSensitivity, CFG::cmLimit);
    _walker.initialize();

    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

    testServo(_servo);
    //scanI2C();
    //g_timerServo.start();
  }

  void loop() {
    //_walker.loop();
  }

  MotorSet& motors() { return _motors; }

private:
  Sonar _sonar;
  MotorSet _motors;
  Servo _servo;

  RandomWalker<Sonar,MotorSet> _walker;

};




class ZeroRemote {


// callback when data is sent
static void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:");
  Serial.print(status);
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? " Delivery Success" : " Delivery Fail");

}


  struct PinMap4ESP32S3 {
    using MCU = ESP32S3;


    static const int VRX = 4; //
    static const int VRY = 5; //
    static const int SW  = 3; //

    static const int SDA = MCU::SDA; // 8
    static const int SLC = MCU::SLC; // 9

    // ESPNOW address: 70:af:09:0d:35:14
  };

public:

  using PinMap = PinMap4ESP32S3;
  static constexpr bool needI2C = false;
  static const bool needESPNOW = true;

  void setup() {
    Serial.println("ZeroRemote::Remote");
    // ESPNOW

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
    
    // Register peer
    memcpy(_peerInfo.peer_addr, carAddr, 6);
    _peerInfo.channel = 1;
    _peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&_peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }    

    _x0 = analogRead(PinMap::VRX);
    _y0 = analogRead(PinMap::VRY);


  }

  void loop() {
    int x = analogRead(PinMap::VRX) - _x0;
    int y = analogRead(PinMap::VRY) - _y0;

    if (delta(_lastXY.x, x) < 100 && delta(_lastXY.y, y) < 100) {
      return;
    }

    _lastXY.x = x;
    _lastXY.y = y;

    Serial.print(_lastXY.x);
    Serial.print(" , ");
    Serial.println(_lastXY.y);

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(carAddr, (uint8_t *) &_lastXY, sizeof(_lastXY));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.print("Error sending the data ");
      Serial.println(result);
    }
    delay(100);

  }

private:
  esp_now_peer_info_t _peerInfo;
  XY16 _lastXY;

  int _x0;
  int _y0;

};




} // bcstem