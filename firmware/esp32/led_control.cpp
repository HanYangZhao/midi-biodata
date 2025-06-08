#include "led_control.h"

void rampUp(int ledPin, int value, int time) {
  LEDFader *led = &leds[ledPin];
  led->fade(map(value, 0, 255, 0, globalSettings.maxBrightness), time);
}

void rampDown(int ledPin, int value, int time) {
  LEDFader *led = &leds[ledPin];
  led->fade(value, time);
}

void checkLED() {
  for (byte i = 0; i < LED_NUM; i++) {
    LEDFader *led = &leds[i];
    led->update();
  }
}

void pulse(int ledPin, int maxValue, int time) {
  LEDFader *led = &leds[ledPin];
  if (!led->is_fading()) {
    if (led->get_value() > 0) {
      led->fade(0, time);
    } else {
      led->fade(maxValue, time);
    }
  }
}

void bootLightshow() {
  for (byte i = 5; i > 0; i--) {
    LEDFader *led = &leds[i - 1];
    led->fade(200, 150);
    while (led->is_fading()) checkLED();
    led->fade(0, 150 + i * 17);
    while (led->is_fading()) checkLED();
  }
}

void displayChannel(int channel) {
  for (byte i = 0; i < 5; i++) {
    digitalWrite(ledNums[i], channel & 1);
    channel >>= 1;
  }
}

void blinkLED(int num, int milis) {
  for (int i = 0; i < num; i++) {
    digitalWrite(LED_PIN_1, HIGH);
    delay(milis);
    digitalWrite(LED_PIN_1, LOW);
    if (i != num - 1) {
      delay(milis);
    }
  }
}
