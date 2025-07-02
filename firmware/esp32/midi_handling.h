#ifndef MIDI_HANDLING_H
#define MIDI_HANDLING_H

#if !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(ARDUINO_ESP32S2_DEV)
#define BLE_MIDI_SUPPORTED 1
#else
#define BLE_MIDI_SUPPORTED 0
#endif

#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include "globals.h"
#include "led_control.h"

#if BLE_MIDI_SUPPORTED
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// BLE MIDI characteristic and connection state
extern BLECharacteristic *pCharacteristic;
extern bool deviceConnected;
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

void triggerChordFreeDrone();

// Preset switching handlers
void setupMidiPresetHandlers();
void trySwitchPresetByCC(uint8_t ccNum);
void trySwitchPresetByPC(uint8_t pcNum);
void trySwitchPresetByNote(uint8_t noteNum);

#if BLE_MIDI_SUPPORTED
void bleMidiInit();
void bleMidiLoop();
#endif

#endif // MIDI_HANDLING_H
