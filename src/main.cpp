#include <Arduino.h>

// App
#if defined(BC_CAT)
  #include "bcapp/zerocat.h"
  using CAT = bcstem::ZeroCat;
#elif defined(BC_REMOTE)
  #include "bcapp/zeroremote.h"
  using CAT = bcstem::ZeroRemote;
#endif

using PinMap = CAT::PinMap;
using MCU = CAT::PinMap::MCU;

namespace bcstem {

CAT g_cat;

//auto g = bcstem::GlobalObject::get();
long _cnt = 0;

volatile bool _espNowReceived = false;
uint16_t _espNowReceiveCount = 0;
uint16_t _espNowActionCount = 0;

constexpr uint8_t MAX_ESPNOW_SIZE = 100;
uint8_t _espNowReceiveBuffer[MAX_ESPNOW_SIZE];
int _espNowReceiveLen = 0;

// So that, the handle in CAT can be a normal member function
// IsEnable
void onEspNowReceived(const uint8_t * macAddr_, const uint8_t *incomingData_, int len_) {

  if constexpr (CAT::isEspNowReceiver) {

    if (len_ > MAX_ESPNOW_SIZE) {
      Serial.println("onEspNowReceived too big");
      len_ = MAX_ESPNOW_SIZE;
    }

    _espNowReceiveLen = len_;
    memcpy(_espNowReceiveBuffer, incomingData_, len_);
    _espNowReceiveCount++;
    g_cat.onEspNowReceived(macAddr_, _espNowReceiveBuffer, _espNowReceiveLen);
    _espNowReceived = true;
  }
}

void onEspNowSent(const uint8_t * macAddr_, esp_now_send_status_t status) {
  if constexpr (CAT::isEspNowSender) {
    g_cat.onEspNowSent(macAddr_, status);
  }
}

};

void setup() {
  Serial.begin(MCU::bandRate);
  if constexpr (MCU::needDelayForSerialSetup) {
    delay(300);
  }

  Serial.println("::setup");

  // ESPNOW
  if constexpr (CAT::isEspNowReceiver || CAT::isEspNowSender) {
    bcstem::setupEspNow();

    if constexpr (CAT::isEspNowReceiver)
    {
      Serial.println("isEspNowReceiver");
      bcstem::setupEspNowReceiver(bcstem::onEspNowReceived);
    }

    if constexpr (CAT::isEspNowSender)
    {
      Serial.println("isEspNowSender");
      bcstem::setupEspNowSender(CAT::espNowTargetAddr, bcstem::onEspNowSent);
    }

  }

  // I2C
  if constexpr (CAT::needI2C) {
    Serial.print("needI2C SDA:");
    Serial.print(PinMap::SDA);
    Serial.print(" SLC:");
    Serial.println(PinMap::SLC);
    if (Wire.begin(PinMap::SDA, PinMap::SLC)) {
      Serial.println("I2C initialized");
    } else {
      bcstem::GlobalObject::failSetup();
      Serial.println("I2C initialization failed");
    }
  }

  // cat specific setup
  bcstem::g_cat.setup();

  // End setup
  Serial.println(bcstem::GlobalObject::get().errorLog);

  //bcstem::scanI2C();


}

void loop() {

  //Serial.println("::loop");
  bcstem::_cnt++;
  //Serial.println(_cnt);

  if (!bcstem::GlobalObject::get().setupPassed()) {
    if (bcstem::_cnt==1) {
      Serial.println("setup is not passed");
    }
    return;
  }

  if (bcstem::_espNowReceived) {
    
    noInterrupts();
    uint8_t buffer[bcstem::MAX_ESPNOW_SIZE];
    int len = bcstem::_espNowReceiveLen;
    memcpy(buffer, bcstem::_espNowReceiveBuffer, len);
    bcstem::_espNowActionCount++;
    auto espNowReceiveCount = bcstem::_espNowReceiveCount;
    bcstem::_espNowReceived = false;
    interrupts();

    if (espNowReceiveCount!=bcstem::_espNowActionCount) {
      Serial.print("ESP-NOW receive count: ");
      Serial.print(espNowReceiveCount);
      Serial.print(" / action count: ");
      Serial.println(bcstem::_espNowActionCount);
    }

    bcstem::g_cat.onEspNowReceivedAction(buffer, len);
  }

  // cat specific loop
  bcstem::g_cat.loop();

/*
  if (g_timerServo.fired(currentMs))
  {
    g_angle++;
    if (g_angle>=180) { g_angle = 0; }
    g_servo.write(g_angle);
  }
*/
  //if (_cnt%100000==0) {
  //  Serial.println(_cnt);
  //}

  //Serial.println("::loop end");

}