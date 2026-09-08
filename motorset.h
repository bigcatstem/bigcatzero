#pragma once

namespace bcstem {

template<int FL1, int FL2, int FR1, int FR2//,
         //int BL1, int BL2,
         //int BR1, int BR2
>
class MotorSetT {

static const int FRONT = 1;
static const int BACK = -1;
static const int STOP = 0;

public:

  void initialize() {
    Serial.println("MotorSetT::initialize");
    const int pins[] = {
      FL1, FL2, FR1, FR2//,
      //BL1, BL2, BR1, BR2
    };

    for (auto pin: pins){
      pinMode(pin, OUTPUT);
    }

    stop();
  }

  void stop() {
    moveSingle(FL1, FL2, STOP);
    moveSingle(FR1, FR2, STOP);
  }

  // level: -255 to 255
  void moveAnalogSingle(int pin1_, int pin2_, int level_) {
    level_ = level_ >  255?  255 : level_;
    level_ = level_ < -255? -255 : level_;

    if (level_ > 0) {
      analogWrite(pin1_, level_);
      analogWrite(pin2_, 0);
    } else if (level_ < 0) {
      analogWrite(pin1_, 0);
      analogWrite(pin2_, level_*-1);
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

  void moveSingle(int pin1_, int pin2_, int level_) {
    if (level_ == FRONT) {
      digitalWrite(pin1_, HIGH);
      digitalWrite(pin2_, LOW);
    } else if (level_ == BACK) {
      digitalWrite(pin1_, LOW);
      digitalWrite(pin2_, HIGH);
    } else {
      digitalWrite(pin1_, LOW);
      digitalWrite(pin2_, LOW);
    }
  }

  // l/r: FRONT / BACK / STOP
  void move(int l_, int r_) {
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

};

} // namespace bcstem
