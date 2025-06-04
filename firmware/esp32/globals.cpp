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
};

// Global variables
uint16_t currScale = 0;
uint16_t maxBrightness = 190;
uint16_t channel = 1;
float throld = 2.3;
float thrMin = 1.61;
float thrMax = 3.71;
uint8_t root = 0;

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
  Serial.print("currScale: "); Serial.println(currScale);
  Serial.print("maxBrightness: "); Serial.println(maxBrightness);
  Serial.print("channel: "); Serial.println(channel);
  Serial.print("throld: "); Serial.println(throld);
  Serial.print("root: "); Serial.println(root);
  Serial.print("droneEnabled: "); Serial.println(droneEnabled);
  Serial.print("bpm: "); Serial.println(bpm);
  Serial.print("barperch: "); Serial.println(barperch);
  Serial.print("noteMin: "); Serial.println(noteMin);
  Serial.print("noteMax: "); Serial.println(noteMax);
  Serial.print("velocityMin: "); Serial.println(velocityMin);
  Serial.print("velocityMax: "); Serial.println(velocityMax);
  Serial.print("ccEnable: "); Serial.println(ccEnable);
  Serial.print("bleEnabled: "); Serial.println(bleEnabled);
  Serial.println("-----------------------------");
}
