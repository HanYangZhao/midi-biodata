#include "eeprom_settings.h"
#include "led_control.h"
#include <Preferences.h>
#include "globals.h"

Preferences prefs;

void readSettings() {
  prefs.begin("midi-bio", true); // read-only

  globalSettings.channel = prefs.getUShort("channel", 1);
  globalSettings.threshold = prefs.getFloat("threshold", 2.3f);
  globalSettings.droneEnabled = prefs.getUShort("droneEnabled", 0);
  globalSettings.bpm = prefs.getUShort("bpm", 120);
  globalSettings.barperch = prefs.getUShort("barperch", 1);
  globalSettings.noteMin = prefs.getUChar("noteMin", 21);
  globalSettings.noteMax = prefs.getUChar("noteMax", 108);
  globalSettings.velocityMin = prefs.getUChar("velocityMin", 30);
  globalSettings.velocityMax = prefs.getUChar("velocityMax", 127);
  globalSettings.ccMessagingEnabled = prefs.getUChar("ccMessagingEnabled", 0);
  globalSettings.maxBrightness = prefs.getUShort("maxBrightness", 190);
  globalSettings.bleEnabled = prefs.getUChar("bleEnabled", 0);
  globalSettings.ccEnable = prefs.getUChar("ccEnable", 0);
  globalSettings.droneVel = prefs.getUChar("droneVel", 40); // default 40
  globalSettings.droneMode = prefs.getUChar("droneMode", 0); // default freeDrone
  globalSettings.droneChannel = prefs.getUChar("droneChannel", 1); // default channel 1

  // Load presets
  for (int i = 0; i < 8; ++i) {
    char key[32];
    snprintf(key, sizeof(key), "preset%d_scale", i);
    presets[i].scale = prefs.getUChar(key, i);
    snprintf(key, sizeof(key), "preset%d_root", i);
    presets[i].rootNote = prefs.getUChar(key, 0);
    snprintf(key, sizeof(key), "preset%d_cc", i);
    presets[i].midiCCTrigger = prefs.getChar(key, -1);
    snprintf(key, sizeof(key), "preset%d_pc", i);
    presets[i].midiPCTrigger = prefs.getChar(key, -1);
    snprintf(key, sizeof(key), "preset%d_note", i);
    presets[i].midiNoteTrigger = prefs.getChar(key, -1);
    snprintf(key, sizeof(key), "preset%d_quality", i);
    presets[i].droneChordQ = prefs.getUChar(key, 0);
  }

  prefs.end();

  printGlobals();
}

void saveSettings() {
  printGlobals();
  prefs.begin("midi-bio", false); // read-write

  prefs.putUShort("channel", globalSettings.channel);
  prefs.putFloat("threshold", globalSettings.threshold);
  prefs.putUShort("droneEnabled", globalSettings.droneEnabled);
  prefs.putUShort("bpm", globalSettings.bpm);
  prefs.putUShort("barperch", globalSettings.barperch);
  prefs.putUChar("noteMin", globalSettings.noteMin);
  prefs.putUChar("noteMax", globalSettings.noteMax);
  prefs.putUChar("velocityMin", globalSettings.velocityMin);
  prefs.putUChar("velocityMax", globalSettings.velocityMax);
  prefs.putUChar("ccMessagingEnabled", globalSettings.ccMessagingEnabled);
  prefs.putUShort("maxBrightness", globalSettings.maxBrightness);
  prefs.putUChar("bleEnabled", globalSettings.bleEnabled);
  prefs.putUChar("ccEnable", globalSettings.ccEnable);
  prefs.putUChar("droneVel", globalSettings.droneVel);
  prefs.putUChar("droneMode", globalSettings.droneMode);
  prefs.putUChar("droneChannel", globalSettings.droneChannel);

  // Save presets
  for (int i = 0; i < 8; ++i) {
    char key[32];
    snprintf(key, sizeof(key), "preset%d_scale", i);
    prefs.putUChar(key, presets[i].scale);
    snprintf(key, sizeof(key), "preset%d_root", i);
    prefs.putUChar(key, presets[i].rootNote);
    snprintf(key, sizeof(key), "preset%d_cc", i);
    prefs.putChar(key, presets[i].midiCCTrigger);
    snprintf(key, sizeof(key), "preset%d_pc", i);
    prefs.putChar(key, presets[i].midiPCTrigger);
    snprintf(key, sizeof(key), "preset%d_note", i);
    prefs.putChar(key, presets[i].midiNoteTrigger);
    snprintf(key, sizeof(key), "preset%d_quality", i);
    prefs.putUChar(key, presets[i].droneChordQ);
  }

  prefs.end();
}

void initializeEEPROM() {
  // Factory reset: clear all stored values
  prefs.begin("midi-bio", false);
  prefs.clear();
  prefs.end();
  // Set all values to defaults and save
  globalSettings.channel = 1;
  globalSettings.threshold = 2.3f;
  globalSettings.droneEnabled = 0;
  globalSettings.bpm = 120;
  globalSettings.barperch = 1;
  globalSettings.noteMin = 21;
  globalSettings.noteMax = 108;
  globalSettings.velocityMin = 30;
  globalSettings.velocityMax = 127;
  globalSettings.ccMessagingEnabled = 0;
  globalSettings.maxBrightness = 190;
  globalSettings.bleEnabled = 0;
  globalSettings.ccEnable = 0;
  globalSettings.droneVel = 40;
  globalSettings.droneMode = 0;

  for (int i = 0; i < 8; ++i) {
    presets[i].scale = i;
    presets[i].rootNote = 0;
    presets[i].midiCCTrigger = -1;
    presets[i].midiPCTrigger = -1;
    presets[i].midiNoteTrigger = -1;
    presets[i].droneChordQ = 0;
  }
  saveSettings();
}
