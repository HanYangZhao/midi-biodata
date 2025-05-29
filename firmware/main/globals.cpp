#include "globals.h"
#include <MIDI.h>

// MIDI instance
MIDI_CREATE_DEFAULT_INSTANCE();

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
  {7, 0, 1, 4, 5, 7, 8, 11}                   // Example: Indian (Raga Bilawal, or user can specify)
};

// Global variables
uint16_t currScale = 0;
uint16_t maxBrightness = 190;
uint16_t channel = 1;
float throld = 2.3;
float thrMin = 1.61;
float thrMax = 3.71;
uint8_t root = 0;

const byte interruptPin = INT0;
const byte knobPin = A0;
uint16_t pulseRate = 350;

const byte samplesize = 10;
const byte analysize = samplesize - 1;

const byte polyphony = 5;

uint8_t noteMin = 36;
uint8_t noteMax = 96;
byte QY8 = 0;
byte controlNumber = 80;
byte controlVoltage = 1;

int value = 0;

volatile unsigned long microseconds = 0;
volatile byte index = 0;
volatile unsigned long samples[samplesize] = {0};


unsigned long previousMillis = 0;
unsigned long previousButtonMillis = 0;
unsigned long currentMillis = 1;


unsigned long buttonPressed = 0;

LEDFader leds[LED_NUM] = {
  LEDFader(3),
  LEDFader(5),
  LEDFader(6),
  LEDFader(9),
  LEDFader(10),
  LEDFader(11)
};

int ledNums[LED_NUM] = {3, 5, 6, 9, 10, 11};
byte controlLED = 5;
byte noteLEDs = 1;

MIDImessage noteArray[polyphony] = {};
int noteIndex = 0;
MIDImessage controlMessage = {};

// Chord timing globals
int bpm = 120;
int barperch = 1;
int droneEnabled = 0;
