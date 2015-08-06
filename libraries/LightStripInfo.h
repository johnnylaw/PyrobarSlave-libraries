#pragma once

#include <Arduino.h>

typedef struct LightStripInfo {
    int voltage;
    int count;
    int maxBrightness;
} LightStripInfo;
