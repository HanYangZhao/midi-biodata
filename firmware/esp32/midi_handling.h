#ifndef MIDI_HANDLING_H
#define MIDI_HANDLING_H

#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include "globals.h"
#include "led_control.h"

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

#endif // MIDI_HANDLING_H
