#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <LEDFader.h>
#include <EEPROM.h>

// =====================
// Hardware Pin Defines
// =====================
#define MIDI_SERIAL1_TX_PIN 10   // MIDI TX pin
#define MIDI_SERIAL1_RX_PIN 9    // MIDI RX pin
#define INTERRUPT_PIN      7     // Interrupt pin
#define KNOB_PIN           0     // Analog knob pin (A0)
#define LED_PIN_1          1
#define LED_PIN_2          2
#define LED_PIN_3          3
#define LED_PIN_4          4
#define LED_PIN_5          5
#define LED_NUM            5

// Constants
#define SCHEMA 0x02


// Scale constants
const int scaleCount = 9;
const int scaleLen = 13; // maximum scale length plus 1 for 'used length'

// Scale arrays
extern int scale[scaleCount][scaleLen];

typedef struct {
  uint16_t channel;
  float threshold;
  int droneEnabled;
  int bpm;
  int barperch;
  uint8_t noteMin;
  uint8_t noteMax;
  uint8_t velocityMin;
  uint8_t velocityMax;
  uint8_t ccMessagingEnabled;
  uint16_t maxBrightness;
  uint8_t bleEnabled;
  uint8_t ccEnable;
  uint8_t droneVel; // velocity for drone notes
} GlobalSettings;

typedef struct {
  uint8_t scale;           // Index of the scale to use for this preset (0 = Chromatic, 1 = Ionian, etc.)
  uint8_t rootNote;        // MIDI note number for the root note of the scale (0 = C, 1 = C#, ..., 11 = B)
  uint8_t midiCCTrigger;   // MIDI Control Change (CC) number that triggers this preset (0-127)
  uint8_t midiPCTrigger;   // MIDI Program Change (PC) number that triggers this preset (0-127)
  uint8_t droneChordQ; // 0 = triad, 1 = seventh chord
} PresetSettings;

extern GlobalSettings globalSettings;
extern PresetSettings presets[8];
extern int activePreset; // Index of the currently active preset (0-7)

// Legacy and hardware variables
extern const byte interruptPin;
extern const byte knobPin;
extern uint16_t pulseRate;

extern const byte samplesize;
extern const byte analysize;

extern const byte polyphony;

extern byte QY8;
extern byte controlNumber;
extern byte controlVoltage;

extern int value;

extern volatile unsigned long microseconds;
extern volatile byte sampleIndex;
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

// Only declare, do not define here!
void printGlobals();

#endif // GLOBALS_H
