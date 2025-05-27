#include "midi_handling.h"
#include "led_control.h"

void setNote(int value, int velocity, long duration, int notechannel) {
  for (int i = 0; i < polyphony; i++) {
    if (!noteArray[i].velocity) {
      noteArray[i].type = 0;
      noteArray[i].value = value;
      noteArray[i].velocity = velocity;
      noteArray[i].duration = currentMillis + duration;
      noteArray[i].channel = notechannel;

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
        if (QY8) {
          MIDI.sendNoteOn(noteArray[i].value, 0, noteArray[i].channel);
        } else {
          MIDI.sendNoteOn(noteArray[i].value, 0, channel);
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
    MIDI.sendNoteOn(i, 0, channel);
    if (QY8) {
      for (byte k = 1; k < 5; k++) {
        delay(1);
        MIDI.sendNoteOn(i, 0, k);
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
