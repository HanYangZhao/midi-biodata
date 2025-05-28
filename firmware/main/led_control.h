#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include "globals.h"

void rampUp(int ledPin, int value, int time);
void rampDown(int ledPin, int value, int time);
void checkLED();
void pulse(int ledPin, int maxValue, int time);
void bootLightshow();
void displayChannel(int channel);

void blinkLED(int num, int milis);

#endif // LED_CONTROL_H
