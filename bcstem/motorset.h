#pragma once

// Set MIN_OUTPUT, so that the motor is more responsive to INPUT

namespace bcstem {

template<int FL1, int FL2, int FR1, int FR2//,
         //int BL1, int BL2,
         //int BR1, int BR2
>
class MotorSetT {

static constexpr int MIN_OUTPUT = 150; // Min value for the motor to move 
static constexpr int MAX_OUTPUT = 255;

static constexpr int FRONT = 1;
static constexpr int BACK = -1;
static constexpr int STOP = 0;

static constexpr uint32_t PWM_FREQUENCY = 20000;
static constexpr uint8_t PWM_RESOLUTION = 8;


public:

  void setup() {
    Serial.println("MotorSetT::setup");
    const int pins[] = {
      FL1, FL2, FR1, FR2//,
      //BL1, BL2, BR1, BR2
    };

    for (auto pin: pins){
      ledcAttach(pin, PWM_FREQUENCY, PWM_RESOLUTION);
      //pinMode(pin, OUTPUT);
    }

    stop();
  }

  void stop() {
    moveSingle(FL1, FL2, STOP);
    moveSingle(FR1, FR2, STOP);
  }

  // level: -255 to 255
  // Out be too small
  // IN          OUT
  // [0,MAX]     [MIN,MAX]
  // [0,-MAX]    [-MIN,-MAX]
  void moveAnalogSingle(int pin1_, int pin2_, int level_) {    
    level_ = level_ >  255?  255 : level_;
    level_ = level_ < -255? -255 : level_;

    int out = map(abs(level_), 1, 255, MIN_OUTPUT, MAX_OUTPUT);
    //int out = MIN_OUTPUT + (MAX_OUTPUT-MIN_OUTPUT) * abs(level_) / MAX_OUTPUT; // always positive

    //if (out > MAX_OUTPUT*0.9) { out = MAX_OUTPUT; }
    if (level_ == 0) { out = 0; }
    Serial.print("moveAnalogSingle ");
    Serial.print(level_);
    Serial.print("->");
    Serial.println(out);

    if (level_ > 0 ) {
      analogWrite(pin1_, out);
      analogWrite(pin2_, 0);
    } else if (level_ < 0 ) {
      analogWrite(pin1_, 0);
      analogWrite(pin2_, out);
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
