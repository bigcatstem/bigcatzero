#include "zerocat.h"


extern bcstem::ZeroCat g_cat;


namespace bcstem {


bool isBetween(int x_, int a_, int b_) {
  return x_>=a_ && x_<=b_;
}

static void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  XY16 xy;
  memcpy(&xy, incomingData, sizeof(xy));
  Serial.print(len);
  Serial.print(" , ");
  Serial.print(xy.x);
  Serial.print(" , ");
  Serial.println(xy.y);

  auto& motors = g_cat.motors();
  if (isBetween((xy.y)/8,-100,100)) {
    if (xy.x/8 > 100) {
      motors.backward();
    } else if (xy.x/8 < -100) {
      motors.forward();
    } else {
      motors.stop();
    }
    //motors.moveAnalog((xy.x-2048)/8, (xy.x-2048)/8);
  } else if (xy.y>0) {
    motors.rotateLeft();
    //motors.moveAnalog(0, (xy.x-2048)/8);
  } else {
    motors.rotateRight();
    //motors.moveAnalog((xy.x-2048)/8, 0);
  }
  
}



}; // namespace bcstem