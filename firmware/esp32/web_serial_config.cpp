#include <Arduino.h>
#include <ArduinoJson.h>
#include "globals.h"
#include <stdint.h>

#include "eeprom_settings.h"
// Use a local Preferences object in this file to avoid multiple definition

void processWebSerialConfig() {
    while (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, line);
        if (error) {
            Serial.print("{\"status\":\"deserializeJson() failed\",\"error\":\"");
            Serial.print(error.c_str());
            Serial.println("\"}");
            continue;
        }

        String command = doc["cmd"] | "";
        command.toLowerCase();

        if (command == "read") {
            // Always refresh globals from storage
            readSettings();

            doc.clear();
            doc["thr"] = throld;
            doc["scale"] = currScale;
            doc["chn"] = channel;
            doc["maxb"] = maxBrightness;
            doc["rnote"] = root;
            doc["bpm"] = bpm;
            doc["barperch"] = barperch;
            doc["drone"] = droneEnabled;
            doc["noteMin"] = noteMin;
            doc["noteMax"] = noteMax;
            doc["velocityMin"] = velocityMin;
            doc["velocityMax"] = velocityMax;
            doc["ccEnable"] = ccEnable;
            doc["bleEnabled"] = bleEnabled;

            serializeJson(doc, Serial);
            Serial.println();
        } else if (command == "save") {
            // Update all global variables from JSON
            throld = doc["thr"] | throld;
            currScale = doc["scale"] | currScale;
            channel = doc["chn"] | channel;
            maxBrightness = doc["maxb"] | maxBrightness;
            root = doc["rnote"] | root;
            bpm = doc["bpm"] | bpm;
            barperch = doc["barperch"] | barperch;
            droneEnabled = doc["drone"] | droneEnabled;
            noteMin = doc["noteMin"] | noteMin;
            noteMax = doc["noteMax"] | noteMax;
            velocityMin = doc["velocityMin"] | velocityMin;
            velocityMax = doc["velocityMax"] | velocityMax;
            ccEnable = doc["ccEnable"];
#if defined(BLE_MIDI_SUPPORTED) && BLE_MIDI_SUPPORTED
            bleEnabled = doc["bleEnabled"];
#else
            bleEnabled = doc["bleEnabled"];
            if (bleEnabled) {
                Serial.println("[ERROR] BLE not supported on this board. bleEnabled set to 0.");
                bleEnabled = 0;
            }
#endif

            // Save all settings using firmware's saveSettings (ensures ccEnable is persisted)
            saveSettings();

            Serial.println("{\"status\":\"saved\"}");
        } else {
            Serial.print("{\"status\":\"Unknown command\",\"command\":\"");
            Serial.print(command);
            Serial.println("\"}");
        }
    }
}
