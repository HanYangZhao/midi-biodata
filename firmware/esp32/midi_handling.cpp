#include "midi_handling.h"

#if BLE_MIDI_SUPPORTED
// BLE MIDI enable flag is now in globalSettings.bleEnabled
#endif

// Define the USB MIDI object
Adafruit_USBD_MIDI usb_midi;

// Create the MIDI interface instance
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, usbMIDI);
// Instantiate the MIDI object on Serial1 for ESP32
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
void handleControlChange(byte channel, byte control, byte value) {
  trySwitchPresetByCC(control);
}

// Forward declaration
void triggerChord();
void triggerChordFreeDrone();

// --- Chord timing and triad logic ---
static unsigned long lastChordMillis = 0;
static int barsElapsed = 0;

// --- BPM Drone MIDI clock tracking ---
static unsigned long midiClockTicks = 0;
static unsigned long lastMidiClockMillis = 0;

// --- Drone note management ---
static int lastDroneNotes[4] = {-1, -1, -1, -1};
static int lastDroneLen = 0;

// MIDI Start handler
void handleMidiStart() {
  // Turn off all previous drone notes
  for (int i = 0; i < lastDroneLen; i++) {
    if (lastDroneNotes[i] >= 0) {
      MIDI.sendNoteOff(lastDroneNotes[i], 0, globalSettings.droneChannel);
      usbMIDI.sendNoteOff(lastDroneNotes[i], 0, globalSettings.droneChannel);
    }
  }
  lastDroneLen = 0;

  // Reset timer and bar count
  lastChordMillis = currentMillis;
  barsElapsed = 0;
  midiClockTicks = 0;

  // Immediately trigger the chord (turn on drone notes)
  if (globalSettings.droneMode == 1) {
    triggerChord();
  } else {
    triggerChordFreeDrone();
  }
}

// MIDI Clock handler (24 ppqn)
void handleMidiClock() {
  if (globalSettings.droneMode != 1) return; // Only for BPMDrone
  midiClockTicks++;
  // 24 clocks per quarter note, 4 quarters per bar = 96 clocks per bar
  if (midiClockTicks % (96 * globalSettings.barperch) == 0) {
    triggerChord();
  }
}

#if BLE_MIDI_SUPPORTED
// BLE MIDI Control Change callback
void bleControlChangeCallback(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp) {
  trySwitchPresetByCC(controller);
}

// BLE MIDI Program Change callback
void bleProgramChangeCallback(uint8_t channel, uint8_t program, uint16_t timestamp) {
  trySwitchPresetByPC(program);
}

// BLE MIDI Start callback
void bleStartCallback(uint16_t timestamp) {
  handleMidiStart();
}

// BLE MIDI Clock callback
void bleClockCallback(uint16_t timestamp) {
  handleMidiClock();
}

// BLE MIDI initialization and loop
void bleMidiInit() {
    BLEMidiClient.begin("Midi client");
    BLEMidiClient.setControlChangeCallback(bleControlChangeCallback);
    BLEMidiClient.setProgramChangeCallback(bleProgramChangeCallback);
    BLEMidiClient.setStartCallback(bleStartCallback);
    BLEMidiClient.setClockCallback(bleClockCallback);
}

void bleMidiLoop() {
    if (!bleEnabled) return;
    if (!BLEMidiClient.isConnected()) {
        int nDevices = BLEMidiClient.scan();
        if (nDevices > 0) {
            if (BLEMidiClient.connect(0)) {
                Serial.println("BLE MIDI Connection established");
            } else {
                Serial.println("BLE MIDI Connection failed");
                delay(3000);
            }
        }
    }
}
#else
// BLE MIDI not supported: do not define stubs here, only in the header
#endif

void handleNoteOn(byte channel, byte note, byte velocity) {
  trySwitchPresetByNote(note);
  // Add any additional note-on logic here if needed
}

// USB/Serial MIDI Start handler
void handleUsbMidiStart() {
  handleMidiStart();
}

// USB/Serial MIDI Clock handler
void handleUsbMidiClock() {
  handleMidiClock();
}

void handleProgramChange(byte channel, byte program) {
  trySwitchPresetByPC(program);
}

