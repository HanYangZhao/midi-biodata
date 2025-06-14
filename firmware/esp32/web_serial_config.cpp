#include <Arduino.h>
#include <ArduinoJson.h>
#include "globals.h"
#include <stdint.h>
#include "midi_handling.h"

#include "eeprom_settings.h"
// Use a local Preferences object in this file to avoid multiple definition

void processWebSerialConfig() {
    static String serialBuffer;
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            serialBuffer.trim();
            if (serialBuffer.length() == 0) {
                serialBuffer = "";
                return;
            }
            String line = serialBuffer;
            serialBuffer = "";

            StaticJsonDocument<1024> doc;
            DeserializationError error = deserializeJson(doc, line);
            if (error) {
                Serial.print("{\"status\":\"deserializeJson() failed\",\"error\":\"");
                Serial.print(error.c_str());
                Serial.println("\"}");
                return;
            }

            String command = doc["cmd"] | "";
            command.toLowerCase();

            if (command == "read") {
                // Always refresh globals from storage
                readSettings();

                doc.clear();
                doc["thr"] = globalSettings.threshold;
                doc["scale"] = presets[0].scale; // Default: preset 0
                doc["chn"] = globalSettings.channel;
                doc["inputChannel"] = globalSettings.inputChannel;
                doc["maxb"] = globalSettings.maxBrightness;
                doc["rnote"] = presets[0].rootNote; // Default: preset 0
                doc["bpm"] = globalSettings.bpm;
                doc["barperch"] = globalSettings.barperch;
                doc["drone"] = globalSettings.droneEnabled;
                doc["droneMode"] = globalSettings.droneMode;
                doc["noteMin"] = globalSettings.noteMin;
                doc["noteMax"] = globalSettings.noteMax;
                doc["velocityMin"] = globalSettings.velocityMin;
                doc["velocityMax"] = globalSettings.velocityMax;
                doc["droneVel"] = globalSettings.droneVel;
                doc["droneChannel"] = globalSettings.droneChannel;
                doc["ccEnable"] = globalSettings.ccEnable;
                doc["bleEnabled"] = globalSettings.bleEnabled;
                doc["activePreset"] = activePreset;

                // Add firmware version
                doc["firmwareVersion"] = FIRMWARE_VERSION;

                // Add all presets to response
                JsonArray arr = doc.createNestedArray("presets");
                for (int i = 0; i < 12; ++i) {
                    JsonObject p = arr.createNestedObject();
                    p["scale"] = presets[i].scale;
                    p["rootNote"] = presets[i].rootNote;
                    p["midiCCTrigger"] = presets[i].midiCCTrigger;
                    p["midiPCTrigger"] = presets[i].midiPCTrigger;
                    p["midiNoteTrigger"] = presets[i].midiNoteTrigger;
                    p["droneChordQ"] = presets[i].droneChordQ;
                }

                serializeJson(doc, Serial);
                Serial.println();
            } else if (command == "save_all") {
                // Save global settings and all presets
                globalSettings.threshold = doc["thr"] | globalSettings.threshold;
                globalSettings.channel = doc["chn"] | globalSettings.channel;
                globalSettings.inputChannel = doc["inputChannel"] | globalSettings.inputChannel;
                globalSettings.maxBrightness = doc["maxb"] | globalSettings.maxBrightness;
                globalSettings.bpm = doc["bpm"] | globalSettings.bpm;
                globalSettings.barperch = doc["barperch"] | globalSettings.barperch;
                globalSettings.droneEnabled = doc["drone"] | globalSettings.droneEnabled;
                globalSettings.droneMode = doc["droneMode"] | globalSettings.droneMode;
                globalSettings.noteMin = doc["noteMin"] | globalSettings.noteMin;
                globalSettings.noteMax = doc["noteMax"] | globalSettings.noteMax;
                globalSettings.velocityMin = doc["velocityMin"] | globalSettings.velocityMin;
                globalSettings.velocityMax = doc["velocityMax"] | globalSettings.velocityMax;
                globalSettings.droneVel = doc["droneVel"] | globalSettings.droneVel;
                globalSettings.droneChannel = doc["droneChannel"] | globalSettings.droneChannel;
                globalSettings.ccEnable = doc["ccEnable"];
        #if defined(BLE_MIDI_SUPPORTED) && BLE_MIDI_SUPPORTED
                globalSettings.bleEnabled = doc["bleEnabled"];
        #else
                globalSettings.bleEnabled = doc["bleEnabled"];
                if (globalSettings.bleEnabled) {
                    Serial.println("[ERROR] BLE not supported on this board. bleEnabled set to 0.");
                    globalSettings.bleEnabled = 0;
                }
        #endif
                // Update all presets from JSON array
                if (doc.containsKey("presets") && doc["presets"].is<JsonArray>()) {
                    JsonArray arr = doc["presets"].as<JsonArray>();
                    int count = arr.size() < 12 ? arr.size() : 12;
                    for (int i = 0; i < count; ++i) {
                        JsonObject p = arr[i];
                        presets[i].scale = p["scale"] | presets[i].scale;
                        presets[i].rootNote = p["rootNote"] | presets[i].rootNote;
                        presets[i].midiCCTrigger = p["midiCCTrigger"] | presets[i].midiCCTrigger;
                        presets[i].midiPCTrigger = p["midiPCTrigger"] | presets[i].midiPCTrigger;
                        presets[i].midiNoteTrigger = p["midiNoteTrigger"] | presets[i].midiNoteTrigger;
                        presets[i].droneChordQ = p["droneChordQ"] | presets[i].droneChordQ;
                    }
                }
                saveSettings();
                Serial.println("{\"status\":\"all_saved\"}");
            } else if (command == "save_presets") {
                // If an index is provided, update only that preset
                if (doc.containsKey("index") && doc.containsKey("presets") && doc["presets"].is<JsonArray>()) {
                    int idx = doc["index"];
                    JsonArray arr = doc["presets"].as<JsonArray>();
                    if (idx >= 0 && idx < 12 && arr.size() > 0) {
                        JsonObject p = arr[0];
                        presets[idx].scale = p["scale"] | presets[idx].scale;
                        presets[idx].rootNote = p["rootNote"] | presets[idx].rootNote;
                        presets[idx].midiCCTrigger = p["midiCCTrigger"] | presets[idx].midiCCTrigger;
                        presets[idx].midiPCTrigger = p["midiPCTrigger"] | presets[idx].midiPCTrigger;
                        presets[idx].midiNoteTrigger = p["midiNoteTrigger"] | presets[idx].midiNoteTrigger;
                        presets[idx].droneChordQ = p["droneChordQ"] | presets[idx].droneChordQ;
                        saveSettings();
                        Serial.print("{\"status\":\"preset_saved\",\"index\":");
                        Serial.print(idx);
                        Serial.println("}");
                    } else {
                        Serial.println("{\"status\":\"error\",\"error\":\"Invalid preset index or array\"}");
                    }
                } else if (doc.containsKey("presets") && doc["presets"].is<JsonArray>()) {
                    // Update all presets from JSON array (legacy/multi)
                    JsonArray arr = doc["presets"].as<JsonArray>();
                    int count = arr.size() < 12 ? arr.size() : 12;
                    for (int i = 0; i < count; ++i) {
                        JsonObject p = arr[i];
                        presets[i].scale = p["scale"] | presets[i].scale;
                        presets[i].rootNote = p["rootNote"] | presets[i].rootNote;
                        presets[i].midiCCTrigger = p["midiCCTrigger"] | presets[i].midiCCTrigger;
                        presets[i].midiPCTrigger = p["midiPCTrigger"] | presets[i].midiPCTrigger;
                        presets[i].droneChordQ = p["droneChordQ"] | presets[i].droneChordQ;
                    }
                    saveSettings();
                    Serial.println("{\"status\":\"presets_saved\"}");
                } else {
                    Serial.println("{\"status\":\"error\",\"error\":\"Invalid presets array\"}");
                }
            } else if (command == "activate_preset") {
                // Set the active preset index from JSON
                int idx = doc["index"] | 0;
                if (idx >= 0 && idx < 12) {
                    activePreset = idx;
                    Serial.print("{\"status\":\"active_preset_set\",\"index\":");
                    Serial.print(idx);
                    Serial.println("}");
                    // Trigger freedrone chord if enabled and in freedrone mode
                    if (globalSettings.droneEnabled && globalSettings.droneMode == 0) {
                        triggerChordFreeDrone();
                    }
                } else {
                    Serial.println("{\"status\":\"error\",\"error\":\"Invalid preset index\"}");
                }
            } else if (command == "get_active_preset") {
                // Respond with the current active preset index
                Serial.print("{\"status\":\"active_preset\",\"activePreset\":");
                Serial.print(activePreset);
                Serial.println("}");
            } else {
                Serial.print("{\"status\":\"Unknown command\",\"command\":\"");
                Serial.print(command);
                Serial.println("\"}");
            }
        } else {
            serialBuffer += c;
        }
    }
}
