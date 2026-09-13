
#pragma once

#include "../bcstem/mcu.h"
#include "../bcstem/util.h"

namespace bcstem {

class ZeroRemote {

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
  static constexpr const uint8_t espNowAddr[] = {0x70, 0xAF, 0x09, 0x0D, 0x35, 0x14};  // ESP32C3 super mini

  static constexpr const uint8_t espNowTargetAddr[] = {0xB0, 0xCB, 0xD8, 0xC6, 0x52, 0x04};

  //static constexpr const uint8_t  espNowTargetAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  static constexpr bool needI2C = false;
  static constexpr bool isEspNowReceiver = false;
  static constexpr bool isEspNowSender = true;

  void setup() {
    Serial.println("ZeroRemote::setup()");

    // mid point
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
    esp_err_t result = esp_now_send(espNowTargetAddr, (uint8_t *) &_lastXY, sizeof(_lastXY));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.print("Error sending the data. esp_now_send return ");
      Serial.println(result);
    }
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

  int _x0;
  int _y0;

};

} // bcstem