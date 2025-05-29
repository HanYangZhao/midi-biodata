#ifndef WEB_SERIAL_CONFIG_H
#define WEB_SERIAL_CONFIG_H

/**
 * Handles all configuration via a single JSON document over USB Serial.
 * Call this regularly in the main loop to process config JSON commands.
 */
void processWebSerialConfig();

#endif // WEB_SERIAL_CONFIG_H
