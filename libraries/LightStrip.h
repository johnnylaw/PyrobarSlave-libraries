#pragma once

#include <Arduino.h>
#include "PololuLedStrip.h"

typedef struct LightStrip {
  PololuLedStripBase *pololuStrip;
  int voltage;
} LightStrip;
