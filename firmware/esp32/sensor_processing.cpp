#include "sensor_processing.h"
#include "led_control.h"
#include "utils.h"
#include "midi_handling.h"
#include "scale_functions.h"
#include <math.h>

// long readVcc() {
//   long result;
//   ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
//   delay(2);
//   ADCSRA |= _BV(ADSC);
//   while (bit_is_set(ADCSRA, ADSC));
//   result = ADCL;
//   result |= ADCH << 8;
//   result = 1126400L / result;
//   return result;
// }

void analyzeSample() {
  unsigned long averg = 0;
  unsigned long maxim = 0;
  unsigned long minim = 100000;
  float stdevi = 0;
  unsigned long delta = 0;
  byte change = 0;

  if (sampleIndex == samplesize) {
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

    if (delta > (stdevi * throld)) {
      change = 1;
    }

    if (change) {
      int dur = 150 + (map(delta % 127, 1, 127, 100, 2500));
      int ramp = 3 + (dur % 100);
      int notechannel = random(1, 5);

      // Velocity based on delta, mapped to velocityMin/velocityMax
      uint8_t velocity = map(delta, 0, 1023, velocityMin, velocityMax);
      if (velocity < velocityMin) velocity = velocityMin;
      if (velocity > velocityMax) velocity = velocityMax;

      static uint8_t previousNotes[2] = {0xFF, 0xFF};
      uint8_t setnote = map(averg % 127, 1, 127, noteMin, noteMax);
      setnote = scaleNote_fast(setnote, root, currScale, previousNotes);
      previousNotes[1] = previousNotes[0];
      previousNotes[0] = setnote;

      bool noteSent = false;
      if (QY8) {
        setNote(setnote, velocity, dur, notechannel,true);
        noteSent = true;
      } else {
        setNote(setnote, velocity, dur, channel,true);
        noteSent = true;
      }

      if (noteSent && ccEnable) {
        int ccValue = map(delta, 0, 1023, 0, 127);
        setControl(controlNumber, ccValue, ccValue, dur);
      }
    }

    sampleIndex = 0;
  }
}
