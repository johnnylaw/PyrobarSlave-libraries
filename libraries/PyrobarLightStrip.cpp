#include "PyrobarLightStrip.h"

PyrobarLightStrip::PyrobarLightStrip(Pixel *pixels, int pixelCount, boolean isSymmetrical, int offset, rgb_color *buffer) : _pixels(pixels), _pixelCount(pixelCount), _isSymmetrical(isSymmetrical), _offset(offset), _buffer(buffer) {
}

void PyrobarLightStrip::setBall(Location center, uint8_t radius, rgb_color color) {
  for (int pixelInd = 0; pixelInd < _pixelCount; pixelInd++) {
    setCurrentColor(pixelInd, color, calculateIntensity(pixelInd, center, radius));
  }
}

void PyrobarLightStrip::update(float decay) {
  for (int pixelInd = 0; pixelInd < _pixelCount; pixelInd++) {
    writeColor(pixelInd, decay);
  }
}

float PyrobarLightStrip::calculateIntensity(int pixelInd, Location center, uint16_t radius) {
  Location pixelLocation = _pixels[pixelInd].location;
  float distance = sqrt((center.x - pixelLocation.x) ^ 2 + (center.y - pixelLocation.y) ^ 2);
  if (distance > radius) return 0.0;
  return cos(distance / radius);
}

void PyrobarLightStrip::writeColor(int pixelInd, float decay) {
  Pixel *pixel = &_pixels[pixelInd];  // may not need a pointer here
  pixel->lastColor.red *= decay;
  pixel->lastColor.green *= decay;
  pixel->lastColor.blue *= decay;
  _buffer[pixelInd] = pixel->lastColor;
}

void PyrobarLightStrip::setCurrentColor(int pixelInd, rgb_color color, float intensity) {
  Pixel *pixel = &_pixels[pixelInd];
  pixel->lastColor.red   = max(pixel->lastColor.red, color.red * intensity);
  pixel->lastColor.green = max(pixel->lastColor.green, color.green * intensity);
  pixel->lastColor.blue  = max(pixel->lastColor.blue, color.blue * intensity);
}