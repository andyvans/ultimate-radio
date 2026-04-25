#pragma once
#include <FastLED.h>

// FastLED color palettes
// Note: DEFINE_GRADIENT_PALETTE creates a variable, so this header
// should only be included from a single .cpp file to avoid linker errors.
DEFINE_GRADIENT_PALETTE(PurpleGlobalPalette) {
    0, 0, 212, 255,    // blue
    255, 179, 0, 255   // purple
}; 