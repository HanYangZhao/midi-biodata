#ifndef EEPROM_SETTINGS_H
#define EEPROM_SETTINGS_H

#include "globals.h"
#include <Preferences.h>

extern Preferences prefs;

void readSettings();
void saveSettings();
void initializeEEPROM();

#endif // EEPROM_SETTINGS_H
