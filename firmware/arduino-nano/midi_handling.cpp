#include "midi_handling.h"
#include "led_control.h"
#include "globals.h"

// Forward declaration
void triggerChord();

void debugPrintNote(int value, int velocity, int notechannel) {
#if defined(ARDUINO_AVR_MEGA2560)
  Serial1.print("Note : value=");
  Serial1.print(value);
  Serial1.print(" channel=");
  Serial1.print(notechannel);
  Serial1.print(" velocity=");
  Serial1.println(velocity);
#endif
}

void setNote(int value, int velocity, long duration, int notechannel) {
  for (int i = 0; i < polyphony; i++) {
    if (!noteArray[i].velocity) {
      noteArray[i].type = 0;
      noteArray[i].value = value;
      noteArray[i].velocity = velocity;
      noteArray[i].duration = currentMillis + duration;
      noteArray[i].channel = notechannel;

      debugPrintNote(value, velocity, notechannel);

      if (QY8) {
        MIDI.sendNoteOn(value, velocity, notechannel);
      } else {
        MIDI.sendNoteOn(value, velocity, channel);
      }

      if (noteLEDs == 1) {

        rampUp(i, maxBrightness, duration);

      } else if (noteLEDs == 2) {
        rampUp(i + 1, maxBrightness, duration);
      }
      break;
    }
  }
}

void setControl(int type, int value, int velocity, long duration) {
  controlMessage.type = type;
  controlMessage.value = value;
  controlMessage.velocity = velocity;
  controlMessage.period = duration;
  controlMessage.duration = currentMillis + duration;
}

void checkControl() {
  signed int distance = controlMessage.velocity - controlMessage.value;
  if (distance != 0) {
    if (currentMillis > controlMessage.duration) {
      controlMessage.duration = currentMillis + controlMessage.period;
      if (distance > 0) {
        controlMessage.value += 1;
      } else {
        controlMessage.value -= 1;
      }
      MIDI.sendControlChange(controlMessage.type, controlMessage.value, channel);
      if (controlVoltage) {
        if (distance > 0) {
          rampUp(controlLED, map(controlMessage.value, 0, 127, 0, 255), 5);
        } else {
          rampDown(controlLED, map(controlMessage.value, 0, 127, 0, 255), 5);
        }
      }
    }
  }
}

void checkNote() {
  for (int i = 0; i < polyphony; i++) {
    if (noteArray[i].velocity) {
      if (noteArray[i].duration <= currentMillis) {
        debugPrintNote(noteArray[i].value, 0, noteArray[i].channel);
        if (QY8) {
          MIDI.sendNoteOff(noteArray[i].value, 0, noteArray[i].channel);
        } else {
          MIDI.sendNoteOff(noteArray[i].value, 0, channel);
        }
        noteArray[i].velocity = 0;
        if (noteLEDs == 1) rampDown(i, 0, 225);
        if (noteLEDs == 2) rampDown(i + 1, 0, 225);
      }
    }
  }
}

void MIDIpanic() {
  for (byte i = 1; i < 128; i++) {
    delay(1);
    MIDI.sendNoteOff(i, 0, channel);
    if (QY8) {
      for (byte k = 1; k < 5; k++) {
        delay(1);
        MIDI.sendNoteOff(i, 0, k);
      }
    }
  }
}

void midiSerial(int type, int channel, int data1, int data2) {
  cli();
  data1 &= 0x7F;
  data2 &= 0x7F;
  byte statusbyte = (type | ((channel - 1) & 0x0F));
#ifndef DEBUG
  Serial.write(statusbyte);
  Serial.write(data1);
  Serial.write(data2);
#else
  if (type == 144) {
    Serial.println(data1);
  }
#endif
  sei();
}

// --- Chord timing and triad logic ---
static unsigned long lastChordMillis = 0;
static int barsElapsed = 0;

void midiChordTick() {
  static int prevDroneEnabled = 1;
  int triad[3] = {
    root + scale[currScale][1],
    root + scale[currScale][3],
    root + scale[currScale][5]
  };

  if (!droneEnabled) {
    if (prevDroneEnabled) {
      // Drone was just disabled, turn off notes
      for (int i = 0; i < 3; i++) {
        MIDI.sendNoteOff(triad[i], 0, channel);
      }
    }
    prevDroneEnabled = droneEnabled;
    return;
  }

  // Calculate ms per bar: (60,000 ms/min) / bpm * 4 beats/bar
  unsigned long msPerBar = (unsigned long)(60000.0 / bpm * 4);
  if (currentMillis - lastChordMillis >= msPerBar) {
    lastChordMillis += msPerBar;
    barsElapsed++;
    if (barsElapsed >= barperch) {
      barsElapsed = 0;
      triggerChord();
    }
  }
  prevDroneEnabled = droneEnabled;
}

void triggerChord() {
  // Triad drone: root, 3rd, 5th of the current scale
  // The root note is played one octave below
  int triad[3] = {
    (root + scale[currScale][1]) - 12, // root one octave down
    root + scale[currScale][3],
    root + scale[currScale][5]
  };

  // Duration for drone = X bars
  unsigned long msPerBar = (unsigned long)(60000.0 / bpm * 4);
  long chordDuration = msPerBar * barperch;

  // Send NoteOff for all triad notes (to avoid overlap)
  for (int i = 0; i < 3; i++) {
    MIDI.sendNoteOff(triad[i], 0, channel);
  }

  // Send chord notes via MIDI
  for (int i = 0; i < 3; i++) {
    setNote(triad[i], 40, chordDuration, channel);
  }
}
