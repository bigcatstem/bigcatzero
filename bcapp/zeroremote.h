
#pragma once

#include "../bcstem/mcu.h"
#include "../bcstem/util.h"

#include "zerocatcommon.h"

#include <ezButton.h>

// Range: -2048 to 2048
// Setup: get center and tolerance
// Noise filtering
// - Special case for center: if pos - center < tolerance => active keep center
// - Last - Center < tolerance => filter out

// TODO: Button for center setting

namespace bcstem {

class ZeroRemote {

  struct PinMap4ESP32S3 {
    using MCU = ESP32S3;
    static const uint8_t VRX = 4; //
    static const uint8_t VRY = 5; //
    static const uint8_t SW  = 6; //
    static const uint8_t SDA = MCU::SDA; // 8
    static const uint8_t SLC = MCU::SLC; // 9


  };

  struct PinMap4ESP32C3 {
    using MCU = ESP32C3;
    static const uint8_t VRX = 4; //
    static const uint8_t VRY = 3; //
    static const uint8_t SW  = 2; //
    static const uint8_t SDA = MCU::SDA; // -1
    static const uint8_t SLC = MCU::SLC; // -1

    static const uint8_t LED_B = 21; //
    static const uint8_t LED_G = 20; //
    static const uint8_t LED_R = 10; //
  };


public:

  //using PinMap = PinMap4ESP32S3;
  using PinMap = PinMap4ESP32C3;

  ZeroRemote() : _button(PinMap::SW) {}

  static constexpr const uint8_t espNowAddr[] = {0x70, 0xAF, 0x09, 0x0D, 0x35, 0x14};  // ESP32C3 super mini

  static constexpr const uint8_t espNowTargetAddr[] = {0xB0, 0xCB, 0xD8, 0xC6, 0x52, 0x04};

  static constexpr bool needI2C = false;
  static constexpr bool isEspNowReceiver = false;
  static constexpr bool isEspNowSender = true;

  void setCenter() {
    Stat xData;
    Stat yData;
    // mid point

    constexpr int SAMPLE_SIZE = 5000;
    for (int i=0;i<SAMPLE_SIZE;i++) {
      xData.sample(analogRead(PinMap::VRX));
      yData.sample(analogRead(PinMap::VRY));
    }
    _xCenter = xData.average();
    _yCenter = yData.average();
    _xTolerance = xData.derivation();
    _yTolerance = yData.derivation();

    Serial.print("Center:");
    Serial.print(_xCenter);
    Serial.print(",");
    Serial.print(_yCenter);
    Serial.print(" Tolerance");
    Serial.print(_xTolerance);
    Serial.print(",");
    Serial.println(_yTolerance);

    _lastXY.x = 0;
    _lastXY.y = 0;

    sendCenter();
  }

  void setup() {
    Serial.println("ZeroRemote::setup()");

    uint8_t pins[] = {PinMap::LED_R, PinMap::LED_G, PinMap::LED_B};
    for (auto pin: pins){
      int32_t f = 5000;
      int8_t res = 8;
      ledcAttach(pin, f, res);
      //pinMode(pin, OUTPUT);
    }

    analogWrite(PinMap::LED_R, 127);
    analogWrite(PinMap::LED_G, 127);
    analogWrite(PinMap::LED_B, 127);


    setCenter();
  }

  void sendCenter() {
    sendEvent(0, 0);
    _lastAtCenter = true;
  }

  void sendEvent(int16_t x_, int16_t y_) {
    JoystickEvent e {x_, y_, _catMode};

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(espNowTargetAddr, (uint8_t *) &e, sizeof(e));
    
    if (result == ESP_OK) {
      e.print();
      Serial.println(" Sent with success");
      //Serial.println(_lastAtCenter);
    } else {
      Serial.print("Error sending the data. esp_now_send return ");
      Serial.println(result);
    }

    _lastXY.x = x_;
    _lastXY.y = y_;
  }

  //
  XY16 getXYFromJoystick() {
    Stat xs;
    Stat ys;

    constexpr int SAMPLE_SIZE = 10;
    for  (int i=0;i<SAMPLE_SIZE;i++) {
      int16_t x = analogRead(PinMap::VRX) - _xCenter;
      int16_t y = analogRead(PinMap::VRY) - _yCenter;
      xs.sample(x);
      ys.sample(y);
    }

    return XY16 {xs.averageExclusive(), ys.averageExclusive()} ;
  }

  bool isButtonNewlyPushed(bool& _wasButtonPushed) {
    bool isPushed = _button.isPressed();
    bool yn = false;
    if (!_wasButtonPushed && isPushed) {
      yn = true;
    }
    _wasButtonPushed = isPushed;
    return yn;
  }

  void loop() {

    _button.loop();

    XY16 xy = getXYFromJoystick();

    if (isButtonNewlyPushed(_wasButtonPushed)) {

      _catMode = _catMode==REMOTE_WALKER? RANDOM_WALKER : REMOTE_WALKER;

      sendEvent(_lastXY.x, _lastXY.y);
    }

    int16_t x = xy.x;
    int16_t y = xy.y;
    
    float pi = 3.14;
    float a = atan(abs(y)/abs(x));
    if (x>=0 && y>=0) {
      a = a;
    }  else if (x<0 && y>=0) {
      a = a + pi/2;
    }  else if (x<0 && y<0) {
      a = a + pi;
    }  else  {
      a = a + pi*3/2;
    }
    int l = sqrt( x*x/64 + y*y/64 );
    l = l * 0.4 / 2;
    int r = (l) * (sin(a)       +1)   ;
    int g = (l) * (sin(a+pi*2/3)+1) ;
    int b = (l) * (sin(a+pi*4/3)+1) ;
    analogWrite(PinMap::LED_R, r);
    analogWrite(PinMap::LED_G, g);
    analogWrite(PinMap::LED_B, b);

    Serial.print(r);
    Serial.print(" , ");
    Serial.println(g);
    Serial.print(" , ");
    Serial.println(b);

    // Special case for center
    if (abs(x) < _xTolerance && abs(y) < _yTolerance) {
      if (!_lastAtCenter) {
        sendCenter();
      }
      delay(100);
      return;
    }

    if (delta(_lastXY.x, x) < _xTolerance && delta(_lastXY.y, y) < _yTolerance ) {
      Serial.println("Filtered");
      delay(100);
      return;
    }



    sendEvent(x, y);
    _lastAtCenter = false;
    _lastXY.x = x;
    _lastXY.y = y;

    delay(100);
  }


  void onEspNowReceivedAction(const uint8_t *espNowReceivedBuffer_, int len_) {
  }

  void onEspNowReceived(const uint8_t * macAddr_, const uint8_t *incomingData, int len) {}

  // callback when data is sent
  void onEspNowSent(const uint8_t * macAddr_, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:");
    Serial.print(status);
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? " Delivery Success" : " Delivery Fail");

  }

private:

  ezButton _button;

  XY16 _lastXY;

  bool _lastAtCenter = true;
  int16_t _xCenter;
  int16_t _yCenter;
  int16_t _xTolerance;
  int16_t _yTolerance;

  bool _wasButtonPushed = false;

  CatMode _catMode;

};

} // bcstem