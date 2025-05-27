#ifndef EEPROM_SETTINGS_H
#define EEPROM_SETTINGS_H

#include "globals.h"

void readSettings();
void initializeEEPROM();

template <class T>
int EEPROM_writeAnything(int ee, const T& value);

template <class T>
int EEPROM_readAnything(int ee, T& value);

// Explicit instantiations for linker
extern template int EEPROM_writeAnything<int>(int, const int&);
extern template int EEPROM_writeAnything<unsigned int>(int, const unsigned int&);
extern template int EEPROM_writeAnything<float>(int, const float&);
extern template int EEPROM_readAnything<int>(int, int&);
extern template int EEPROM_readAnything<unsigned int>(int, unsigned int&);
extern template int EEPROM_readAnything<float>(int, float&);

#endif // EEPROM_SETTINGS_H
