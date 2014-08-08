#pragma once

#include <Arduino.h>
#include "PyrobarSlaveConstants.h"

class PyrobarLightStrip {
public:
  PyrobarLightStrip(Pixel *pixels, int pixelCount, boolean isSymmetrical, int offset, rgb_color *buffer);

private:
  Pixel *_pixels;
  int _pixelCount;
  boolean _isSymmetrical;
  int _offset;
  rgb_color *_buffer;

public:
  void setBall(Location center, uint8_t radius, rgb_color color);
  void update(float decay);

private:
  float distanceFromCenter(Location location0, Location location1);
  float calculateIntensity(int pixelInd, Location center, uint16_t radius);
  void writeColor(int pixelInd, float decay);
  void setCurrentColor(int pixelInd, rgb_color color, float intensity);
};