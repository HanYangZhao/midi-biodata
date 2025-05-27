#include <Arduino.h>
#include "globals.h"
#include "eeprom_settings.h"
#include "led_control.h"
#include "midi_handling.h"
#include "menu_handling.h"
#include "sensor_processing.h"

// Interrupt timing sample array
void sample() {
  if (index < samplesize) {
    samples[index] = micros() - microseconds;
    microseconds = samples[index] + microseconds;
    index += 1;
  }
}

ISR(PCINT0_vect) {
  if (digitalRead(buttonPin) == LOW) {
    buttonPressed = millis();
  }
}

void pciSetup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit(digitalPinToPCMSKbit(pin));
  PCIFR |= bit(digitalPinToPCICRbit(pin));
  PCICR |= bit(digitalPinToPCICRbit(pin));
}

void setup() {
  pinMode(knobPin, INPUT);
  pinMode(buttonPin, INPUT);
  pinMode(10, OUTPUT);
  digitalWrite(buttonPin, HIGH);
  readSettings();
  randomSeed(analogRead(0));

#ifdef DEBUG
  cli();
  Serial.begin(9600);
  Serial.println("MIDI_Sprout_V027+");
  sei();
#else
  Serial.begin(31250);
#endif

  MIDI.begin();
  controlMessage.value = 0;
  checkBattery();
  if (noteLEDs) bootLightshow();
  attachInterrupt(interruptPin, sample, RISING);
  pciSetup(buttonPin);
}

void loop() {
  currentMillis = millis();
  checkBattery();

  if (index >= samplesize) {
    analyzeSample();
  }

  checkNote();
  checkControl();
  checkLED();

  if (Serial.available() > 0) {
    char input = Serial.read();
    switch (input) {
      case '0':
        currMenu = 0;
        noteLEDs = 1;
        Serial.println("Mode 0");
        break;
      case '1':
        currMenu = 1;
        Serial.println("Mode threshold");
        thresholdMode();
        break;
      case '2':
        currMenu = 2;
        Serial.println("Mode scale");
        scaleMode();
        break;
      case '3':
        currMenu = 3;
        Serial.println("Mode Channel");
        channelMode();
        break;
      case '4':
        currMenu = 4;
        Serial.println("Mode Brightness");
        brightnessMode();
        break;
      case '5':
        currMenu = 5;
        Serial.println("Mode Root Note");
        rootNoteMode();
        break;
      default:
        break;
    }
  }

  if ((currentMillis - buttonPressed) < 100) {
    checkButton();
  }

  if (currMenu > 0) {
    checkMenu();
  }
}
