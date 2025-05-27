#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <LEDFader.h>
#include <MIDI.h>
#include <EEPROM.h>

// Constants
#define SCHEMA 0x02
#define BUTTON_DEBOUNCE 100
#define LED_NUM 6

extern MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> MIDI;

// Scale constants
const int scaleCount = 4;
const int scaleLen = 13; // maximum scale length plus 1 for 'used length'

// Scale arrays
extern int scale[scaleCount][scaleLen];

// Global variables
extern uint16_t currScale;
extern uint16_t maxBrightness;
extern uint16_t channel;
extern float threshold;
extern float threshMin;
extern float threshMax;
extern int root;

extern const byte interruptPin;
extern const byte knobPin;
extern const byte buttonPin;
extern int menus;
extern int mode;
extern int currMenu;
extern int pulseRate;

extern const byte samplesize;
extern const byte analysize;

extern const byte polyphony;

extern int noteMin;
extern int noteMax;
extern byte QY8;
extern byte controlNumber;
extern byte controlVoltage;
extern long batteryLimit;
extern byte checkBat;

extern byte timeout;
extern int value;
extern int prevValue;

extern volatile unsigned long microseconds;
extern volatile byte index;
extern volatile unsigned long samples[];

extern float knobMin;
extern float knobMax;

extern unsigned long buttonPressed;

extern unsigned long previousMillis;
extern unsigned long previousButtonMillis;
extern unsigned long currentMillis;

extern unsigned long batteryCheck;
extern unsigned long menuTimeout;

extern LEDFader leds[LED_NUM];
extern int ledNums[LED_NUM];
extern byte controlLED;
extern byte noteLEDs;

typedef struct _MIDImessage {
  unsigned int type;
  int value;
  int velocity;
  long duration;
  long period;
  int channel;
} MIDImessage;

extern MIDImessage noteArray[];
extern int noteIndex;
extern MIDImessage controlMessage;

#endif // GLOBALS_H
