#pragma once

#include <ESP32Servo.h>

namespace bcstem {

template<typename CFG, typename SONAR, typename MOTORSET>
class Follower {

public:

  Follower(SONAR& sonar_, MOTORSET& motors_, Servo& servo_):
    _sonar(sonar_),
    _motors(motors_),
    _servo(servo_)
  {}

  void setup() {
    Serial.println("Follower::setup");
  }

  void loop() {

    auto sonarResult = _sonar.check(100, 200, 600);

    int speed = 255;

    if (sonarResult != _lastSonarResult) {
      if (sonarResult == SonarBase::Result::TOO_CLOSE) {
        _motors.moveAnalog(speed*-1, speed*-1);
      } else if (sonarResult == SonarBase::Result::IN_RANGE) {
        _motors.moveAnalog(0, 0);
      } else if (sonarResult == SonarBase::Result::TOO_FAR) {
        _motors.moveAnalog(speed, speed);
      } else if (sonarResult == SonarBase::Result::OUT_OF_REACH) {
        _motors.moveAnalog(0, 0);
      }
      _lastSonarResult = sonarResult;
    }
  }

private:

  SONAR& _sonar;
  MOTORSET& _motors;
  Servo& _servo;

  SonarBase::Result _lastSonarResult = SonarBase::Result::OUT_OF_REACH;

};

} // namespace bcstem