#include "sensor_processing.h"
#include "led_control.h"
#include "utils.h"
#include "midi_handling.h"
#include "scale_functions.h"
#include <math.h>

long readVcc() {
  long result;
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1126400L / result;
  return result;
}

void checkBattery() {
  if (batteryCheck < currentMillis) {
    batteryCheck = currentMillis + 10000;
    if (readVcc() < batteryLimit) {
      if (checkBat) {
        for (byte j = 0; j < LED_NUM; j++) {
          leds[j].stop_fade();
          leds[j].set_value(0);
        }
        noteLEDs = 0;
        checkBat = 0;
      }
    }
  }
}

void analyzeSample() {
  unsigned long averg = 0;
  unsigned long maxim = 0;
  unsigned long minim = 100000;
  float stdevi = 0;
  unsigned long delta = 0;
  byte change = 0;

  if (index == samplesize) {
    unsigned long sampanalysis[analysize];
    for (byte i = 0; i < analysize; i++) {
      sampanalysis[i] = samples[i + 1];
      if (sampanalysis[i] > maxim) {
        maxim = sampanalysis[i];
      }
      if (sampanalysis[i] < minim) {
        minim = sampanalysis[i];
      }
      averg += sampanalysis[i];
      stdevi += sampanalysis[i] * sampanalysis[i];
    }

    averg = averg / analysize;
    stdevi = sqrt(stdevi / analysize - averg * averg);
    if (stdevi < 1) {
      stdevi = 1.0;
    }

    delta = maxim - minim;

    if (delta > (stdevi * threshold)) {
      change = 1;
    }

    if (change) {
      int dur = 150 + (map(delta % 127, 1, 127, 100, 2500));
      int ramp = 3 + (dur % 100);
      int notechannel = random(1, 5);

      int setnote = map(averg % 127, 1, 127, noteMin, noteMax);
      setnote = scaleNote_fast(setnote, root);

      if (QY8) {
        setNote(setnote, 100, dur, notechannel);
      } else {
        setNote(setnote, 100, dur, channel);
      }

      setControl(controlNumber, controlMessage.value, delta % 127, ramp);
    }

    index = 0;
  }
}
