#include "midi_handling.h"

// BLE MIDI support conditional
#if BLE_MIDI_SUPPORTED
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// BLE MIDI characteristic and connection state
BLECharacteristic *pCharacteristic = nullptr;
bool deviceConnected = false;

// BLE MIDI UUIDs
#define SERVICE_UUID        "03B80E5A-EDE8-4B33-A751-6CE34EC4C700"
#define CHARACTERISTIC_UUID "7772E5DB-3868-4112-A1A9-F2669D106BF3"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("[BLE MIDI] Device connected");
    }
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("[BLE MIDI] Device disconnected");
    }
};

// BLE Characteristic Callbacks for receiving MIDI over BLE
class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) override {
        String rxValue = pCharacteristic->getValue();
        if (rxValue.length() < 5) return; // BLE MIDI packet is 5 bytes
        uint8_t data[5];
        rxValue.getBytes(data, 5);
        // Parse MIDI packet (assumes 5-byte BLE MIDI packet)
        // data[2]: status byte, data[3]: note/control, data[4]: velocity/value
        uint8_t status = data[2] & 0xF0;
        uint8_t channel = (data[2] & 0x0F) + 1;
        uint8_t data1 = data[3];
        uint8_t data2 = data[4];
        if (status == 0x90) { // Note On
            setNote(data1, data2, 0, channel, false);
        } else if (status == 0x80) { // Note Off
            // Implement note off handling if needed
        } else if (status == 0xB0) { // Control Change
            setControl(data1, data2, data2, 0);
        }
    }
};

void bleMidiInit() {
    Serial.println("[BLE MIDI] Before BLEDevice::init");
    BLEDevice::init("MIDI_BIODATA");
    Serial.println("[BLE MIDI] After BLEDevice::init");
    BLEServer *pServer = BLEDevice::createServer();
    Serial.println("[BLE MIDI] After createServer");
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));
    Serial.println("[BLE MIDI] After createService");
    pCharacteristic = pService->createCharacteristic(
        BLEUUID(CHARACTERISTIC_UUID),
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_WRITE_NR
    );
    Serial.println("[BLE MIDI] After createCharacteristic");
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
    pCharacteristic->addDescriptor(new BLE2902());
    pService->start();
    Serial.println("[BLE MIDI] After service start");
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->start();
    Serial.println("[BLE MIDI] BLE MIDI advertising started");
}

void bleMidiLoop() {
    // No periodic BLE MIDI logic needed for server mode
}
#endif // BLE_MIDI_SUPPORTED

// Define the USB MIDI object
Adafruit_USBD_MIDI usb_midi;

