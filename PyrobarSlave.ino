#include <Wire.h>
#include "PyrobarSlaveConstants.h"
#include "PyrobarSlaveOnlyConstants.h"
#include "PyrobarLightStrip.h"
#include "ZoneMapping.h"
#include "LightStrip.h"

#define SLAVE 0

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

void setup() {
  createZoneMappings();
  Serial.begin(115200);

  Serial.print("Setting up I2C at "); Serial.println(BASE_I2C_ADDRESS + SLAVE);
  Wire.begin(BASE_I2C_ADDRESS + SLAVE);
  Wire.onReceive(parseLightValues);

  Serial.print("Slave board "); Serial.println(SLAVE);
  Serial.print("Zone count: "); Serial.println(zoneCount);
  Serial.print("Strip count: "); Serial.println(stripCount);
  Serial.print("Size of tempColors: "); Serial.println(sizeof(tempColors));

  delay(1000);
}

void loop() {
  delay(100);
}

void parseLightValues(int packetSize) {
  for (int zoneIndex = 0; zoneIndex < zoneCount; zoneIndex++) {
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

