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
  if (index < samplesize) {
    samples[index] = micros() - microseconds;
    microseconds = samples[index] + microseconds;
    index += 1;
  }
}

void setup() {
  pinMode(knobPin, INPUT_PULLUP);
  pinMode(10, OUTPUT);
  readSettings();
  randomSeed(analogRead(0));

#ifdef DEBUG
  cli();
  Serial.begin(9600);
#if defined(ARDUINO_AVR_MEGA2560)
  Serial1.begin(9600);
#endif
  Serial.println("MIDI_Sprout_V027+");
  sei();
#else
  Serial.begin(31250);
#if defined(ARDUINO_AVR_MEGA2560)
  Serial1.begin(11520);
#endif
#endif

  MIDI.begin();
  controlMessage.value = 0;
  if (noteLEDs) bootLightshow();
  attachInterrupt(interruptPin, sample, RISING);
}

void loop() {
  currentMillis = millis();

  if (index >= samplesize) {
    analyzeSample();
  }

  checkNote();
  checkControl();
  checkLED();

  // --- Only process JSON commands via web_serial_config ---
  processWebSerialConfig();

}
