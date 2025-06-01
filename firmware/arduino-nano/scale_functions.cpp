#include "scale_functions.h"
#include "globals.h"

/**
 * Find the closest note in the scale (across all octaves) that is >= the input note.
 * @param note The input MIDI note number.
 * @param root The root note (MIDI number, e.g., 17 for F).
 * @param scaleType The scale type index (e.g., 1 for major).
 * @return The closest note in the scale >= input note.
 */

#include <stdint.h>
uint8_t scaleSearch(uint8_t note, uint8_t root, uint8_t scaleType, uint8_t previousNotes[2]) {
  uint8_t scalesize = scale[scaleType][0];
  int closestNote = -1;
  int minDiff = 128; // max MIDI note difference

  int closestNonRoot = -1;
  int minNonRootDiff = 128;

  for (uint8_t octave = 0; octave < 11; octave++) { // MIDI 0-127
    for (uint8_t i = 1; i < scalesize; i++) {
      int scaleNote = root + (octave * 12) + scale[scaleType][i];
      if (scaleNote >= 0 && scaleNote < 128) {
        int diff = abs(scaleNote - note);
        if (diff < minDiff) {
          minDiff = diff;
          closestNote = scaleNote;
        }
        // Track closest non-root note
        if (scaleNote != root && diff < minNonRootDiff) {
          minNonRootDiff = diff;
          closestNonRoot = scaleNote;
        }
      }
    }
  }
  // If root is in previous notes, return the fifth of the scale if available
  if (previousNotes[0] == root || previousNotes[1] == root) {
    int fifth = root;
    if (scale[scaleType][0] > 5) {
      fifth = root + scale[scaleType][5];
      // Clamp to MIDI range
      if (fifth < 0) fifth = 0;
      if (fifth > 127) fifth = 127;
    }
    return (uint8_t)fifth;
  }
  // Fallback: if not found, return input note
  if (closestNote == -1) {
    return note;
  }
  return (uint8_t)closestNote;
}

/**
 * Given a note, root, and scale type, return the closest note in the scale >= input note.
 * @param note The input MIDI note number.
 * @param root The root note (MIDI number, e.g., 17 for F).
 * @param scaleType The scale type index (e.g., 1 for major).
 * @return The closest note in the scale >= input note.
 */

uint8_t scaleNote_fast(uint8_t note, uint8_t root, uint8_t scaleType, uint8_t previousNotes[2]) {
  return scaleSearch(note, root, scaleType, previousNotes);
}
