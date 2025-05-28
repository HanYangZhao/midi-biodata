#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "globals.h"

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

        String command = doc["command"] | "";
        command.toLowerCase();

        if (command == "readall") {
            int scale = 0;
            int maxbrightness = 0;
            int channel = 0;
            float threshold = 0;
            int rootnote = 0;

            EEPROM.get(2, scale);
            EEPROM.get(4, maxbrightness);
            EEPROM.get(6, channel);
            EEPROM.get(8, threshold);
            EEPROM.get(12, root);

            StaticJsonDocument<64> outDoc;
            outDoc["threshold"] = threshold;
            outDoc["scale"] = scale;
            outDoc["channel"] = channel;
            outDoc["maxbrightness"] = maxbrightness;
            outDoc["rootnote"] = root;

            serializeJson(outDoc, Serial);
            Serial.println();
        } else if (command == "saveall") {
            float threshold = doc["threshold"] | 0.0;
            int scale = doc["scale"] | 0;
            int channelVal = doc["channel"] | 0;
            int maxbrightness = doc["maxbrightness"] | 0;
            int rootnote = doc["rootnote"] | 0;

            EEPROM.put(2, scale);
            EEPROM.put(4, maxbrightness);
            EEPROM.put(6, channelVal);
            EEPROM.put(8, threshold);
            EEPROM.put(12, rootnote);

            // Update globals
            currScale = scale;
            maxBrightness = maxbrightness;
            channel = channelVal;
            threshold = threshold;
            root = rootnote;

            Serial.println("{\"status\":\"saved\"}");
        } else {
            Serial.print("{\"status\":\"Unknown command\",\"command\":\"");
            Serial.print(command);
            Serial.println("\"}");
        }
    }
}
