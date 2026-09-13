
#pragma once

#include "../bcstem/mcu.h"
#include "../bcstem/util.h"

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
    static const uint8_t SW  = 3; //

    static const uint8_t SDA = MCU::SDA; // 8
    static const uint8_t SLC = MCU::SLC; // 9

  };

public:

  using PinMap = PinMap4ESP32S3;
  static constexpr const uint8_t espNowAddr[] = {0x70, 0xAF, 0x09, 0x0D, 0x35, 0x14};  // ESP32C3 super mini

  static constexpr const uint8_t espNowTargetAddr[] = {0xB0, 0xCB, 0xD8, 0xC6, 0x52, 0x04};

  static constexpr bool needI2C = false;
  static constexpr bool isEspNowReceiver = false;
  static constexpr bool isEspNowSender = true;

  void setCenter() {
    Stat xData;
    Stat yData;
    // mid point
    for (int i=0;i<5000;i++) {
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

    setCenter();
  }

  void sendCenter() {
    sendXY(0, 0);
    _lastAtCenter = true;
  }

  void sendXY(int16_t x_, int16_t y_) {
    XY16 xy{x_, y_};

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(espNowTargetAddr, (uint8_t *) &xy, sizeof(xy));
    
    if (result == ESP_OK) {
      Serial.print(x_);
      Serial.print(",");
      Serial.print(y_);
      Serial.println(" Sent with success");
      //Serial.println(_lastAtCenter);
    } else {
      Serial.print("Error sending the data. esp_now_send return ");
      Serial.println(result);
    }
  }

  void loop() {
    int16_t x = analogRead(PinMap::VRX) - _xCenter;
    int16_t y = analogRead(PinMap::VRY) - _yCenter;

    Serial.print(x);
    Serial.print(" , ");
    Serial.println(y);

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

    sendXY(x, y);
    _lastAtCenter = false;
    _lastXY.x = x;
    _lastXY.y = y;

    delay(100);
  }

  void onEspNowReceived(const uint8_t * macAddr_, const uint8_t *incomingData, int len) {}

  // callback when data is sent
  void onEspNowSent(const uint8_t * macAddr_, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:");
    Serial.print(status);
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? " Delivery Success" : " Delivery Fail");

  }

private:
  XY16 _lastXY;

  bool _lastAtCenter = true;
  int16_t _xCenter;
  int16_t _yCenter;
  int16_t _xTolerance;
  int16_t _yTolerance;

};

} // bcstem