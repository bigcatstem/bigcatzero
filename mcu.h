#pragma once

namespace bcstem {

struct ESP32C3 {
  static const bool needDelayForSerialSetup = true;
  static const long bandRate = 115200;
  // MAC Address: b0:cb:d8:c6:52:04
};

struct ESP32S {
  static const bool needDelayForSerialSetup = false;
  static const long bandRate = 115200;
  // MAC Address: b0:cb:d8:c6:52:04
};
} // bcstem