void setupMidiPresetHandlers() {
  MIDI.setHandleControlChange(handleControlChange);
  usbMIDI.setHandleControlChange(handleControlChange);
  MIDI.setHandleProgramChange(handleProgramChange);
  usbMIDI.setHandleProgramChange(handleProgramChange);

  MIDI.setHandleNoteOn(handleNoteOn);
  usbMIDI.setHandleNoteOn(handleNoteOn);

  // MIDI Start (0xFA) handler for both interfaces
  MIDI.setHandleStart(handleUsbMidiStart);
  usbMIDI.setHandleStart(handleUsbMidiStart);

  // MIDI Clock (0xF8) handler for both interfaces
  MIDI.setHandleClock(handleUsbMidiClock);
  usbMIDI.setHandleClock(handleUsbMidiClock);
}

void debugPrintNote(int value, int velocity, int notechannel) {
  Serial.print("Note : value=");
  Serial.print(value);
  Serial.print(" channel=");
  Serial.print(notechannel);
  Serial.print(" velocity=");
  Serial.println(velocity);
}

void setNote(int value, int velocity, long duration, int notechannel, bool debug) {

  for (int i = 0; i < polyphony; i++) {

    if (!noteArray[i].velocity) {
      noteArray[i].type = 0;
      noteArray[i].value = value;
      noteArray[i].velocity = velocity;
      noteArray[i].duration = currentMillis + duration;
      noteArray[i].channel = notechannel;

      if(debug) {
        debugPrintNote(value, velocity, notechannel);
      }

      if (QY8) {
        MIDI.sendNoteOn(value, velocity, notechannel);
        usbMIDI.sendNoteOn(value, velocity, notechannel);
#if BLE_MIDI_SUPPORTED
        if (globalSettings.bleEnabled && BLEMidiClient.isConnected()) {
          BLEMidiClient.noteOn(notechannel, value, velocity);
        }
#endif
      } else {
        MIDI.sendNoteOn(value, velocity, globalSettings.channel);
        usbMIDI.sendNoteOn(value, velocity, globalSettings.channel);
#if BLE_MIDI_SUPPORTED
        if (globalSettings.bleEnabled && BLEMidiClient.isConnected()) {
          BLEMidiClient.noteOn(globalSettings.channel, value, velocity);
        }
#endif
      }

      if (noteLEDs == 1) {
        rampUp(i, globalSettings.maxBrightness, duration);
      } else if (noteLEDs == 2) {
        rampUp(i + 1, globalSettings.maxBrightness, duration);
      }
      break;
    }
  }
}

void trySwitchPresetByCC(uint8_t ccNum) {
  for (int i = 0; i < 8; ++i) {
    if (presets[i].midiCCTrigger == -1) continue;
    if (presets[i].midiCCTrigger == ccNum) {
      if (activePreset != i) {
        activePreset = i;
        Serial.print("[PRESET] Switched to preset ");
        Serial.println(i + 1);
        // Notify web UI of active preset change
        Serial.print("{\"status\":\"active_preset_set\",\"index\":");
        Serial.print(i);
        Serial.println("}");
      }
      // In freeDrone mode, change drone notes immediately
      if (globalSettings.droneEnabled && globalSettings.droneMode == 0) {
        triggerChordFreeDrone();
      }
      break;
    }
  }
}

void trySwitchPresetByPC(uint8_t pcNum) {
  for (int i = 0; i < 8; ++i) {
    if (presets[i].midiPCTrigger == -1) continue;
    if (presets[i].midiPCTrigger == pcNum) {
      if (activePreset != i) {
        activePreset = i;
        Serial.print("[PRESET] Switched to preset ");
        Serial.println(i + 1);
      }
      // In freeDrone mode, change drone notes immediately
      if (globalSettings.droneEnabled && globalSettings.droneMode == 0) {
        triggerChordFreeDrone();
      }
      break;
    }
  }
}

// Switch preset by MIDI note (noteNum)
// Only triggers if midiNoteTrigger != -1 and (noteNum < noteMin or noteNum > noteMax)
void trySwitchPresetByNote(uint8_t noteNum) {
  for (int i = 0; i < 8; ++i) {
    if (presets[i].midiNoteTrigger != -1 &&
        presets[i].midiNoteTrigger == noteNum &&
        (noteNum < globalSettings.noteMin || noteNum > globalSettings.noteMax)) {
      if (activePreset != i) {
        activePreset = i;
        Serial.print("[PRESET] Switched to preset (note) ");
        Serial.println(i + 1);
        Serial.print("{\"status\":\"active_preset_set\",\"index\":");
        Serial.print(i);
        Serial.println("}");
      }
      if (globalSettings.droneEnabled && globalSettings.droneMode == 0) {
        triggerChordFreeDrone();
      }
      break;
    }
  }
}

