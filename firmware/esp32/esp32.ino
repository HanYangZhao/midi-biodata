#include <Arduino.h>
#include <ArduinoJson.h>
#include "globals.h"
#include "eeprom_settings.h"
#include "led_control.h"
#include "midi_handling.h"
#include "sensor_processing.h"
#include "web_serial_config.h"

// Interrupt timing sample array
void sample() {
  if (sampleIndex < samplesize) {
    samples[sampleIndex] = micros() - microseconds;
    microseconds = samples[sampleIndex] + microseconds;
    sampleIndex += 1;
  }
}

void setup() {
  pinMode(KNOB_PIN, INPUT_PULLUP);
  pinMode(LED_PIN_5, OUTPUT); // Example: update as needed for your hardware
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);
  pinMode(LED_PIN_4, OUTPUT);




  randomSeed(analogRead(0));

  Serial.begin(31250);
  Serial1.begin(31250, SERIAL_8N1, MIDI_SERIAL1_RX_PIN, MIDI_SERIAL1_TX_PIN);

  // Factory reset: uncomment the next line, upload, then comment again and re-upload
  // initializeEEPROM();

  readSettings();
  // USB MIDI initialization (from Adafruit_TinyUSB example)
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }
  usb_midi.setStringDescriptor("TinyUSB MIDI");
  // This will also call usb_midi's begin()
  usbMIDI.begin(MIDI_CHANNEL_OMNI);

  bleMidiInit();

  printGlobals();
  MIDI.begin();
  controlMessage.value = 0;
  if (noteLEDs) bootLightshow();
  attachInterrupt(interruptPin, sample, RISING);
}

void loop() {
  currentMillis = millis();

  bleMidiLoop();

  if (sampleIndex >= samplesize) {
    analyzeSample();
  }

  checkNote();

  // Serial.println("[CHECKPOINT] checkControl about to run");
  checkControl();
  // Serial.println("[CHECKPOINT] checkControl finished");

  checkLED();

  midiChordTick();

  // --- Only process JSON commands via web_serial_config ---
  processWebSerialConfig();
}
