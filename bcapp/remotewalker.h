#pragma once

#include <ESP32Servo.h>

namespace bcstem {

template<typename SONAR, typename MOTORSET>
class RemoteWalker {

public:

  RemoteWalker(SONAR& sonar_, MOTORSET& motors_, Servo& servo_):
    _sonar(sonar_),
    _motors(motors_),
    _servo(servo_)
  {}

  // -2048->2048
  void onControl(int16_t x_, int16_t y_) {

    Serial.print("onControl ");
    Serial.print(x_);
    Serial.print(",");
    Serial.println(y_);

    if (x_==0 && y_==0) {
      _motors.moveAnalog(0, 0);
    }

    if (_isBlocked) {
      _motors.moveAnalog(0, 0);
      _isBlocked = _sonar.isBlocked();
      //return;
    }

    // -256->256
    int16_t x = x_ / 8;
    int16_t y = y_ / 8;

    float fy = abs(y);
    float fx = abs(x);
    float ft = atan(fy/fx);
    int32_t r = sqrt (fx*fx + fy*fy);
    if (x_ > 0) {
      r = r * -1;
    }
    int16_t o = r * cos(2*ft);

    if (x_>=0) {
      if (y_<=0) {
        _motors.moveAnalog(o, r);
      } else {
        _motors.moveAnalog(r, o);
      }
    } else {
      if (y_<=0) {
        _motors.moveAnalog(r, o);
      } else {
        _motors.moveAnalog(o, r);
      }
    }

    if (_sonar.isBlocked()) {
      _isBlocked = true;
      _motors.moveAnalog(0, 0);
    }

  }

  void setup() {
    Serial.println("RemoteWalker::setup");
  }

  void loop() {

    _isBlocked = _sonar.isBlocked();
    if (_isBlocked) {
      _motors.moveAnalog(-75, -75);
    }

  }

private:

  SONAR& _sonar;
  MOTORSET& _motors;
  Servo& _servo;

  bool _isBlocked = false;

};

} // namespace bcstem