void setControl(int type, int value, int velocity, long duration) {
  // Check for preset switch by CC
  trySwitchPresetByCC(type);

  controlMessage.type = type;
  controlMessage.value = value;
  controlMessage.velocity = velocity;
  controlMessage.period = duration;
  controlMessage.duration = currentMillis + duration;
  // Direct CC logic: send CC immediately, schedule off
  if (globalSettings.ccEnable == 1) {
    Serial.print("[MIDI CC] type=");
    Serial.print(controlMessage.type);
    Serial.print(" value=");
    Serial.print(value);
    Serial.print(" channel=");
    Serial.println(globalSettings.channel);
    MIDI.sendControlChange(type, velocity, globalSettings.channel);
    usbMIDI.sendControlChange(type, velocity, globalSettings.channel);
    controlMessage.value = velocity; // Track last sent value
  }
}

void checkControl() {
  // Guard: skip if not properly initialized
  if (controlMessage.period == 0 || controlMessage.type == 0) {
    return;
  }
  // Range checks
  if (controlMessage.value < 0) controlMessage.value = 0;
  if (controlMessage.value > 127) controlMessage.value = 127;
  if (controlLED < 0 || controlLED >= LED_NUM) {
    // Auto-correct invalid controlLED to 0
    controlLED = 0;
  }
  // Direct CC logic: turn off after duration
  static int lastSentValue = -1;
  if (globalSettings.ccEnable == 1) {
    if (controlMessage.value != 0 && currentMillis > controlMessage.duration) {
      Serial.print("[MIDI CC] type=");
      Serial.print(controlMessage.type);
      Serial.print(" value=0 (auto reset)");
      Serial.print(" channel=");
      Serial.println(globalSettings.channel);
      MIDI.sendControlChange(controlMessage.type, 0, globalSettings.channel);
      usbMIDI.sendControlChange(controlMessage.type, 0, globalSettings.channel);
      controlMessage.value = 0;
      lastSentValue = 0;
    }
    return;
  }
}

