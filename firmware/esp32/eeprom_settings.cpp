#include "eeprom_settings.h"
#include "led_control.h"
#include <Preferences.h>
#include "globals.h"

Preferences prefs;

void readSettings() {
  prefs.begin("midi-bio", true); // read-only

  currScale = prefs.getUShort("currScale", 1);
  maxBrightness = prefs.getUShort("maxBrightness", 255);
  channel = prefs.getUShort("channel", 1);
  throld = prefs.getFloat("throld", 2.3f);
  root = prefs.getUShort("root", 60);
  droneEnabled = prefs.getUShort("droneEnabled", 0);
  bpm = prefs.getUShort("bpm", 120);
  barperch = prefs.getUShort("barperch", 1);
  noteMin = prefs.getUChar("noteMin", 21);
  noteMax = prefs.getUChar("noteMax", 108);
  velocityMin = prefs.getUChar("velocityMin", 30);
  velocityMax = prefs.getUChar("velocityMax", 127);

  prefs.end();

  printGlobals();
}

void saveSettings() {
  prefs.begin("midi-bio", false); // read-write

  prefs.putUShort("currScale", currScale);
  prefs.putUShort("maxBrightness", maxBrightness);
  prefs.putUShort("channel", channel);
  prefs.putFloat("throld", throld);
  prefs.putUShort("root", root);
  prefs.putUShort("droneEnabled", droneEnabled);
  prefs.putUShort("bpm", bpm);
  prefs.putUShort("barperch", barperch);
  prefs.putUChar("noteMin", noteMin);
  prefs.putUChar("noteMax", noteMax);
  prefs.putUChar("velocityMin", velocityMin);
  prefs.putUChar("velocityMax", velocityMax);

  prefs.end();
}

void initializeEEPROM() {
  // Set all values to defaults and save
  currScale = 1;
  maxBrightness = 255;
  channel = 1;
  throld = 2.3f;
  root = 60;
  droneEnabled = 0;
  bpm = 120;
  barperch = 1;
  noteMin = 21;
  noteMax = 108;
  velocityMin = 30;
  velocityMax = 127;
  saveSettings();
}
