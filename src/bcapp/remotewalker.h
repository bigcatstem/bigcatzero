#pragma once

#include <ESP32Servo.h>

namespace bcstem {

template<typename CFG, typename SONAR, typename MOTORSET>
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
      return;
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

    uint8_t q = 0;
    const float pi = 3.14;
    if (x>0 && y>=0) {
      ft = ft;
      q = 1;
    } else if (x<=0 && y>0) {
      ft = pi - ft;
      q = 2;
    } else if (x<0 && y<=0) {
      ft = pi + ft;
      q = 3;
    } else if (x>=0 && y<0) {
      ft = 2*pi - ft;
      q = 4;
    }
    
    int32_t r = sqrt (fx*fx + fy*fy);
    int16_t o = r * cos(2*(ft-3.14/2));

    /*
    Serial.print("onControl ");
    Serial.print(ft*180/3.14);Serial.print(" ");
    Serial.print(r);Serial.print(" ");
    Serial.print(o);Serial.print(" ");
    Serial.println(q);
    */

    if (q==1) {
      _motors.moveAnalog(r, o);
    } else if (q==2) {
      _motors.moveAnalog(o, r);
    } else if (q==3) {
      _motors.moveAnalog(r*-1, o*-1);
    } else if (q==4) {
      _motors.moveAnalog(o*-1, r*-1);
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

    bool wasBlocked = _isBlocked;
    _isBlocked = _sonar.isBlocked();
    if (_isBlocked) {

      _motors.moveAnalog(-75, -75);

      if (!wasBlocked) {
        testServo(_servo);
      }

    }

    if (wasBlocked && !_isBlocked) {
      _motors.moveAnalog(0, 0);      
    }

  }

private:

  SONAR& _sonar;
  MOTORSET& _motors;
  Servo& _servo;

  CatMode _catMode;

  bool _isBlocked = false;

};

} // namespace bcstem