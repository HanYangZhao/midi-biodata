#include "scale_functions.h"
#include "globals.h"

int scaleSearch(int note, int scale[], int scalesize) {
  for (byte i = 1; i < scalesize; i++) {
    if (note == scale[i]) {
      return note;
    }
    else {
      if (note < scale[i]) {
        return scale[i];
      }
    }
  }
  return 6;
}

int scaleNote_fast(int note, int root) {
  int scaled = note % 12;
  int octave = note / 12;
  int scalesize = (scale[currScale][0]);
  scaled = scaleSearch(scaled, scale[currScale], scalesize);
  scaled = (scaled + (12 * octave)) + (root % 12);
  return scaled;
}
