#pragma once

#include <esp_wifi.h>
#include <Wire.h>
#include <WiFi.h>

namespace bcstem {

// Data ////////////////////////////////////////////////////////

struct XY16 {
  uint16_t x;
  uint16_t y;
};


struct GlobalObject {
  bool setupPassed = true;
  String errorLog;

  static GlobalObject& get() {
    static GlobalObject g;
    return g;
  }
};


// SERIAL ///////////////////////////////////////////////////////

void serialEcho() 
{
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0) {
      Serial.print("Echo: ");
      Serial.println(input);
    }
  }
}

// ESPNOW ///////////////////////////////////////////////////////

void printMacAddress() {
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("ESPNOW MAC address:%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    GlobalObject::get().errorLog += "Failed to read MAC address\n";
    Serial.println("Failed to read MAC address");
  }

  uint8_t primary;
  wifi_second_chan_t second;
  esp_wifi_get_channel(&primary, &second);

  Serial.print("Channel: ");
  Serial.println(primary);
}

bool setupEspNow() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  WiFi.STA.begin();
  printMacAddress();
  return true;
}

// I2C ///////////////////////////////////////////////////////

bool testI2CAddress(byte addr_) {
    Wire.beginTransmission(addr_);
    byte error = Wire.endTransmission();
    if (error==0) {
      return true;
    }
    Serial.print("testI2CAddress address:");
    Serial.printf("0x%02x error:", addr_);
    Serial.println(error);
    GlobalObject::get().errorLog += "testI2CAddress\n";
}

void scanI2C() {
  for (byte address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);

    byte error = Wire.endTransmission();
    Serial.println(address);

    if (error == 0)
    {
      Serial.print("I2C device found at 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }  
}

// Timer ///////////////////////////////////////////////////////

class Timer
{
public:
  Timer(unsigned long duration_) : 
    _duration(duration_),
    _startMs(millis())
  {
  }

  void start() {
    _startMs = millis();
  }

  bool fired(unsigned long currentMs)
  {
    if (currentMs - _startMs > _duration) 
    {
      _startMs = millis();
      return true;
    }

    return false;
  }

private:
  unsigned long _startMs;
  unsigned long _duration;
};

} // namespace bcstem
