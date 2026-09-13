/*
 - In ESP, Some pin will reset the MCU
 - USB CDC on boot
 - ~/Library/Arduino15/packages/arduino/hardware/avr/<version>/boards.txt
Done:
  - avoid D15 / D5
TODO:
  - Joystick button
  - Better natural reaction from joystick to motor 
*/

//#include <Wire.h>

// Util
#include "bcstem/util.h"

// App
#include "bcapp/zerocat.h"
#include "bcapp/zeroremote.h"
using CAT = bcstem::ZeroCat;
//using CAT = bcstem::ZeroRemote;
CAT g_cat;

//using namespace bcstem;
using PinMap = CAT::PinMap;
using MCU = CAT::PinMap::MCU;

//auto g = bcstem::GlobalObject::get();
long _cnt = 0;

// So that, the handle in CAT can be a normal member function
// IsEnable
void onEspNowReceived(const uint8_t * macAddr_, const uint8_t *incomingData_, int len_) {

  if constexpr (CAT::isEspNowReceiver) {
    g_cat.onEspNowReceived(macAddr_, incomingData_, len_);
  }
}

void onEspNowSent(const uint8_t * macAddr_, esp_now_send_status_t status) {
  if constexpr (CAT::isEspNowSender) {
    g_cat.onEspNowSent(macAddr_, status);
  }
}

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
      bcstem::setupEspNowReceiver(onEspNowReceived);
    }

    if constexpr (CAT::isEspNowSender)
    {
      bcstem::setupEspNowSender(CAT::espNowTargetAddr, onEspNowSent);
    }

  }

  // I2C
  if constexpr (CAT::needI2C) {
    Wire.begin(PinMap::SDA, PinMap::SLC);
  }

  // cat specific setup
  g_cat.setup();

  // End setup
  Serial.println(bcstem::GlobalObject::get().errorLog);

}

void loop() {

  //Serial.println("::loop");
  //_cnt++;
  //Serial.println(_cnt);

  if (!bcstem::GlobalObject::get().setupPassed()) {
    Serial.println("setup is not passed");
    return;
  }

  //Serial.println(g_sonar.ping());
  // cat specific loop
  g_cat.loop();

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
}