// Create the MIDI interface instance
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, usbMIDI);
// Instantiate the MIDI object on Serial1 for ESP32
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
void handleControlChange(byte channel, byte control, byte value) {
  if (channel == globalSettings.inputChannel ){
    trySwitchPresetByCC(control);
  }
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


void handleNoteOn(byte channel, byte note, byte velocity) {
  if (channel == globalSettings.inputChannel ){
    trySwitchPresetByNote(note);
    // Add any additional note-on logic here if needed
  }
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
  if (channel == globalSettings.inputChannel ){
    trySwitchPresetByPC(program);
  }
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
        // BLE MIDI Note On
        if (globalSettings.bleEnabled && deviceConnected && pCharacteristic) {
          uint8_t midiPacket[5] = {0x80, 0x80, (uint8_t)(0x90 | ((notechannel-1) & 0x0F)), (uint8_t)value, (uint8_t)velocity};
          pCharacteristic->setValue(midiPacket, 5);
          pCharacteristic->notify();
        }
#endif
      } else {
        MIDI.sendNoteOn(value, velocity, globalSettings.channel);
        usbMIDI.sendNoteOn(value, velocity, globalSettings.channel);
#if BLE_MIDI_SUPPORTED
        // BLE MIDI Note On
        if (globalSettings.bleEnabled && deviceConnected && pCharacteristic) {
          uint8_t midiPacket[5] = {0x80, 0x80, (uint8_t)(0x90 | ((globalSettings.channel-1) & 0x0F)), (uint8_t)value, (uint8_t)velocity};
          pCharacteristic->setValue(midiPacket, 5);
          pCharacteristic->notify();
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
  for (int i = 0; i < 12; ++i) {
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
  for (int i = 0; i < 12; ++i) {
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
  for (int i = 0; i < 12; ++i) {
    if (presets[i].midiNoteTrigger != -1 &&
        presets[i].midiNoteTrigger == noteNum) {
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
        // BLE MIDI Note Off
        if (globalSettings.bleEnabled && deviceConnected && pCharacteristic) {
          uint8_t midiPacket[5] = {0x80, 0x80, (uint8_t)(0x90 | ((noteArray[i].channel-1) & 0x0F)), (uint8_t)noteArray[i].value, 0};
          pCharacteristic->setValue(midiPacket, 5);
          pCharacteristic->notify();
        }
#endif
        } else {
          MIDI.sendNoteOff(noteArray[i].value, 0, globalSettings.channel);
          usbMIDI.sendNoteOff(noteArray[i].value, 0, globalSettings.channel);
#if BLE_MIDI_SUPPORTED
          // BLE MIDI Note Off
          if (globalSettings.bleEnabled && deviceConnected && pCharacteristic) {
            uint8_t midiPacket[5] = {0x80, 0x80, (uint8_t)(0x90 | ((globalSettings.channel-1) & 0x0F)), (uint8_t)noteArray[i].value, 0};
            pCharacteristic->setValue(midiPacket, 5);
            pCharacteristic->notify();
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
  if (presets[activePreset].droneChordQ == 0) { // triad
    chordNotes[0] = presets[activePreset].rootNote + scale[presets[activePreset].scale][1];
    chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][3];
    chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
    chordLen = 3;
  } else if (presets[activePreset].droneChordQ == 1) { // seventh
    chordNotes[0] = presets[activePreset].rootNote + scale[presets[activePreset].scale][1];
    chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][3];
    chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
    chordNotes[3] = presets[activePreset].rootNote + scale[presets[activePreset].scale][7];
    chordLen = 4;
  } else if (presets[activePreset].droneChordQ == 2) { // seventh sus 4
    chordNotes[0] = presets[activePreset].rootNote + scale[presets[activePreset].scale][1];
    chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][4];
    chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
    chordNotes[3] = presets[activePreset].rootNote + scale[presets[activePreset].scale][7];
    chordLen = 4;
  } else if (presets[activePreset].droneChordQ == 3) { // seventh sus 2
    chordNotes[0] = presets[activePreset].rootNote + scale[presets[activePreset].scale][1];
    chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][2];
    chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
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
  if (presets[activePreset].droneChordQ == 0) { // triad
    chordNotes[0] = (presets[activePreset].rootNote + scale[presets[activePreset].scale][1]) - 12; // root one octave down
    chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][3];
    chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
    chordLen = 3;
  } else if (presets[activePreset].droneChordQ == 1) { // seventh
    chordNotes[0] = (presets[activePreset].rootNote + scale[presets[activePreset].scale][1]) - 12;
    chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][3];
    chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
    chordNotes[3] = presets[activePreset].rootNote + scale[presets[activePreset].scale][7];
    chordLen = 4;
  } else if (presets[activePreset].droneChordQ == 2) { // seventh sus 4
    chordNotes[0] = (presets[activePreset].rootNote + scale[presets[activePreset].scale][1]) - 12;
    chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][4];
    chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
    chordNotes[3] = presets[activePreset].rootNote + scale[presets[activePreset].scale][7];
    chordLen = 4;
  } else if (presets[activePreset].droneChordQ == 3) { // seventh sus 2
    chordNotes[0] = (presets[activePreset].rootNote + scale[presets[activePreset].scale][1]) - 12;
    chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][2];
    chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
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
    // BLE MIDI Note On (server mode)
    if (globalSettings.bleEnabled && deviceConnected && pCharacteristic) {
      uint8_t midiPacket[5] = {0x80, 0x80, (uint8_t)(0x90 | ((globalSettings.channel-1) & 0x0F)), (uint8_t)chordNotes[i], (uint8_t)globalSettings.droneVel};
      pCharacteristic->setValue(midiPacket, 5);
      pCharacteristic->notify();
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
      // BLE MIDI Note Off (server mode)
      if (globalSettings.bleEnabled && deviceConnected && pCharacteristic) {
        uint8_t midiPacket[5] = {0x80, 0x80, (uint8_t)(0x90 | ((globalSettings.channel-1) & 0x0F)), (uint8_t)lastDroneNotes[i], 0};
        pCharacteristic->setValue(midiPacket, 5);
        pCharacteristic->notify();
      }
#endif
    }
  }
  lastDroneLen = 0;
  int chordNotes[4];
  int chordLen = 3;
  if (presets[activePreset].droneChordQ == 0) { // triad
    chordNotes[0] = (presets[activePreset].rootNote + scale[presets[activePreset].scale][1]) - 12;
    chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][3];
    chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
    chordLen = 3;
  } else if (presets[activePreset].droneChordQ == 1) { // seventh
    chordNotes[0] = (presets[activePreset].rootNote + scale[presets[activePreset].scale][1]) - 12;
    chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][3];
    chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
    chordNotes[3] = presets[activePreset].rootNote + scale[presets[activePreset].scale][7];
    chordLen = 4;
  } else if (presets[activePreset].droneChordQ == 2) { // seventh sus 4
    chordNotes[0] = (presets[activePreset].rootNote + scale[presets[activePreset].scale][1]) - 12;
    chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][4];
    chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
    chordNotes[3] = presets[activePreset].rootNote + scale[presets[activePreset].scale][7];
    chordLen = 4;
  } else if (presets[activePreset].droneChordQ == 3) { // seventh sus 2
    chordNotes[0] = (presets[activePreset].rootNote + scale[presets[activePreset].scale][1]) - 12;
    chordNotes[1] = presets[activePreset].rootNote + scale[presets[activePreset].scale][2];
    chordNotes[2] = presets[activePreset].rootNote + scale[presets[activePreset].scale][5];
    chordNotes[3] = presets[activePreset].rootNote + scale[presets[activePreset].scale][7];
    chordLen = 4;
  }
  for (int i = 0; i < chordLen; i++) {
    MIDI.sendNoteOn(chordNotes[i], globalSettings.droneVel, globalSettings.droneChannel);
    usbMIDI.sendNoteOn(chordNotes[i], globalSettings.droneVel, globalSettings.droneChannel);
#if BLE_MIDI_SUPPORTED
    // BLE MIDI Note On (server mode)
    if (globalSettings.bleEnabled && deviceConnected && pCharacteristic) {
      uint8_t midiPacket[5] = {0x80, 0x80, (uint8_t)(0x90 | ((globalSettings.channel-1) & 0x0F)), (uint8_t)chordNotes[i], (uint8_t)globalSettings.droneVel};
      pCharacteristic->setValue(midiPacket, 5);
      pCharacteristic->notify();
    }
#endif
    lastDroneNotes[i] = chordNotes[i];
  }
  lastDroneLen = chordLen;
}
