#pragma once

#include <Arduino.h>
#include "LightStripInfo.h"

typedef struct RGBColor {
  unsigned char red, green, blue;

  RGBColor(unsigned char c1, unsigned char c2, unsigned char c3, LightStripInfo info) {
    if (info.maxBrightness) {
      attenuate(&c1, &c2, &c3, info.maxBrightness);
    }

    if (info.voltage == 12) {
      green = c1; blue = c2; red = c3;
    } else {
      red = c1; green = c2; blue = c3;
    }
  }
  RGBColor() : red(0), green(0), blue(0) {}

  void attenuate(unsigned char *c1, unsigned char *c2, unsigned char *c3, int maxBrightness) {
    float brightness = *c1 + *c2 + *c3;
    // Serial.print("Brightness: ");
    // Serial.println(brightness);
    if (brightness > maxBrightness) {
      float c1f = (float)*c1 / 255.0;
      float c2f = (float)*c2 / 255.0;
      float c3f = (float)*c3 / 255.0;

      float adjustment = log2f(brightness / (float)maxBrightness);
      // Serial.print("Adjustment: ");
      // Serial.println(adjustment);
      float pow1 = log2f(c1f) - adjustment;
      float pow2 = log2f(c2f) - adjustment;
      float pow3 = log2f(c3f) - adjustment;

      *c1 = powf(2.0, pow1) * 256 - 1;
      *c2 = powf(2.0, pow2) * 256 - 1;
      *c3 = powf(2.0, pow3) * 256 - 1;
    }
  }
} RGBColor;

