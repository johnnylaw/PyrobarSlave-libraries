#pragma once

#include <Arduino.h>
#include <Vector.h>

typedef struct ZoneStripMapping {
  uint16_t strip, start, count;
} ZoneStripMapping;

typedef Vector<ZoneStripMapping> ZoneStripMappingSet;