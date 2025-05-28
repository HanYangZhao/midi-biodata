#include "eeprom_settings.h"
#include "led_control.h"
#include <EEPROM.h>

void readSettings() {
  // check if eeprom is empty or not
  int schema = 0;
  EEPROM.get(0, schema);
  if (schema != SCHEMA) {
    initializeEEPROM();
    blinkLED(3, 500);
  }

  // read!
  EEPROM.get(2, currScale);
  EEPROM.get(4, maxBrightness);
  EEPROM.get(6, channel);
  EEPROM.get(8, threshold);
  EEPROM.get(12, root);
}

void initializeEEPROM() {
  EEPROM.put(0, SCHEMA);          
  EEPROM.put(2, 0);              // currScale default value (int, 2 bytes)
  EEPROM.put(4, 255);            // maxBrightness default value (int, 2 bytes)
  EEPROM.put(6, 1);              // MIDI channel default value (int, 2 bytes)
  EEPROM.put(8, 2.3f);           // threshold default value (float, 4 bytes)
  EEPROM.put(12, 60);            // root default value (int, 2 bytes, e.g. MIDI note 60 = C4)
}
