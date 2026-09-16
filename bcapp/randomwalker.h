#pragma once

//#include "motorset.h"
//#include "sonar.h"
#include <ESP32Servo.h>

namespace bcstem {

template<typename SONAR, typename MOTORSET>
class RandomWalker {

enum {
  ACTION_FORWARD = 0,
  ACTION_RIGHT,
  ACTION_LEFT,
  ACTION_BACK,
  //ACTION_BACK_SMALL,
  ACTION_STOP,
  ACTION_MAX
};

enum {
  MODE_NORMAL,
  MODE_BLOCKED
};

public:

  RandomWalker(SONAR& sonar_, MOTORSET& motors_, Servo& servo_):
    _sonar(sonar_),
    _motors(motors_),
    _servo(servo_)
  {}

  void setup() {
    Serial.println("Walker::setup");
    minSteps[ACTION_FORWARD] = 3000;
    minSteps[ACTION_RIGHT] = 200;
    minSteps[ACTION_LEFT] = 200;
    minSteps[ACTION_BACK] = 200;
    //minSteps[ACTION_BACK_SMALL] = 100;
    minSteps[ACTION_STOP] = 500;
  }

  void nextRandomAction() {
    int newAction = random(ACTION_FORWARD, ACTION_LEFT+1);
    if (newAction!=_action) {
      action(newAction);
    }
  }

  void actionWithDelay(int action_)
  {
    action(action_);
    delay(minSteps[action_]);
  }

  void action(int action_) {
    switch(action_) {
      case ACTION_FORWARD: _motors.forward(); break;
      case ACTION_RIGHT:   _motors.rotateRight(); break;
      case ACTION_LEFT:    _motors.rotateLeft(); break;
      case ACTION_BACK:    _motors.backward(); break;
      //case ACTION_BACK_SMALL:    _motors.backward(); break;
      case ACTION_STOP:    _motors.stop(); break;
      default:             _motors.stop();
    }
    _action = action_;
    _duration = random(minSteps[_action], minSteps[_action]*2);
    _actionStartMs = millis();
  }


  void normalAction() {
    //Serial.println(_cnt);
    unsigned long currentMillis = millis();
    //Serial.print(currentMillis); Serial.print(' ');
    //Serial.print(_actionStartMs); Serial.print(' ');
    //Serial.println(_duration); 
    if (currentMillis - _actionStartMs > _duration) {
      //Serial.println(_cnt);
      //Serial.println("nextRandomAction");
      nextRandomAction();

      //Serial.println(blocked);
      //Serial.println(_cnt);
      //Serial.println("return2");
      return;
    }
    //Serial.println("return");
    return;
  }

  void actionIfTooClose(bool needScan_) {
    int cm = _sonar.ping();

    if (cm < 5) {
      actionWithDelay(ACTION_BACK);
    }

    if (needScan_) {
      _resolveDirection = blockedScan();
    }

  }

  // return action
  int blockedScan() {

    _servo.write(0);
    delay(200);
    const int N = 9;
    int scan[N];
    for (int i=0;i<N;i=i+1) {
      int a = i*180/N;
      if (i==N-1) { a = 179; }
      a = a<179? a:179;
      _servo.write(a);
      scan[i] = _sonar.ping();
    }
    delay(100);
    _servo.write(90);

    int bestDir = -1;
    int right = 0;
    int left = 0;
    int front = 0;
    for (int i=0;i<N;i=i+1) {
      if (i<(N-1)/2) {
        right += scan[i];
      } else if (i>(N-1)/2) {
        left += scan[i];
      } else {
        front = scan[i];
      }
    }

    right = right / ((N-1)/2);
    left = left / ((N-1)/2);

    if (right > left && right > front) {
      return ACTION_RIGHT;
    }
    if (left > right && left > front) {
      return ACTION_LEFT;
    }

    //return ACTION_FORWARD;
    return ACTION_LEFT;

  }

  void loop() {
    //Serial.println("Walker::loop");

    //_cnt++;

    //unsigned long dCm = _sonar.ping();
    //Serial.println(dCm);
    //Serial.println(_action);

    bool blocked = _sonar.isBlocked();
    //Serial.println(blocked);

    if (blocked && _mode!=MODE_BLOCKED) { // New blocked

      Serial.println("New blocked ");
      action(ACTION_STOP);

      _resolveDirection = blockedScan();
      _blockedStartMs = millis();

      //Serial.print(_cnt);
      //Serial.println(blocked);

      //Serial.println(dCm);
      //if (!_sonar.isBlocked()) {
      //  Serial.println("False alarm");
      //  return;
      //}

      //action(ACTION_STOP);
      //::delay(200);
      _mode = MODE_BLOCKED;

      actionIfTooClose(false);

      //if (scanDir!=ACTION_LEFT && scanDir!=ACTION_RIGHT)
      //if (_resolveDirection==0) {
      actionWithDelay(_resolveDirection);
      //} else {
        //action(ACTION_RIGHT);
        //delay(500);
      //}
      return;
    }

    bool isFree = _sonar.isFree();

    //if (!blocked && _mode==MODE_BLOCKED) { // Unblock
    if (isFree && _mode==MODE_BLOCKED) {

      _mode = MODE_NORMAL;
      int cm = _sonar.ping();

      if (cm > 30) {
        action(ACTION_FORWARD);
      } else {
        nextRandomAction();
      }

      //action(ACTION_FORWARD);
      return ;
    }

    if (!blocked && _mode==MODE_NORMAL) { // Normal
      normalAction();
      return;
    }

    if (blocked && _mode==MODE_BLOCKED) { // Old block
      Serial.println("Old block");


      if (millis() - _blockedStartMs > 2000) {
        Serial.println("Old block 2");
        //action(ACTION_STOP);
        //delay(200);
        //action(ACTION_BACK);
        //delay(500);
        actionIfTooClose(true);
        actionWithDelay(_resolveDirection);
      } else {
        actionIfTooClose(false);
      }
      return;
    }

    //Serial.println(_action);

  }

private:

  SONAR& _sonar;
  MOTORSET& _motors;
  Servo& _servo;

  unsigned long _actionStartMs = 0;
  unsigned long _blockedStartMs = 0;
  int _action = ACTION_STOP;
  int _mode = MODE_NORMAL;
  int _duration = 0;
  int _resolveDirection = 0;
  int minSteps[ACTION_MAX];

};

} // namespace bcstem
