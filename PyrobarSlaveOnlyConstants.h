#ifndef _PYROBAR_SLAVE_ONLY_CONSTANTS_H
#define _PYROBAR_SLAVE_ONLY_CONSTANTS_H

#include <math.h>
#include "PololuLedStrip.h"

#define DEBUG_SLAVE
#define SLAVE1 // if not, Slave is assumed as 0 when compiling slave sketch

typedef struct LightZoneInfo {
  uint8_t strip;
  uint16_t start,        // first index
           count;        // number of distinct addresses
  boolean isSymmetrical;
} LightZoneInfo;

typedef struct Location {
  uint8_t x, y;
} Location;

typedef struct Pixel {
  Location location;
  rgb_color lastColor;
} Pixel;

#endif