#pragma once

#include <esp_wifi.h>
#include <esp_now.h>

#include <Wire.h>
#include <WiFi.h>

namespace bcstem {

// Data ////////////////////////////////////////////////////////

struct XY16 {
  int16_t x;
  int16_t y;
};

struct GlobalObject {
  String errorLog;

  void _failSetup() { _setupPassed==false; }

  bool setupPassed() { return _setupPassed; }

  static GlobalObject& get() {
    static GlobalObject g;
    return g;
  }

  static void failSetup() {
    get()._failSetup();
  }

public:
  bool _setupPassed = true;

};

// number /////////////////////////////////////////////////////

inline bool isBetween(int x_, int a_, int b_) {
  return x_>=a_ && x_<=b_;
}

inline int delta(int x_, int y_) {
  return abs(x_ - y_);
}

// SERIAL ///////////////////////////////////////////////////////

inline void serialEcho() 
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

constexpr uint8_t ESPNOW_ADDR_SIZE = 6;

inline void printEspNowAddress(const uint8_t macAddr_[ESPNOW_ADDR_SIZE]) {

  Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                macAddr_[0], macAddr_[1], macAddr_[2],
                macAddr_[3], macAddr_[4], macAddr_[5]);
}

inline void printLocalEspNowAddress() {
  uint8_t baseMac[ESPNOW_ADDR_SIZE];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.print("Base EspNow address: ");
    printEspNowAddress(baseMac);
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

inline bool setupEspNow() {
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  WiFi.STA.begin();
  printLocalEspNowAddress();

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return false;
  }

  return true;
}

inline bool setupEspNowSender(const uint8_t* targetAddr_, void(*onEspNowSent_)(const uint8_t *, esp_now_send_status_t)) {

  Serial.println("setupEspNowSender");
  Serial.print("Target address: ");
  printEspNowAddress(targetAddr_);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(esp_now_send_cb_t(onEspNowSent_));

  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, targetAddr_, ESPNOW_ADDR_SIZE);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return false;
  }

  return true;
}

inline bool setupEspNowReceiver(void(*onEspNowReceived_)(const uint8_t *, const uint8_t *, int )) {
    esp_now_register_recv_cb(esp_now_recv_cb_t(onEspNowReceived_));

    return true;
}

// I2C ///////////////////////////////////////////////////////

inline bool testI2CAddress(byte i2cAddr_) {
    Wire.beginTransmission(i2cAddr_);
    byte error = Wire.endTransmission();
    if (error==0) {
      return true;
    }
    Serial.print("testI2CAddress address:");
    Serial.printf("0x%02x error:", i2cAddr_);
    Serial.println(error);
    GlobalObject::get().errorLog += "testI2CAddress\n";

    return false;
}

inline void scanI2C() {
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
