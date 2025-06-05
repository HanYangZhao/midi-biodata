#ifndef MIDI_HANDLING_H
#define MIDI_HANDLING_H

#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include "globals.h"
#include "led_control.h"

#if !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(ARDUINO_ESP32S2_DEV)
#include <BLEMidi.h>
#define BLE_MIDI_SUPPORTED 1
#else
#define BLE_MIDI_SUPPORTED 0
#endif

 // Expose usb_midi and usbMIDI for use in other files
extern Adafruit_USBD_MIDI usb_midi;
extern midi::MidiInterface<midi::SerialMIDI<Adafruit_USBD_MIDI>> usbMIDI;
extern MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>> MIDI;


void setNote(int value, int velocity, long duration, int notechannel, bool debug);
void setControl(int type, int value, int velocity, long duration);
void checkControl();
void checkNote();
void MIDIpanic();
void midiSerial(int type, int channel, int data1, int data2);

void midiChordTick();

// Preset switching handlers
void setupMidiPresetHandlers();
void trySwitchPresetByCC(uint8_t ccNum);
void trySwitchPresetByPC(uint8_t pcNum);

#if BLE_MIDI_SUPPORTED
void bleMidiInit();
void bleMidiLoop();
#else
inline void bleMidiInit() {}
inline void bleMidiLoop() {}
#endif

#endif // MIDI_HANDLING_H