void checkNote() {
  for (int i = 0; i < polyphony; i++) {
    if (noteArray[i].velocity) {
      if (noteArray[i].duration <= currentMillis) {
        debugPrintNote(noteArray[i].value, 0, noteArray[i].channel);
        if (QY8) {
          MIDI.sendNoteOff(noteArray[i].value, 0, noteArray[i].channel);
          usbMIDI.sendNoteOff(noteArray[i].value, 0, noteArray[i].channel);
#if BLE_MIDI_SUPPORTED
          if (bleEnabled && BLEMidiClient.isConnected()) {
            BLEMidiClient.noteOff(noteArray[i].channel, noteArray[i].value, 0);
          }
#endif
        } else {
          MIDI.sendNoteOff(noteArray[i].value, 0, globalSettings.channel);
          usbMIDI.sendNoteOff(noteArray[i].value, 0, globalSettings.channel);
#if BLE_MIDI_SUPPORTED
          if (globalSettings.bleEnabled && BLEMidiClient.isConnected()) {
            BLEMidiClient.noteOff(globalSettings.channel, noteArray[i].value, 0);
          }
#endif
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
    MIDI.sendNoteOff(i, 0, globalSettings.channel);
    usbMIDI.sendNoteOff(i, 0, globalSettings.channel);
    if (QY8) {
      for (byte k = 1; k < 5; k++) {
        delay(1);
        MIDI.sendNoteOff(i, 0, k);
        usbMIDI.sendNoteOff(i, 0, k);
      }
    }
  }
}

void midiSerial(int type, int channel, int data1, int data2) {
  data1 &= 0x7F;
  data2 &= 0x7F;

  // Check for preset switch by PC (Program Change)
  if ((type & 0xF0) == 0xC0) { // 0xC0 = Program Change
    trySwitchPresetByPC(data1);
  }

  // MIDI Start (0xFA)
  if (type == 0xFA) {
    handleMidiStart();
  }
  // MIDI Clock (0xF8)
  if (type == 0xF8) {
    handleMidiClock();
  }

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
}

void midiChordTick() {
  static int prevDroneEnabled = 1;
  // Chord notes: triad or seventh
  int chordNotes[4];
  int chordLen = 3;
  chordNotes[0] = presets[activePreset].rootNote + scale[presets[activePreset].scale][1];
  chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][3];
  chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
  if (presets[activePreset].droneChordQ == 1) {
    chordNotes[3] = presets[activePreset].rootNote + scale[presets[activePreset].scale][7];
    chordLen = 4;
  }

  if (!globalSettings.droneEnabled) {
    if (prevDroneEnabled) {
      // Drone was just disabled, turn off notes
      for (int i = 0; i < lastDroneLen; i++) {
        if (lastDroneNotes[i] >= 0) {
          MIDI.sendNoteOff(lastDroneNotes[i], 0, globalSettings.channel);
          usbMIDI.sendNoteOff(lastDroneNotes[i], 0, globalSettings.channel);
        }
      }
      lastDroneLen = 0;
    }
    prevDroneEnabled = globalSettings.droneEnabled;
    return;
  }

  // BPMDrone mode: ignore timer, handled by MIDI clock
  if (globalSettings.droneMode == 1) {
    prevDroneEnabled = globalSettings.droneEnabled;
    return;
  }

  // FreeDrone mode: trigger chord on enable, and when preset changes
  if (globalSettings.droneEnabled && prevDroneEnabled == 0) {
    triggerChordFreeDrone();
  }
  prevDroneEnabled = globalSettings.droneEnabled;
}

void triggerChord() {
  // Chord: triad or seventh, root note played one octave down
  int chordNotes[4];
  int chordLen = 3;
  chordNotes[0] = (presets[activePreset].rootNote + scale[presets[activePreset].scale][1]) - 12; // root one octave down
  chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][3];
  chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
  if (presets[activePreset].droneChordQ == 1) {
    chordNotes[3] = presets[activePreset].rootNote + scale[presets[activePreset].scale][7];
    chordLen = 4;
  }

  // Send NoteOff for all previous drone notes (to avoid overlap)
  for (int i = 0; i < lastDroneLen; i++) {
    if (lastDroneNotes[i] >= 0) {
      MIDI.sendNoteOff(lastDroneNotes[i], 0, globalSettings.channel);
      usbMIDI.sendNoteOff(lastDroneNotes[i], 0, globalSettings.channel);
    }
  }

  // Send chord notes via MIDI (directly, not using setNote/noteArray)
  for (int i = 0; i < chordLen; i++) {
    MIDI.sendNoteOn(chordNotes[i], globalSettings.droneVel, globalSettings.droneChannel);
    usbMIDI.sendNoteOn(chordNotes[i], globalSettings.droneVel, globalSettings.droneChannel);
#if BLE_MIDI_SUPPORTED
    if (globalSettings.bleEnabled && BLEMidiClient.isConnected()) {
      BLEMidiClient.noteOn(globalSettings.channel, chordNotes[i], globalSettings.droneVel);
    }
#endif
    lastDroneNotes[i] = chordNotes[i];
  }
  lastDroneLen = chordLen;
}

// FreeDrone: just play the chord, don't turn off previous notes
void triggerChordFreeDrone() {
  Serial.println("free drone trigger");
  // Turn off all previous drone notes
  for (int i = 0; i < lastDroneLen; i++) {
    if (lastDroneNotes[i] >= 0) {
      MIDI.sendNoteOff(lastDroneNotes[i], 0, globalSettings.droneChannel);
      usbMIDI.sendNoteOff(lastDroneNotes[i], 0, globalSettings.droneChannel);
#if BLE_MIDI_SUPPORTED
      if (globalSettings.bleEnabled && BLEMidiClient.isConnected()) {
        BLEMidiClient.noteOff(globalSettings.channel, lastDroneNotes[i], 0);
      }
#endif
    }
  }
  lastDroneLen = 0;
  int chordNotes[4];
  int chordLen = 3;
  chordNotes[0] = (presets[activePreset].rootNote + scale[presets[activePreset].scale][1]) - 12;
  chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][3];
  chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
  if (presets[activePreset].droneChordQ == 1) {
    chordNotes[3] = presets[activePreset].rootNote + scale[presets[activePreset].scale][7];
    chordLen = 4;
  }
  for (int i = 0; i < chordLen; i++) {
    MIDI.sendNoteOn(chordNotes[i], globalSettings.droneVel, globalSettings.droneChannel);
    usbMIDI.sendNoteOn(chordNotes[i], globalSettings.droneVel, globalSettings.droneChannel);
#if BLE_MIDI_SUPPORTED
    if (globalSettings.bleEnabled && BLEMidiClient.isConnected()) {
      BLEMidiClient.noteOn(globalSettings.channel, chordNotes[i], globalSettings.droneVel);
    }
#endif
    lastDroneNotes[i] = chordNotes[i];
  }
  lastDroneLen = chordLen;
}
