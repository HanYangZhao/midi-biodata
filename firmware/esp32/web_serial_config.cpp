#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "globals.h"
#include <stdint.h>

#include "eeprom_settings.h"
// Use a local Preferences object in this file to avoid multiple definition

void processWebSerialConfig() {
    while (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        StaticJsonDocument<128> doc;
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
            Preferences localPrefs;
            localPrefs.begin("midi-bio", true);

            doc.clear();
            doc["thr"] = localPrefs.getFloat("throld", 2.3f);
            doc["scale"] = localPrefs.getUShort("currScale", 1);
            doc["chn"] = localPrefs.getUShort("channel", 1);
            doc["maxb"] = localPrefs.getUShort("maxBrightness", 255);
            doc["rnote"] = localPrefs.getUShort("root", 60);
            doc["bpm"] = localPrefs.getUShort("bpm", 120);
            doc["barperch"] = localPrefs.getUShort("barperch", 1);
            doc["drone"] = localPrefs.getUShort("droneEnabled", 0);
            doc["noteMin"] = localPrefs.getUChar("noteMin", 21);
            doc["noteMax"] = localPrefs.getUChar("noteMax", 108);
            doc["velocityMin"] = localPrefs.getUChar("velocityMin", 30);
            doc["velocityMax"] = localPrefs.getUChar("velocityMax", 127);

            localPrefs.end();

            serializeJson(doc, Serial);
            Serial.println();
        } else if (command == "save") {
            Preferences localPrefs;
            localPrefs.begin("midi-bio", false);

            localPrefs.putFloat("throld", doc["thr"] | 2.3f);
            localPrefs.putUShort("currScale", doc["scale"] | 1);
            localPrefs.putUShort("channel", doc["chn"] | 1);
            localPrefs.putUShort("maxBrightness", doc["maxb"] | 255);
            localPrefs.putUShort("root", doc["rnote"] | 60);
            localPrefs.putUShort("bpm", doc["bpm"] | 120);
            localPrefs.putUShort("barperch", doc["barperch"] | 1);
            localPrefs.putUShort("droneEnabled", doc["drone"] | 0);
            localPrefs.putUChar("noteMin", doc["noteMin"] | 21);
            localPrefs.putUChar("noteMax", doc["noteMax"] | 108);
            localPrefs.putUChar("velocityMin", doc["velocityMin"] | 30);
            localPrefs.putUChar("velocityMax", doc["velocityMax"] | 127);

            localPrefs.end();

            // Update globals
            readSettings();

            Serial.println("{\"status\":\"saved\"}");
        } else {
            Serial.print("{\"status\":\"Unknown command\",\"command\":\"");
            Serial.print(command);
            Serial.println("\"}");
        }
    }
}
