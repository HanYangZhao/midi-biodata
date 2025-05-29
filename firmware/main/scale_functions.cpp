#include "scale_functions.h"
#include "globals.h"

/**
 * Find the closest note in the scale (across all octaves) that is >= the input note.
 * @param note The input MIDI note number.
 * @param root The root note (MIDI number, e.g., 17 for F).
 * @param scaleType The scale type index (e.g., 1 for major).
 * @return The closest note in the scale >= input note.
 */
int scaleSearch(int note, int root, int scaleType, int previousNote) {
  int scalesize = scale[scaleType][0];
  int closestNote = -1;
  int minDiff = 128; // max MIDI note difference

  // Track the second closest note in case we need to avoid consecutive root notes
  int secondClosestNote = -1;
  int secondMinDiff = 128;

  for (int octave = 0; octave < 11; octave++) { // MIDI 0-127
    for (int i = 1; i < scalesize; i++) {
      int scaleNote = root + (octave * 12) + scale[scaleType][i];
      if (scaleNote >= 0 && scaleNote < 128) {
        int diff = abs(scaleNote - note);
        if (diff < minDiff) {
          // Update second closest before overwriting closest
          secondMinDiff = minDiff;
          secondClosestNote = closestNote;
          minDiff = diff;
          closestNote = scaleNote;
        }
        // If two notes are equally close, prefer the higher one
        else if (diff == minDiff && scaleNote > closestNote) {
          secondMinDiff = minDiff;
          secondClosestNote = closestNote;
          closestNote = scaleNote;
        }
      }
    }
  }
  // If closestNote is root and previousNote is also root, use secondClosestNote if available
  if (closestNote == root && previousNote == root && secondClosestNote != -1) {
    closestNote = secondClosestNote;
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
int scaleNote_fast(int note, int root, int scaleType, int previousNote) {
  int results = scaleSearch(note, root, scaleType, previousNote);
  return results;
}
