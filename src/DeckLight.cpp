#include <Arduino.h>
#include "DeckLight.h"
#include "ColourPalettes.h"

// Static member initialisation
CRGBPalette16 DeckLight::purplePalette = PurpleGlobalPalette;

DeckLight::DeckLight()
{
  matrix = new FastLED_NeoMatrix(matrixLeds, DeckLightMatrixWidth, DeckLightMatrixHeight,
    NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS);
}

void DeckLight::Setup()
{
  Serial.println("Setting up DeckLight");

  // Careful with the amount of power here if running off USB port
  const int MaxMilliAmps = 400;
  const int MaxVolts = 5;

  // Setup LED matrix
  // WS2812B is the chip type, GRB is the colour order
  FastLED.addLeds<WS2812B, LED_MATRIX_PIN, EOrder::GRB>(matrixLeds, DeckLightLedCount).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(MaxVolts, MaxMilliAmps);
  FastLED.setBrightness(30);
  FastLED.clear();
}

void DeckLight::DisplayLine(int index)
{
  FastLED.clear();

  for (int y = 0; y < DeckLightMatrixHeight; y++)
  {
    matrix->drawPixel(index, y, ColorFromPalette(purplePalette, 200));
  }

  // Enable indicators (these overlap with bars but that's ok)
  matrix->drawPixel(0, 1, CHSV(0, 0, 255));
  matrix->drawPixel(DeckLightMatrixWidth - 1, 1, CHSV(0, 0, 255));

  FastLED.show();
}