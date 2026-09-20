#pragma once

// Set MIN_OUTPUT, so that the motor is more responsive to INPUT.  Dead-zone

namespace bcstem {

template<int FL1, int FL2, int FR1, int FR2, typename CFG>
class MotorSetT {

using MCFG = CFG::Motor;

using ScaledSpeedMapper = ScaledSpeedMapperT<MCFG::LOGICAL_MAX, MCFG::SCALED_LB, MCFG::SCALED_UB>;

enum Dir : int8_t {
  FRONT = 1,
  BACK = -1,
  STOP = 0
};

public:

  void setup() {
    Serial.println("MotorSetT::setup");
    const int pins[] = {
      FL1, FL2, FR1, FR2
    };

    for (auto pin: pins){
      ledcAttach(pin, MCFG::PWM_FREQUENCY, MCFG::PWM_RESOLUTION);
      //pinMode(pin, OUTPUT);
    }

    _defaultSpeed = ScaledSpeedMapper::mid();

    stop();
  }

  void stop() {
    moveSingle(FL1, FL2, STOP);
    moveSingle(FR1, FR2, STOP);
  }

  void setSpeed(Speed speed_) {
    _defaultSpeed = ScaledSpeedMapper::map(speed_);
  }

  // level: -255 to 255
  // Out be too small
  // IN          OUT
  // [0,MAX]     [MIN,MAX]
  // [0,-MAX]    [-MIN,-MAX]
  void moveAnalogSingle(int pin1_, int pin2_, int speed_) {

    ScaledSpeed scaled = ScaledSpeedMapper::map(speed_);

    Serial.print("moveAnalogSingle ");
    Serial.print(speed_);
    Serial.print("->");
    Serial.println(scaled);

    if (scaled > 0 ) {
      analogWrite(pin1_, scaled);
      analogWrite(pin2_, 0);
    } else if (scaled < 0 ) {
      analogWrite(pin1_, 0);
      analogWrite(pin2_, scaled*-1);
    } else {
      analogWrite(pin1_, 0);
      analogWrite(pin2_, 0);
    }

  }

  // l/r: -255 to 255
  void moveAnalog(int l_, int r_) {
    moveAnalogSingle(FL1, FL2, l_);
    moveAnalogSingle(FR1, FR2, r_);
  }
  
  void moveSingle(int pin1_, int pin2_, Dir dir_) {
    if (dir_ == FRONT) {
      moveAnalogSingle(pin1_, pin2_, _defaultSpeed);
    } else if (dir_ == BACK) {
      moveAnalogSingle(pin1_, pin2_, _defaultSpeed*-1);
    } else {
      moveAnalogSingle(pin1_, pin2_, 0);
    }
  }

  void moveSingleDigital(int pin1_, int pin2_, Dir dir_) {
    if (dir_ == FRONT) {
      digitalWrite(pin1_, HIGH);
      digitalWrite(pin2_, LOW);
    } else if (dir_ == BACK) {
      digitalWrite(pin1_, LOW);
      digitalWrite(pin2_, HIGH);
    } else {
      digitalWrite(pin1_, LOW);
      digitalWrite(pin2_, LOW);
    }
  }

  // l/r: FRONT / BACK / STOP
  void move(Dir l_, Dir r_) {
    moveSingle(FL1, FL2, l_);
    moveSingle(FR1, FR2, r_);
  }

  void forward() {
    move(FRONT, FRONT);
  }

  void backward() {
    move(BACK, BACK);
  }

  void rotateLeft() {
    move(BACK, FRONT);
  }

  void rotateRight() {
    move(FRONT, BACK);
  }

private:
  ScaledSpeed _defaultSpeed;

};

} // namespace bcstem
