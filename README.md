# MIDI Biodata Sonification Device

This repository was built upon code from https://github.com/electricityforprogress/BiodataFeather

Here you find the hardware and enclosure design files for the MIDI Biodata Sonification Devices

- enclosures: contains FreeCAD design and STL files for the enclosure
- firmware: 
  - MIDI_Psychogalv_328p_v021: unmodified firmware from electricityforprogress/MIDIsprout
  - arduino-nano: firmware for MIDI Biodata Sonification Device by me. This firmware offers the following new features over original version
    - Instead of menus, we use a web page to configure the device. The webpage uses webserial to send a json config to the board.
    - Allow all 7 modes of the major scale to be selected. I'm also included the chromatic and Raga Bhairav scale.
    - Selectable root note
    - Chord drone with selectable bpm and frequency. The chord is a triad based on the scale chosen.
    - Refactored code into multipe files for easier understanding.
    - Remove menu codes to save space, some optimization to free up the stack.
  - esp32: Same as main but for ESP32, the arduino-LEDFader folder needs to be copied to .../Arduino/Libraries. This firmware offer the additional:
    - 12 presets can be configured, each preset contains its own scale and root note, and can be triggered via MIDI CC/PC/NOTE
    - BLE MIDI support
- kicad: hardware design as KiCAD project and Gerber files. The board is designed to be used add-on to any microcontroller but also has footprint for the esp32s3 supermini board.

The hardware design and firmware is based on the work of electricityforprogress and thus licensed under the MIT license.


The enclosure design is however licensed under CC0 (public domain), except for the logo which is under copyright of [DipthDesign](https://www.dipth.de). For private use, anyone is free to print the enclosure with logo and without permission.


## Installation 

### Arduino nano

Download https://github.com/jgillick/arduino-LEDFader/tree/master to .../Arduino/Libraries

### ESP32

* Copy ./firmware/esp32/arduino-LEDFader to .../Arduino/Libraries
* Install arduino tiny usb library
* Device mode only support S2/S3/P4 and additional Tools menu are needed
    USB Mode=USB-OTG (TinyUSB) for S3 and P4
    USB CDC On Boot=Enabled, USB Firmware MSC On Boot=Disabled, USB DFU On Boot=Disabled
* BLE MIDI only works on ESP32-S3


