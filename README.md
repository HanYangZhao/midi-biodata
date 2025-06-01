# MIDI Biodata Sonification system

This repository was originally a fork of https://github.com/leetronics/midi-biodata.

Here you find the hardware and enclosure design files for the MIDI Biodata Sonification Devices

- enclosures: contains FreeCAD design and STL files for the enclosure
- firmware: 
  - MIDI_Psychogalv_328p_v021: unmodified firmware from electricityforprogress/MIDIsprout
  - main: firmware for MIDI Biodata Sonification Device by me. This firmware offers the following new features over 13-37.org version
    - Instead of menus, we use a web page to configure the device. The webpage uses webserial to send a json config to the board.
    - Allow all 7 modes of the major scale to be selected. I'm also included the chromatic and Raga Bhairav scale.
    - Selectable root note
    - Chord drone with selectable bpm and frequency. The chord is a triad based on the scale chosen.
    - Refactored code into multipe files for easier understanding.
    - Remove menu codes to save space, some optimization to free up the stack.
  - esp32: Same as main but for ESP32, the arduino-LEDFader folder needs to be copied to .../Arduino/Libraries
- kicad: hardware design as KiCAD project and Gerber files. The board is designed to be used add-on to any microcontroller. It doesn't contain the microcontroller.

The hardware design and firmware is based on the work of electricityforprogress and thus licensed under the MIT license.


The enclosure design is however licensed under CC0 (public domain), except for the logo which is under copyright of [DipthDesign](https://www.dipth.de). For private use, anyone is free to print the enclosure with logo and without permission.
