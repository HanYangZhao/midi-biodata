#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <LEDFader.h>
#include <MIDI.h>
#include <EEPROM.h>

// Constants
#define SCHEMA 0x02
#define LED_NUM 6

extern MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> MIDI;

// Scale constants
const int scaleCount = 10;
const int scaleLen = 13; // maximum scale length plus 1 for 'used length'

// Scale arrays
extern int scale[scaleCount][scaleLen];

// Global variables
extern uint16_t currScale;
extern uint16_t maxBrightness;
extern uint16_t channel;
extern float throld;
extern float thrMin;
extern float thrMax;
extern uint8_t root;

extern const byte interruptPin;
extern const byte knobPin;
extern uint16_t pulseRate;

extern const byte samplesize;
extern const byte analysize;

extern const byte polyphony;

extern uint8_t noteMin;
extern uint8_t noteMax;
extern byte QY8;
extern byte controlNumber;
extern byte controlVoltage;

extern int value;

extern volatile unsigned long microseconds;
extern volatile byte index;
extern volatile unsigned long samples[];

extern unsigned long buttonPressed;

extern unsigned long previousMillis;
extern unsigned long previousButtonMillis;
extern unsigned long currentMillis;


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

// Chord timing globals
extern int bpm;
extern int barperch;

// Drone enable global
extern int droneEnabled;

#endif // GLOBALS_H
