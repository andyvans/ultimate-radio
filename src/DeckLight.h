#pragma once

#include "constants.h"
#include <OneButton.h>

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED_NeoMatrix.h>

#define DeckLightLedCount 24
#define DeckLightMatrixWidth 8
#define DeckLightMatrixHeight 3

class DeckLight
{
public:
  DeckLight();
  void Setup();
  void DisplayLine(int band);
  void DrawBluetoothBar();

private:
  // FastLed globals
  static const int BrightnessSettings[];
  static CRGBPalette16 purplePalette;
  
  // RGB LED matrix
  FastLED_NeoMatrix* matrix;
  CRGB matrixLeds[DeckLightLedCount];
};