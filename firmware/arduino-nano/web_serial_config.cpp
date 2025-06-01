#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "globals.h"
#include <stdint.h>

// Use unique static config variables to avoid conflicts
static uint8_t configScale = 0;
static uint8_t configMaxBrightness = 0;
static uint8_t configChannel = 0;
static float configThreshold = 0;
static uint8_t configRootNote = 0;
static uint8_t configBpm = 120;
static uint8_t configbarperch = 1;
static uint8_t configDroneEnabled = 0;

void processWebSerialConfig() {
    while (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        StaticJsonDocument<64> doc;
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
            EEPROM.get(2, configScale);
            EEPROM.get(4, configMaxBrightness);
            EEPROM.get(6, configChannel);
            EEPROM.get(8, configThreshold);
            EEPROM.get(12, configRootNote);
            EEPROM.get(14, configBpm);
            EEPROM.get(16, configbarperch);
            EEPROM.get(18, configDroneEnabled);

            doc.clear();
            doc["thr"] = configThreshold;
            doc["scale"] = configScale;
            doc["chn"] = configChannel;
            doc["maxb"] = configMaxBrightness;
            doc["rnote"] = configRootNote;
            doc["bpm"] = configBpm;
            doc["barperch"] = configbarperch;
            doc["drone"] = configDroneEnabled;

            serializeJson(doc, Serial);
            Serial.println();
        } else if (command == "save") {
            configThreshold = doc["thr"] | 0.0;
            configScale = doc["scale"] | 0;
            configChannel = doc["chn"] | 0;
            configMaxBrightness = doc["maxb"] | 0;
            configRootNote = doc["rnote"] | 0;
            configBpm = doc["bpm"] | 120;
            configbarperch = doc["barperch"] | 4;
            configDroneEnabled = doc["drone"] | 0;

            EEPROM.put(2, configScale);
            EEPROM.put(4, configMaxBrightness);
            EEPROM.put(6, configChannel);
            EEPROM.put(8, configThreshold);
            EEPROM.put(12, configRootNote);
            EEPROM.put(14, configBpm);
            EEPROM.put(16, configbarperch);
            EEPROM.put(18, configDroneEnabled);

            // Update globals
            currScale = configScale;
            maxBrightness = configMaxBrightness;
            channel = configChannel;
            throld = configThreshold;
            root = configRootNote;
            bpm = configBpm;
            barperch = configbarperch;
            droneEnabled = configDroneEnabled;

            Serial.println("{\"status\":\"saved\"}");
        } else {
            Serial.print("{\"status\":\"Unknown command\",\"command\":\"");
            Serial.print(command);
            Serial.println("\"}");
        }
    }
}
