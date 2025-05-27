
#include "menu_handling.h"
#include "led_control.h"
#include "midi_handling.h"
#include "eeprom_settings.h"
#include "sensor_processing.h"
#include "utils.h"

void thresholdMode() {
  int runMode = 1;
  noteLEDs = 2;
  float prevThreshold = 0;
  while (runMode) {
    threshold = analogRead(knobPin);
    threshold = mapfloat(threshold, knobMin, knobMax, threshMin, threshMax);
    pulse(value, maxBrightness, (pulseRate / 2));
    if (threshold != prevThreshold) {
      Serial.print("Threshold is :");
      Serial.println(threshold);
      prevThreshold = threshold;
    }
    checkLED();
    if (index >= samplesize) {
      analyzeSample();
    }
    checkNote();
    checkControl();
    runMode = checkButtonToExitMenu();
    currentMillis = millis();
  }
  Serial.print("Threshold set to :");
  Serial.println(threshold);
  EEPROM_writeAnything(7, threshold);
  currMenu = 0;
  noteLEDs = 1;
  leds[prevValue].stop_fade();
  leds[prevValue].set_value(0);
}

void scaleMode() {
  int runMode = 1;
  int prevScale = 0;
  while (runMode) {
    currScale = analogRead(knobPin);
    currScale = map(currScale, knobMin, knobMax, 0, scaleCount);
    pulse(value, maxBrightness, (pulseRate / 2));
    pulse(currScale, maxBrightness, (pulseRate / 4));
    if (currScale != prevScale) {
      leds[prevScale].stop_fade();
      leds[prevScale].set_value(0);
      Serial.print("Scale is :");
      Serial.println(currScale);
    }
    prevScale = currScale;
    checkLED();
    if (index >= samplesize) {
      analyzeSample();
    }
    checkNote();
    checkControl();
    runMode = checkButtonToExitMenu();
  }
  Serial.print("Scale set to :");
  Serial.println(currScale);
  EEPROM_writeAnything(1, currScale);
  noteLEDs = 1;
  leds[prevValue].stop_fade();
  leds[prevValue].set_value(0);
  leds[currScale].stop_fade();
  leds[currScale].set_value(0);
}

void displayChannel(int channel);

void channelMode() {
  int runMode = 1;
  int prevChannel = channel;
  while (runMode) {
    channel = analogRead(knobPin);
    if (prevChannel != channel) {
      channel = map(channel, knobMin, knobMax, 1, 17);
      displayChannel(channel);
      Serial.print("Channel is :");
      Serial.println(channel);
    }
    if (index >= samplesize) {
      analyzeSample();
    }
    checkNote();
    checkControl();
    runMode = checkButtonToExitMenu();
  }
  EEPROM_writeAnything(5, channel);
  Serial.print("Channel set to :");
  Serial.println(channel);
  currMenu = 0;
  noteLEDs = 1;
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledNums[i], LOW);
  }
}

void brightnessMode() {
  int runMode = 1;
  while (runMode) {
    maxBrightness = analogRead(knobPin);
    maxBrightness = map(maxBrightness, knobMin, knobMax, 1, 255);
    Serial.print("Brightness is: ");
    Serial.println(maxBrightness);
    if (maxBrightness > 1)
      pulse(value, maxBrightness, (pulseRate / 2));
    else
      pulse(value, 1, (pulseRate / 6));
    checkLED();
    if (index >= samplesize) {
      analyzeSample();
    }
    checkNote();
    checkControl();
    runMode = checkButtonToExitMenu();
    currentMillis = millis();
  }
  EEPROM_writeAnything(3, maxBrightness);
  Serial.print("maxBrightness set to :");
  Serial.println(maxBrightness);
  currMenu = 0;
  if (maxBrightness > 1)
    noteLEDs = 1;
  leds[prevValue].stop_fade();
  leds[prevValue].set_value(0);
}

int checkButtonToExitMenu() {
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == '0' || input == 'q' || input == 'Q') {
      return 0;
    }
  }
  return 1;
}

void checkButton() {
  if (digitalRead(buttonPin) == LOW) {
    delay(BUTTON_DEBOUNCE);
    if (digitalRead(buttonPin) == LOW) {
      noteLEDs = 0;
      for (byte j = 0; j < LED_NUM; j++) {
        leds[j].stop_fade();
        leds[j].set_value(0);
      }
      delay(250);
      switch (currMenu) {
        case 0:
          prevValue = 0;
          currMenu = 1;
          previousMillis = currentMillis;
          break;
        case 1:
          switch (value) {
            case 0:
              delay(250);
              thresholdMode();
              return;
            case 1:
              delay(250);
              scaleMode();
              return;
            case 2:
              delay(250);
              channelMode();
              return;
            case 3:
              brightnessMode();
              return;
            default:
              break;
          }
          break;
        default:
          break;
      }
    }
  }
}

void checkMenu() {
  value = analogRead(knobPin);
  value = map(value, knobMin, knobMax, 0, menus);
  if (value != prevValue) {
    leds[prevValue].stop_fade();
    leds[prevValue].set_value(0);
    prevValue = value;
    previousMillis = currentMillis;
  }
  switch (currMenu) {
    case 0:
      break;
    case 1:
      noteLEDs = 0;
      pulse(value, maxBrightness, pulseRate);
      break;
    case 2:
      noteLEDs = 0;
      pulse(value, maxBrightness, (pulseRate / 2));
      break;
    default:
      break;
  }
  if ((currentMillis - previousMillis) > menuTimeout) {
    currMenu = 0;
    if (maxBrightness > 1)
      noteLEDs = 1;
    leds[prevValue].stop_fade();
    leds[prevValue].set_value(0);
  }
}
