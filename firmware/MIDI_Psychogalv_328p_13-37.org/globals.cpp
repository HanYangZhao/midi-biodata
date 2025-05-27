#include "globals.h"
#include <MIDI.h>

// MIDI instance
MIDI_CREATE_DEFAULT_INSTANCE();

// Scale arrays
int scale[scaleCount][scaleLen] = {
  {12, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}, // Chromatic
  {7, 1, 3, 5, 6, 8, 10, 12},                   // Major
  {7, 1, 2, 2, 5, 6, 9, 11},                    // Indian
  {7, 1, 3, 4, 6, 8, 9, 11}                     // Minor
};

// Global variables
uint16_t currScale = 0;
uint16_t maxBrightness = 190;
uint16_t channel = 1;
float threshold = 2.3;
float threshMin = 1.61;
float threshMax = 3.71;
int root = 0;

const byte interruptPin = INT0;
const byte knobPin = A0;
const byte buttonPin = 13;
int menus = 4;
int mode = 0;
int currMenu = 0;
int pulseRate = 350;

const byte samplesize = 10;
const byte analysize = samplesize - 1;

const byte polyphony = 5;

int noteMin = 36;
int noteMax = 96;
byte QY8 = 0;
byte controlNumber = 80;
byte controlVoltage = 1;
long batteryLimit = 3000;
byte checkBat = 1;

byte timeout = 0;
int value = 0;
int prevValue = 0;

volatile unsigned long microseconds = 0;
volatile byte index = 0;
volatile unsigned long samples[samplesize] = {0};

float knobMin = 1;
float knobMax = 1024;

unsigned long previousMillis = 0;
unsigned long previousButtonMillis = 0;
unsigned long currentMillis = 1;

unsigned long batteryCheck = 5000;
unsigned long menuTimeout = 5000;

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
