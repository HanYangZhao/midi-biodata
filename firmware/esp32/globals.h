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
extern uint8_t velocityMin;
extern uint8_t velocityMax;
extern byte QY8;
extern byte controlNumber;
extern byte controlVoltage;

extern uint8_t ccEnable;
extern uint8_t bleEnabled;

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
