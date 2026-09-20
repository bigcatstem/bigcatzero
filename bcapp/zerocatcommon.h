#pragma once

namespace bcstem {

enum CatMode : uint8_t {
  RANDOM_WALKER = 0,
  REMOTE_WALKER
};

struct JoystickEvent {
  int16_t x;
  int16_t y;
  CatMode catMode;

  void print() const {
    Serial.print("JoystickEvent ");
    Serial.print(x);Serial.print(",");
    Serial.print(y);Serial.print(" catMode:");
    Serial.print(catMode);
  }

  void println() const {
    Serial.print("JoystickEvent ");
    Serial.print(x);Serial.print(",");
    Serial.print(y);Serial.print(" catMode:");
    Serial.println(catMode);
  }

};




}