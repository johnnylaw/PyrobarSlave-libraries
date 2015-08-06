#include <Wire.h>
#include "PyrobarSlaveConstants.h"
#include "PyrobarSlaveOnlyConstants.h"
#include "ZoneMapping.h"
#include "LightStripInfo.h"
#include "LightStrip.h"

#define SLAVE 0

#if SLAVE == 0

static ZoneStripMappingSet zoneStripMappingSets[7]; // std::vector
const int zoneCount = 7;
const int stripCount = 5;
RGBColor tempColors[stripCount][300]; // Second size must be that of strip with highest number of bulbs

PololuLedStrip<37> strip0;
PololuLedStrip<35> strip1;
PololuLedStrip<33> strip2;
PololuLedStrip<31> strip3;
PololuLedStrip<29> strip4;

// LightStrip is struct with
//   - PololuLedStrip pointer
//   - LightStripInfo, which is struct with
//       - (int) strip voltage
//       - (int) strip bulb count
//       - (int) maximum total brightness of all 3 channels (optional)
//            NOTE: Max brightness is dependent on physical power available
//                  to the strip, as without this, dimming and loss of color
//                  (particularly blue) results in longer strips. Leaving this blank
//                  in struct initialization results in full brightness always.
// None of the strip bulb counts should be higher than secondary size of tempColors array
LightStrip lightStrips[] = {
  {&strip0, {12, 104}},       // Crane (4 zones)
  {&strip1, {12, 100}},       // Lounge (1 zone)
  {&strip2, {5, 300, 400}},   // Bar ceiling (1 zone)
  {&strip3, {5, 300, 400}},   // Bar surface (1 zone)
  {&strip4, {12, 19}},        // DJ booth (same zone as bar surface)
};

#elif SLAVE == 1

static ZoneStripMappingSet zoneStripMappingSets[2];
const int zoneCount = 2;
const int stripCount = 2;
rgb_color tempColors[stripCount][350];

PololuLedStrip<37> strip0;
PololuLedStrip<35> strip1;

LightStrip lightStrips[] = {
  {&strip0, 12, 50},         // Steps (1 zone)
  {&strip1, 5, 1050},        // Undercarriage (1 zone)
};

#endif

void createZoneMappings(void) {
#if SLAVE == 0

// ZoneStripMapping is struct with:
//   - (int) strip number
//   - (int) starting bulb of zone in strip
//   - (int) count of bulbs following starting bulb
// Format:
//   zoneStripMappingSets[zone number].push(ZoneStripMapping{strip, starting bulb, bulb count})
zoneStripMappingSets[0].push(ZoneStripMapping{0, 0, 16});     // Crane ring
zoneStripMappingSets[1].push(ZoneStripMapping{0, 19, 26});    // Crane top (16 - 19 black)
zoneStripMappingSets[2].push(ZoneStripMapping{0, 45, 29});    // Crane middle
zoneStripMappingSets[3].push(ZoneStripMapping{0, 74, 29});    // Crane bottom
zoneStripMappingSets[4].push(ZoneStripMapping{1, 0, 100});    // Lounge
zoneStripMappingSets[5].push(ZoneStripMapping{2, 0, 130});    // Bar ceiling (130 - 169 black)
zoneStripMappingSets[5].push(ZoneStripMapping{2, 170, 130});  // Bar ceiling
zoneStripMappingSets[6].push(ZoneStripMapping{3, 0, 130});    // Bar surface (130 - 169 black)
zoneStripMappingSets[6].push(ZoneStripMapping{3, 170, 130});  // Bar surface ...
zoneStripMappingSets[6].push(ZoneStripMapping{4, 0, 19});     //   ... and DJ booth

#elif SLAVE == 1

zoneStripMappingSets[0].push(ZoneStripMapping{0, 0, 35});     // Steps
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
  delay(1000);
//  unsigned char values[] = {255, 200, 50};
//  fakeParse(values);
//  unsigned char values1[] = {128, 125, 125};
//  fakeParse(values1);
}

//void fakeParse(unsigned char *values) {
//  writeBuffer(0, values[0], values[1], values[2]);
//}

void parseLightValues(int packetSize) {
  for (int zoneIndex = 0; zoneIndex < zoneCount; zoneIndex++) {
    writeBuffer(zoneIndex, Wire.read(), Wire.read(), Wire.read());
    packetSize -= 3;
  }
  while (packetSize--) { Wire.read(); }
  writeStrips();
}

void writeBuffer(int zoneIndex, uint8_t red, uint8_t green, uint8_t blue) {
  ZoneStripMappingSet mappingSet = zoneStripMappingSets[zoneIndex];
  for (int mapIndex = 0; mapIndex < mappingSet.size(); mapIndex++) {
    ZoneStripMapping mapping = mappingSet[mapIndex];
//    Serial.print("\nTakes ");
//    unsigned long t0 = millis();
    RGBColor color = RGBColor(red, green, blue, lightStrips[mapping.strip].info);
//    Serial.print(millis() - t0);
//    Serial.println(" ms to create:");
//    Serial.print("   ");
//    Serial.print(color.red);
//    Serial.print(", ");
//    Serial.print(color.green);
//    Serial.print(", ");
//    Serial.println(color.blue);
    for (int address = mapping.start; address < mapping.start + mapping.count; address++) {
      tempColors[mapping.strip][address] = color;
    }
  }
}

void writeStrips() {
  for (int stripInd = 0; stripInd < stripCount; stripInd++) {
    LightStrip strip = lightStrips[stripInd];
    strip.pololuStrip->write(tempColors[stripInd], strip.info.count);
  }
}

