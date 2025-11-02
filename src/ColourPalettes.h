#pragma once

// FastLED color palettes
DEFINE_GRADIENT_PALETTE(PurpleGlobalPalette){
    0, 0, 212, 255,    // blue
    255, 179, 0, 255}; // purple

DEFINE_GRADIENT_PALETTE(OutRunGlobalPalette){
    0, 141, 0, 100,   // purple
    127, 255, 192, 0, // yellow
    255, 0, 5, 255};  // blue

DEFINE_GRADIENT_PALETTE(GreenBlueGlobalPalette){
    0, 0, 255, 60,    // green
    64, 0, 236, 255,  // cyan
    128, 0, 5, 255,   // blue
    192, 0, 236, 255, // cyan
    255, 0, 255, 60}; // green

DEFINE_GRADIENT_PALETTE(RedYellowGlobalPalette){
    0, 200, 200, 200,    // white
    64, 255, 218, 0,     // yellow
    128, 231, 0, 0,      // red
    192, 255, 218, 0,    // yellow
    255, 200, 200, 200}; // white