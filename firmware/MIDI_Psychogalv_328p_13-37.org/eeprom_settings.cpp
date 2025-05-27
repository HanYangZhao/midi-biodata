#include "eeprom_settings.h"
#include "led_control.h"
#include <EEPROM.h>

void readSettings() {
  // check if eeprom is empty or not
  short schema;
  EEPROM_readAnything(0, schema);
  if (schema != SCHEMA) {
    initializeEEPROM();
  }

  // check if MENU button is hold down for > 1s (to reset EEPROM settings)
  if (digitalRead(buttonPin) == LOW) {
    delay(1000);
    if (digitalRead(buttonPin) == LOW) {
      initializeEEPROM();
    }
    // blink WHITE LED 3 times (500ms)
    blinkLED(3, 500);
  }

  // read!
  EEPROM_readAnything(1, currScale);
  EEPROM_readAnything(3, maxBrightness);
  EEPROM_readAnything(5, channel);
  EEPROM_readAnything(7, threshold);
}

void initializeEEPROM() {
  EEPROM_writeAnything(1, 0); // curScale default value
  EEPROM_writeAnything(3, 255); // maxBrightness default value
  EEPROM_writeAnything(5, 1); // MIDI channel default value
  EEPROM_writeAnything(7, 2.3f); // threshold default value
  EEPROM_writeAnything(0, SCHEMA);
}

template <class T>
int EEPROM_writeAnything(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

template <class T>
int EEPROM_readAnything(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

// Explicit template instantiations for linker
template int EEPROM_writeAnything<int>(int, const int&);
template int EEPROM_writeAnything<unsigned int>(int, const unsigned int&);
template int EEPROM_writeAnything<float>(int, const float&);
template int EEPROM_readAnything<int>(int, int&);
template int EEPROM_readAnything<unsigned int>(int, unsigned int&);
template int EEPROM_readAnything<float>(int, float&);
