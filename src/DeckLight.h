#pragma once

#include "constants.h"
#include <OneButton.h>

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED_NeoMatrix.h>

#define DeckLightLedCount 24
#define DeckLightMatrixWidth 3
#define DeckLightMatrixHeight 8

class DeckLight
{
public:
  DeckLight();
  void Setup();
  void DisplayAudio(int bandValues[]);
  void ChangeTheme();
  void StartAutoMode();  
  void BrightnessOff();
  void ChangeBrightness();
  void DisplayLine(int band);

private:
  void UpdateBarVuMeter(int bandValues[]);
  void UpdateAnalogVuMeter();
  void RainbowBars(int band, int barHeight);
  void PurpleBars(int band, int barHeight);
  void ChangingBars(int band, int barHeight);
  void CenterBars(int band, int barHeight);
  void WhitePeak(int band);
  void OutrunPeak(int band);
  void Waterfall(int bandValues[], int band);

  int amplitude = 27000;
  
  // FastLed globals
  static const int BrightnessSettings[];
  static CRGBPalette16 purplePalette;
  static CRGBPalette16 outRunPalette;
  static CRGBPalette16 greenBluePalette;
  static CRGBPalette16 heatPalette;

  // RGB LED matrix
  FastLED_NeoMatrix* matrix;
  CRGB matrixLeds[DeckLightLedCount];
  static const int BarWidth =  (DeckLightMatrixWidth / (NUM_BANDS - 1));
  int oldBarHeights[NUM_BANDS] = {0};
  byte peak[NUM_BANDS] = {0};
  uint8_t colorTimer = 0;
 
  // Theme settings
  static int themeIndex;
  static bool autoChangePatterns;
};