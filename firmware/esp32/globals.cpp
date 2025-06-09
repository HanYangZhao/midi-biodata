// Firmware version
const char* FIRMWARE_VERSION = "1.0.0";

#include "globals.h"
#include <MIDI.h>


// Scale arrays
int scale[scaleCount][scaleLen] = {
  {12, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, // Chromatic (0-based)
  {7, 0, 2, 4, 5, 7, 9, 11},                  // Ionian (Major)
  {7, 0, 2, 3, 5, 7, 9, 10},                  // Dorian
  {7, 0, 1, 3, 5, 7, 8, 10},                  // Phrygian
  {7, 0, 2, 4, 6, 7, 9, 11},                  // Lydian
  {7, 0, 2, 4, 5, 7, 9, 10},                  // Mixolydian
  {7, 0, 2, 3, 5, 7, 8, 10},                  // Aeolian (Minor)
  {7, 0, 1, 3, 5, 6, 8, 10},                  // Locrian
  {7, 0, 1, 4, 5, 7, 8, 11},                  // Raga Bhairav (double harmonic major)
  {8, 0, 2, 4, 7, 9, 0, 2, 4},                // Major Pentatonic (padded for chord logic)
  {8, 0, 3, 5, 7, 10, 0, 3, 5}                // Minor Pentatonic (padded for chord logic)
};

GlobalSettings globalSettings = {
  .channel = 1,
  .threshold = 2.3,
  .droneEnabled = 0,
  .bpm = 120,
  .barperch = 1,
  .noteMin = 21,
  .noteMax = 108,
  .velocityMin = 30,
  .velocityMax = 127,
  .ccMessagingEnabled = 0,
  .maxBrightness = 190,
  .bleEnabled = 0,
  .ccEnable = 0,
  .droneVel = 40,
  .droneMode = 0,
  .droneChannel = 1,
  .inputChannel = 1
};

PresetSettings presets[8] = {
  {0, 0, -1, -1, -1, 0}, {1, 0, -1, -1, -1, 0}, {2, 0, -1, -1, -1, 0}, {3, 0, -1, -1, -1, 0},
  {4, 0, -1, -1, -1, 0}, {5, 0, -1, -1, -1, 0}, {6, 0, -1, -1, -1, 0}, {7, 0, -1, -1, -1, 0}
};

int activePreset = 0; // Default to preset 1

const byte interruptPin = INTERRUPT_PIN;
const byte knobPin = KNOB_PIN;
uint16_t pulseRate = 350;

const byte samplesize = 10;
const byte analysize = samplesize - 1;

const byte polyphony = 5;

uint8_t noteMin = 21;  // A0, lowest piano key
uint8_t noteMax = 108; // C8, highest piano key
uint8_t velocityMin = 30;
uint8_t velocityMax = 127;
byte QY8 = 0;
byte controlNumber = 80;
byte controlVoltage = 1;
uint8_t ccEnable = 0;
uint8_t bleEnabled = 0;

int value = 0;

volatile unsigned long microseconds = 0;
volatile byte sampleIndex = 0;
volatile unsigned long samples[samplesize] = {0};


unsigned long previousMillis = 0;
unsigned long previousButtonMillis = 0;
unsigned long currentMillis = 1;


unsigned long buttonPressed = 0;

LEDFader leds[LED_NUM] = {
  LEDFader(LED_PIN_1),
  LEDFader(LED_PIN_2),
  LEDFader(LED_PIN_3),
  LEDFader(LED_PIN_4),
  LEDFader(LED_PIN_5),
};

int ledNums[LED_NUM] = {LED_PIN_1, LED_PIN_2, LED_PIN_3, LED_PIN_4, LED_PIN_5};
byte controlLED = 5;
byte noteLEDs = 1;

MIDImessage noteArray[polyphony] = {};
int noteIndex = 0;
MIDImessage controlMessage = {};

// Chord timing globals
int bpm = 120;
int barperch = 1;
int droneEnabled = 0;

void printGlobals() {
  Serial.println("---- EEPROM/Globals Debug ----");
  Serial.print("channel: "); Serial.println(globalSettings.channel);
  Serial.print("threshold: "); Serial.println(globalSettings.threshold);
  Serial.print("droneEnabled: "); Serial.println(globalSettings.droneEnabled);
  Serial.print("bpm: "); Serial.println(globalSettings.bpm);
  Serial.print("barperch: "); Serial.println(globalSettings.barperch);
  Serial.print("noteMin: "); Serial.println(globalSettings.noteMin);
  Serial.print("noteMax: "); Serial.println(globalSettings.noteMax);
  Serial.print("velocityMin: "); Serial.println(globalSettings.velocityMin);
  Serial.print("velocityMax: "); Serial.println(globalSettings.velocityMax);
  Serial.print("ccMessagingEnabled: "); Serial.println(globalSettings.ccMessagingEnabled);
  Serial.print("maxBrightness: "); Serial.println(globalSettings.maxBrightness);
  Serial.print("bleEnabled: "); Serial.println(globalSettings.bleEnabled);
  Serial.print("ccEnable: "); Serial.println(globalSettings.ccEnable);
  Serial.print("droneVel: "); Serial.println(globalSettings.droneVel);
  Serial.print("droneMode: "); Serial.println(globalSettings.droneMode);
  Serial.print("inputChannel: "); Serial.println(globalSettings.inputChannel);
  for (int i = 0; i < 8; ++i) {
    Serial.print("Preset "); Serial.print(i); Serial.print(": scale=");
    Serial.print(presets[i].scale); Serial.print(", rootNote=");
    Serial.print(presets[i].rootNote); Serial.print(", midiCCTrigger=");
    Serial.print(presets[i].midiCCTrigger); Serial.print(", midiPCTrigger=");
    Serial.print(presets[i].midiPCTrigger); Serial.print(", midiNoteTrigger=");
    Serial.print(presets[i].midiNoteTrigger); Serial.print(", droneChordQuality=");
    Serial.println(presets[i].droneChordQ == 1 ? "Seventh" : "Triad");
  }
  Serial.println("-----------------------------");
}
