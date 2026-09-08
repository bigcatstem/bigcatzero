#pragma once

namespace bcstem {

struct ESP32C3Zero {

/*
5V 
GND 
3.3V 
GPIO20 
GPIO21 
GPIO2 
GPIO1 
GPIOO 

GPI010 
GPI09  SCL
GPIO8  SDA
GPIO7 
GPI06 
GPIO5
GP104 
GPIO3 
*/

  static const bool needDelayForSerialSetup = true;
  static const long bandRate = 115200;
  static const int SDA = 8;
  static const int SLC = 9;

  // MAC Address: b0:cb:d8:c6:52:04
};

struct ESP32S {
  static const bool needDelayForSerialSetup = false;
  static const long bandRate = 115200;
  static const int SDA = 21;
  static const int SLC = 22;
  // MAC Address: b0:cb:d8:c6:52:04
};
} // bcstem