#pragma once

#include <Arduino.h>
#include "LightStripInfo.h"
#include "PololuLedStrip.h"

typedef struct LightStrip {
  PololuLedStripBase *pololuStrip;
  LightStripInfo info;
} LightStrip;

