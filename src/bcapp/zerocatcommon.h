#pragma once

namespace bcstem {

enum CatMode : uint8_t {
  RANDOM_WALKER = 0,
  REMOTE_WALKER
};

struct JoystickEvent {
  uint16_t magic = 0xABCD;
  uint16_t seq = 0;
  uint8_t type = 0;
  int16_t x;
  int16_t y;
  CatMode catMode;

  JoystickEvent(int16_t x_, int16_t y_, CatMode catMode_) : x(x_), y(y_), catMode(catMode_) {
    type = 1;
  }

  void print() const {
    Serial.print("JoystickEvent ");
    Serial.print(seq);Serial.print(" ");
    Serial.print(x);Serial.print(",");
    Serial.print(y);Serial.print(" catMode:");
    Serial.print(catMode);
  }

  void println() const {
    Serial.print("JoystickEvent ");
    Serial.print(seq);Serial.print(" ");
    Serial.print(x);Serial.print(",");
    Serial.print(y);Serial.print(" catMode:");
    Serial.println(catMode);
  }

};




}