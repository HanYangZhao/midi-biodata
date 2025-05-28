#include "scale_functions.h"
#include "globals.h"

/**
 * Find the closest note in the scale (across all octaves) that is >= the input note.
 * @param note The input MIDI note number.
 * @param root The root note (MIDI number, e.g., 17 for F).
 * @param scaleType The scale type index (e.g., 1 for major).
 * @return The closest note in the scale >= input note.
 */
int scaleSearch(int note, int root, int scaleType) {
  int scalesize = scale[scaleType][0];
  int closestNote = -1;
  int minDiff = 128; // max MIDI note difference

  // Search all possible scale notes in MIDI range
  for (int octave = 0; octave < 11; octave++) { // MIDI 0-127
    for (int i = 1; i < scalesize; i++) {
      int scaleNote = root + (octave * 12) + scale[scaleType][i];
      if (scaleNote >= 0 && scaleNote < 128) {
        int diff = abs(scaleNote - note);
        if (diff < minDiff) {
          minDiff = diff;
          closestNote = scaleNote;
        }
        // If two notes are equally close, prefer the higher one
        else if (diff == minDiff && scaleNote > closestNote) {
          closestNote = scaleNote;
        }
      }
    }
  }
  // Fallback: if not found, return input note
  if (closestNote == -1) {
    closestNote = note;
  }
  return closestNote;
}

/**
 * Given a note, root, and scale type, return the closest note in the scale >= input note.
 * @param note The input MIDI note number.
 * @param root The root note (MIDI number, e.g., 17 for F).
 * @param scaleType The scale type index (e.g., 1 for major).
 * @return The closest note in the scale >= input note.
 */
int scaleNote_fast(int note, int root, int scaleType) {
  // Serial.print("Note:");
  // Serial.println(note);
  // Serial.print("root:");
  // Serial.println(root);
  // Serial.println("scaletype:");
  // Serial.println(scaleType);
  int results = scaleSearch(note, root, scaleType);
  // Serial.println("scaled note:");
  // Serial.println(results);
  return results;
}
