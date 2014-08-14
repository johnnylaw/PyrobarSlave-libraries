//#include <SPI.h>
//#include <Ethernet.h>
#include <Wire.h>
#include "PyrobarSlaveConstants.h"
#include "PyrobarSlaveOnlyConstants.h"
#include "PyrobarLightStrip.h"

const int lowZone = slaveZoneAddresses[SLAVE].low;
const int highZone = slaveZoneAddresses[SLAVE].high;

#if SLAVE == 0

const int zoneCount = 8;
const int stripCount = 5;
rgb_color tempColors[stripCount][100];
const int stripAddressCounts[stripCount] = {100, 100, 90, 90, 30};

#elif SLAVE == 1

const int zoneCount = 2;
const int stripCount = 2;
rgb_color tempColors[stripCount][350];
const int stripAddressCounts[stripCount] = {50, 300};

#endif

const int lightProgramPacketSize = 3 * zoneCount;

#if SLAVE == 0

PololuLedStrip<37> strip0;
PololuLedStrip<35> strip1;
PololuLedStrip<33> strip2;
PololuLedStrip<31> strip3;
PololuLedStrip<29> strip4;

LightZoneInfo lightZonesInfo[zoneCount] = {
  {0, 0, 18, false},  // Crane ring
  {0, 18, 27, false},  // Crane top
  {0, 45, 27, false},  // Crane middle
  {0, 72, 27, false},   // Crane base
  {1, 0, 50, true},    // Lounge
  {2, 0, 45, true},    // Bar ceiling
  {3, 0, 45, true},    // Bar surface
  {4, 0, 15, true},    // DJ booth
};
PololuLedStripBase *ledStrips[stripCount] = {&strip0, &strip1, &strip2, &strip3, &strip4};

#elif SLAVE == 1

PololuLedStrip<37> strip0;
PololuLedStrip<35> strip1;

LightZoneInfo lightZonesInfo[zoneCount] = {
  {0, 0, 25, true},    // Steps
  {1, 0, 150, true}  // Undercarriage (offset will change for cartesian system)
};
PololuLedStripBase *ledStrips[stripCount] = {&strip0, &strip1};

#endif

//#if SLAVE == 0

//Pixel pixels0[100] = {{1000, 500}, {999, 499}}; // Crane and ring
//Pixel pixels1[50] =  {{1000, 500}}; // Lounge
//Pixel pixels2[45] =  {{1000, 500}}; // Bar ceiling
//Pixel pixels3[45] =  {{1000, 500}}; // Bar surface
//Pixel pixels4[15] =  {{1000, 500}}; // DJ booth
//PyrobarLightStrip lightStrips2d[ledStripCount] = {
//  {pixels0, 100, false, 0, tempColors[0]},
//  {pixels1, 50, true, 0, tempColors[1]},
//  {pixels2, 45, true, 0, tempColors[2]},
//  {pixels3, 45, true, 0, tempColors[3]},
//  {pixels4, 15, true, 0, tempColors[4]},
//};

//#elif SLAVE == 1

//Pixel pixels0[50] = {{1000, 500}, {999, 499}}; // Steps
//Pixel pixels1[175] =  {{1000, 500}}; // Undercarriage
//PyrobarLightStrip lightStrips2d[ledStripCount] = {
//  {pixels0, 50, true, 100, tempColors[0]},
//  {pixels1, 175, true, 0, tempColors[1]}
//};

//#endif

float halfLife;
unsigned long lastLoopTime;

LightMode lightMode = BALL_DRAG;

void setup() {
  Serial.begin(115200);

  Serial.print("Setting up I2C at "); Serial.println(BASE_I2C_ADDRESS + SLAVE);
  Wire.begin(BASE_I2C_ADDRESS + SLAVE);
  Wire.onReceive(parseIncoming);

  Serial.print("Slave board "); Serial.println(SLAVE);
  Serial.print("Zone count: "); Serial.println(zoneCount);
  Serial.print("Zones "); Serial.print(lowZone); Serial.print(" thru "); Serial.println(highZone);
  Serial.print("Strip count: "); Serial.println(stripCount);
  Serial.print("Size of tempColors: "); Serial.println(sizeof(tempColors));
  Serial.print("Light program packet size: "); Serial.println(lightProgramPacketSize);

  delay(2000);
  lastLoopTime = millis();
}

void loop() {
  lastLoopTime = millis();
  if (lightMode == BALL_DRAG) {
    float elapsedTime = (millis() - lastLoopTime) / 1000.0;
    float decayFactor = pow(0.5, elapsedTime / halfLife);
    for (int stripInd = 0; stripInd < stripCount; stripInd++) {
//      lightStrips2d[stripInd].update(decayFactor);
    }
    writeToStrips();
  }

  delay(10);
}

void parseIncoming(int packetSize) {
  Serial.print("Incoming: ");
  if (packetSize == 7) {   // light ball information (x, y, radius, red, green, blue, halfLifeInt)
    Serial.println("Light Ball");
    lightMode = BALL_DRAG;
    Location location = {Wire.read(), Wire.read()};
    uint8_t radius = Wire.read();
    rgb_color color = {Wire.read(), Wire.read(), Wire.read()};
    halfLife = Wire.read() / 128.0; // highest = 2.0
    for (int stripInd = 0; stripInd < stripCount; stripInd++) {
//      lightStrips2d[stripInd].setBall(location, radius, color);
    }
  } else if (packetSize == lightProgramPacketSize) {
#ifdef DEBUG_SLAVE
    Serial.println("Program");
#endif
    lightMode = PROGRAM;
    for (int zoneIndex = lowZone; zoneIndex <= highZone; zoneIndex++) {
      // Can't simply do the following because of reordering of colors
      // rgb_color color = {Wire.read(), Wire.read(), Wire.read()};
      rgb_color color;
      color.red = Wire.read();
      color.green = Wire.read();
      color.blue = Wire.read();
      writeEntireZoneBuffer(zoneIndex, color);
    } 

    writeToStrips();
  }
#ifdef DEBUG_SLAVE    
  else {
    Serial.print("random packet of ");
    Serial.println(packetSize);
  }
#endif 
}

void writeToStrips() {
  for (int stripInd = 0; stripInd < stripCount; stripInd++) {
    ledStrips[stripInd]->write(tempColors[stripInd], stripAddressCounts[stripInd]);
  }
}

void writeEntireZoneBuffer(int zoneIndex, rgb_color color) {
#ifdef DEBUG_SLAVE
  Serial.print("Writing zone "); 
  Serial.print(zoneIndex);
  Serial.print(" with ");
  Serial.print(color.red);
  Serial.print(", ");
  Serial.print(color.green);
  Serial.print(", ");
  Serial.print(color.blue);
#endif
  LightZoneInfo zoneInfo = lightZonesInfo[zoneIndex];
  int multiplier = zoneInfo.isSymmetrical ? 2 : 1;
  for (int address = zoneInfo.start; address < zoneInfo.count + zoneInfo.start; address++) {
    tempColors[zoneInfo.strip][address] = color;
  }
}

void igniteLightProgram() {
  lightMode = PROGRAM;
}

