#include <Wire.h>
#include "PyrobarSlaveConstants.h"
#include "PyrobarSlaveOnlyConstants.h"
#include "PyrobarLightStrip.h"

#define SLAVE 1

const int lowZone = slaveZoneAddresses[SLAVE].low;
const int highZone = slaveZoneAddresses[SLAVE].high;

#if SLAVE == 0

const int zoneCount = 8;
const int stripCount = 5;
rgb_color tempColors[stripCount][100];
const int stripAddressCounts[stripCount] = {
  100, // Crane         (zones 0 - 3)
  100, // Lounge        (zone 4)
  90,  // Bar ceiling   (zone 5)
  90,  // Bar surface   (zone 6)
  30   // DJ booth      (zone 7)
};
const int stripType[stripCount] = {  // use voltage of strip
  12,       // Crane
  12,       // Lounge (currently no lounge; will probably end up as 5)
  5,        // Bar ceiling
  5,        // Bar surface
  12        // DJ booth
}

#elif SLAVE == 1

const int zoneCount = 2;
const int stripCount = 2;
rgb_color tempColors[stripCount][350];
const int stripAddressCounts[stripCount] = {
  50,  // Steps                 (zone 8)
  300  // Undercarriage         (zone 9)
};
const int stripType[stripCount] = {
  12,   // Steps
  5,    // Undercarriage
};

#endif

const int lightProgramPacketSize = 3 * zoneCount;

#if SLAVE == 0

PololuLedStrip<37> strip0;
PololuLedStrip<35> strip1;
PololuLedStrip<33> strip2;
PololuLedStrip<31> strip3;
PololuLedStrip<29> strip4;

LightZoneInfo lightZonesInfo[zoneCount] = {
  {0, 0, 18, false},    // Crane ring
  {0, 18, 27, false},   // Crane top
  {0, 45, 27, false},   // Crane middle
  {0, 72, 27, false},   // Crane base
  {1, 0, 50, true},     // Lounge
  {2, 0, 45, true},     // Bar ceiling
  {3, 0, 45, true},     // Bar surface
  {4, 0, 15, true},     // DJ booth
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
}

void loop() {
  writeToStrips();
  delay(100);
}

void parseIncoming(int packetSize) {
  Serial.print("Incoming: ");
#ifdef DEBUG_SLAVE
  Serial.println("Program");
#endif
  for (int zoneIndex = lowZone; zoneIndex <= highZone; zoneIndex++) {
    rgb_color color = { Wire.read(), Wire.read(), Wire.read() };
    if (stripType[zoneIndex] == 12) {
      unsigned char temp = color.red;
      color.red = color.green;
      color.green = color.blue;
      color.blue = temp;
    }
    writeEntireZoneBuffer(zoneIndex, color);
  } 
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

