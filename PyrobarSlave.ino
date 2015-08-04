#include <Wire.h>
#include "PyrobarSlaveConstants.h"
#include "PyrobarSlaveOnlyConstants.h"
#include "PyrobarLightStrip.h"
#include "ZoneMapping.h"
#include "LightStrip.h"

#define SLAVE 0

//////// LEGACY
const int lowZone = slaveZoneAddresses[SLAVE].low;
const int highZone = slaveZoneAddresses[SLAVE].high;
//////// END LEGACY

#if SLAVE == 0

static ZoneStripMappingSet zoneStripMappingSets[7];
const int zoneCount = 7;
const int stripCount = 5;
rgb_color tempColors[stripCount][300];

PololuLedStrip<37> strip0;
PololuLedStrip<35> strip1;
PololuLedStrip<33> strip2;
PololuLedStrip<31> strip3;
PololuLedStrip<29> strip4;

LightStrip lightStrips[] = {
  {&strip0, 12, 104},
  {&strip1, 12, 100},
  {&strip2, 5, 300},
  {&strip3, 5, 300},
  {&strip4, 12, 19},
};

#elif SLAVE == 1

static ZoneStripMappingSet zoneStripMappingSets[2];
const int zoneCount = 2;
const int stripCount = 2;
rgb_color tempColors[stripCount][350];

PololuLedStrip<37> strip0;
PololuLedStrip<35> strip1;

LightStrip lightStrips[] = {
  {&strip0, 12, 50},
  {&strip1, 5, 1050},
};

#endif

void createZoneMappings(void) {
#if SLAVE == 0

zoneStripMappingSets[0].push(ZoneStripMapping{0, 0, 18});     // Crane ring
zoneStripMappingSets[1].push(ZoneStripMapping{0, 18, 27});    // Crane top
zoneStripMappingSets[2].push(ZoneStripMapping{0, 45, 29});    // Crane middle
zoneStripMappingSets[3].push(ZoneStripMapping{0, 74, 29});    // Crane bottom
zoneStripMappingSets[4].push(ZoneStripMapping{1, 0, 100});    // Lounge
zoneStripMappingSets[5].push(ZoneStripMapping{2, 0, 300});    // Bar ceiling
zoneStripMappingSets[6].push(ZoneStripMapping{3, 0, 300});    // Bar surface ...
zoneStripMappingSets[6].push(ZoneStripMapping{4, 0, 19});     //   ... and DJ booth

#elif SLAVE == 1

zoneStripMappingSets[0].push(ZoneStripMapping{0, 0, 25});     // Steps
zoneStripMappingSets[1].push(ZoneStripMapping{1, 18, 1050});  // Undercarriage

#endif
}

//// LEGACY
#if SLAVE == 0

const int stripType[stripCount] = {  // use voltage of strip
  12,       // Crane
  12,       // Lounge (currently no lounge; will probably end up as 5)
  5,        // Bar ceiling
  5,        // Bar surface
  12        // DJ booth
};

const int stripAddressCounts[stripCount] = {
  104, // Crane         (zones 0 - 3)
  100, // Lounge        (zone 4)
  300,  // Bar ceiling   (zone 5)
  300,  // Bar surface   (zone 6)
  30   // DJ booth      (zone 7)
};

#elif SLAVE == 1

// TODO: Figure out undercarriage and steps strip lengths; undercarriage probably closer to 1050

const int stripType[stripCount] = {
  12,   // Steps
  5,    // Undercarriage
};

const int stripAddressCounts[stripCount] = {
  50,  // Steps                 (zone 8)
  300  // Undercarriage         (zone 9)
};

#endif

#if SLAVE == 0

LightZoneInfo lightZonesInfo[zoneCount + 1] = {
  {0, 0, 18, false},    // Crane ring
  {0, 18, 27, false},   // Crane top
  {0, 45, 29, false},   // Crane middle
  {0, 74, 29, false},   // Crane base
  {1, 0, 50, true},     // Lounge
  {2, 0, 150, true},     // Bar ceiling
  {3, 0, 150, true},     // Bar surface
  {4, 0, 19, true},     // DJ booth
};
PololuLedStripBase *ledStrips[stripCount] = {&strip0, &strip1, &strip2, &strip3, &strip4};

#elif SLAVE == 1

LightZoneInfo lightZonesInfo[zoneCount] = {
  {0, 0, 25, true},    // Steps
  {1, 0, 150, true}  // Undercarriage (offset will change for cartesian system)
};
PololuLedStripBase *ledStrips[stripCount] = {&strip0, &strip1};

#endif
//// END LEGACY


void setup() {
  createZoneMappings();
  Serial.begin(115200);

  Serial.print("Setting up I2C at "); Serial.println(BASE_I2C_ADDRESS + SLAVE);
  Wire.begin(BASE_I2C_ADDRESS + SLAVE);
  Wire.onReceive(parseIncoming);

  Serial.print("Slave board "); Serial.println(SLAVE);
  Serial.print("Zone count: "); Serial.println(zoneCount);
  Serial.print("Zones "); Serial.print(lowZone); Serial.print(" thru "); Serial.println(highZone);
  Serial.print("Strip count: "); Serial.println(stripCount);
  Serial.print("Size of tempColors: "); Serial.println(sizeof(tempColors));

  delay(2000);
  delay(1000);
}

void loop() {
  delay(100);
}

void parseLightValues(int packetSize) {
  int zoneIndex = zoneCount;
  while (zoneIndex--) {
    writeBuffer(zoneIndex, Wire.read(), Wire.read(), Wire.read());
  }
  while (Wire.read());
  writeStrips();
}

void writeBuffer(int zoneIndex, uint8_t red, uint8_t green, uint8_t blue) {
  ZoneStripMappingSet mappingSet = zoneStripMappingSets[zoneIndex];
  for (int mapIndex = 0; mapIndex < mappingSet.size(); mapIndex++) {
    ZoneStripMapping mapping = mappingSet[mapIndex];
    rgb_color color = rgb_color(red, green, blue, lightStrips[mapping.strip].voltage);
    for (int address = mapping.start; address < mapping.start + mapping.count; address++) {
      tempColors[mapping.strip][address] = color;
    }
  }
}

void writeStrips() {
  for (int stripInd = 0; stripInd < stripCount; stripInd++) {
    LightStrip strip = lightStrips[stripInd];
    strip.pololuStrip->write(tempColors[stripInd], strip.count);
  }
}


//    LEGACY TO END OF FILE
void parseIncoming(int packetSize) {
#ifdef DEBUG_SLAVE
  Serial.print("Incoming: ");
#endif
  for (int zoneIndex = lowZone; zoneIndex <= highZone; zoneIndex++) {
    rgb_color color;
    LightZoneInfo info = lightZonesInfo[zoneIndex];
    if (stripType[info.strip] == 12) {
      color.green = Wire.read();
      color.blue = Wire.read();
      color.red = Wire.read();
    } else {
      color.red = Wire.read();
      color.green = Wire.read();
      color.blue = Wire.read();
    }
    writeEntireZoneBuffer(zoneIndex, color);
  } 
  writeToStrips();
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
  for (int address = zoneInfo.start; address < zoneInfo.count * multiplier + zoneInfo.start; address++) {
    tempColors[zoneInfo.strip][address] = color;
  }
}

