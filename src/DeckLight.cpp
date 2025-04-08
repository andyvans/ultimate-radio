#include <Arduino.h>
#include "DeckLight.h"
#include "ColourPalettes.h"

// Static member initialisation
CRGBPalette16 DeckLight::purplePalette = PurpleGlobalPalette;
CRGBPalette16 DeckLight::outRunPalette = OutRunGlobalPalette;
CRGBPalette16 DeckLight::greenBluePalette = GreenBlueGlobalPalette;
CRGBPalette16 DeckLight::heatPalette = RedYellowGlobalPalette;

int DeckLight::themeIndex = 0;
bool DeckLight::autoChangePatterns = false;
const int DeckLight::BrightnessSettings[] = {60, 30, 10};

DeckLight::DeckLight() : 
  modeBtn(ROTARY_ENCODER_1_SW_PIN)
{
  matrix = new FastLED_NeoMatrix(matrixLeds, DeckLightMatrixWidth, DeckLightMatrixHeight, 
    NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG + NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS);
}

void DeckLight::Setup()
{
  modeBtn.attachClick([](void *scope){
    ((DeckLight*)scope)->ChangeTheme();
  }, this);
  modeBtn.attachLongPressStart([](void *scope){
    ((DeckLight*)scope)->ChangeBrightness();
    }, this);
  modeBtn.attachDoubleClick([](void *scope){
    ((DeckLight*)scope)->StartAutoMode();
  }, this);

  // Careful with the amount of power here if running off USB port
  const int MaxMilliAmps = 400; 

  // Setup LED matrix
  // WS2812B is the chip type, GRB is the colour order
  FastLED.addLeds<WS2812B, LED_MATRIX_PIN, EOrder::GRB>(matrixLeds, DeckLightLedCount).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTS, MaxMilliAmps);
  FastLED.setBrightness(BrightnessSettings[1]);
  FastLED.clear();
}

void DeckLight::Tick()
{
  modeBtn.tick();
}

void DeckLight::DisplayAudio(int bandValues[])
{
  // Don't clear screen if waterfall pattern
  if (themeIndex != 5)
    FastLED.clear();

  // Update the bar vu meter
  UpdateBarVuMeter(bandValues);

  // Update the analog vu meter
  UpdateAnalogVuMeter();

  // Decay peak
  EVERY_N_MILLISECONDS(60)
  {
    for (byte band = 0; band < NUM_BANDS; band++)
    {
      if (peak[band] > 0)
        peak[band] -= 1;
    }
    colorTimer++;
  }

  // Used in some of the patterns
  EVERY_N_MILLISECONDS(10)
  {
    colorTimer++;
  }

  EVERY_N_SECONDS(10)
  {
    if (autoChangePatterns)
      themeIndex = (themeIndex + 1) % 6;
  }

  FastLED.show();
}

void DeckLight::UpdateBarVuMeter(int bandValues[])
{
  // Process the FFT data into bar heights
  for (byte band = 0; band < NUM_BANDS; band++)
  {
    // Scale the bars for the display
    int barHeight = bandValues[band] / amplitude;

    // Small amount of averaging between frames
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

    if (barHeight > DeckLightMatrixHeight)
      barHeight = DeckLightMatrixHeight;

    // Move peak up
    if (barHeight > peak[band])
    {
      peak[band] = min(DeckLightMatrixHeight, barHeight);
    }

    // Draw bars
    switch (themeIndex)
    {
    case 0:
      RainbowBars(band, barHeight);
      break;
    case 1:
      // No bars on this one
      break;
    case 2:
      PurpleBars(band, barHeight);
      break;
    case 3:
      CenterBars(band, barHeight);
      break;
    case 4:
      ChangingBars(band, barHeight);
      break;
    case 5:
      Waterfall(bandValues, band);
      break;
    }

    // Draw peaks
    switch (themeIndex)
    {
    case 0:
      WhitePeak(band);
      break;
    case 1:
      OutrunPeak(band);
      break;
    case 2:
      WhitePeak(band);
      break;
    case 3:
      // No peaks
      break;
    case 4:
      // No peaks
      break;
    case 5:
      // No peaks
      break;
    }

    // Save oldBarHeights for averaging later
    oldBarHeights[band] = barHeight;
  }
}

void DeckLight::UpdateAnalogVuMeter()
{
  // Update analog vu meter. Average the peaks and map the output to the meter
  // Exclude the highest band as it's usually too high
  int vuPeak = 0;
  const int bandCount = NUM_BANDS - 1;
  for (byte band = 0; band < bandCount; band++)
  {
    vuPeak += oldBarHeights[band];
  }

  vuPeak = (vuPeak / bandCount) * AnalogVUPeakScalar;
  int vuUnits = map(vuPeak, 0, DeckLightMatrixHeight, 0, 255);

  // Limit the output to protect the analog VU meters
  vuUnits = min(vuUnits, AnalogVUMax);

  

  // Update the desk leds
  switch (themeIndex)
  {
  case 0:
    vuLeds[0] = CHSV(vuUnits, 255, 255);
    vuLeds[1] = CHSV(vuUnits, 255, 255);
    vuLeds[2] = CHSV(vuUnits, 255, 255);
    vuLeds[3] = CHSV(vuUnits, 255, 255);
    vuLeds[4] = CHSV(vuUnits, 255, 255);
    break;
  case 1:
    vuLeds[0] = ColorFromPalette(outRunPalette, vuUnits);
    vuLeds[1] = ColorFromPalette(outRunPalette, vuUnits);
    vuLeds[2] = ColorFromPalette(outRunPalette, vuUnits);
    vuLeds[3] = ColorFromPalette(outRunPalette, vuUnits);
    vuLeds[4] = ColorFromPalette(outRunPalette, vuUnits);
    break;
  case 2:
    vuLeds[0] = ColorFromPalette(purplePalette, vuUnits);
    vuLeds[1] = ColorFromPalette(purplePalette, vuUnits);
    vuLeds[2] = ColorFromPalette(purplePalette, vuUnits);
    vuLeds[3] = ColorFromPalette(purplePalette, vuUnits);
    vuLeds[4] = ColorFromPalette(purplePalette, vuUnits);
    break;
  case 3:
    vuLeds[0] = ColorFromPalette(heatPalette, vuUnits);
    vuLeds[1] = ColorFromPalette(heatPalette, vuUnits);
    vuLeds[2] = ColorFromPalette(heatPalette, vuUnits);
    vuLeds[3] = ColorFromPalette(heatPalette, vuUnits);
    vuLeds[4] = ColorFromPalette(heatPalette, vuUnits);
    break;
  case 4:
    vuLeds[0] = CHSV(vuUnits, 255, 255);
    vuLeds[1] = CHSV(vuUnits, 255, 255);
    vuLeds[2] = CHSV(vuUnits, 255, 255);
    vuLeds[3] = CHSV(vuUnits, 255, 255);
    vuLeds[4] = CHSV(vuUnits, 255, 255);
    break;
  case 5:
    vuLeds[0] = CHSV(vuUnits, 255, 255);
    vuLeds[1] = CHSV(vuUnits, 255, 255);
    vuLeds[2] = CHSV(vuUnits, 255, 255);
    vuLeds[3] = CHSV(vuUnits, 255, 255);
    vuLeds[4] = CHSV(vuUnits, 255, 255);
    break;
  }

  EVERY_N_MILLISECONDS(500)
  {
    activeDeckLed = (activeDeckLed + 1) % 3;
  }

  // Draw the active deck led as white
  vuLeds[activeDeckLed] = CRGB::White;
}

void DeckLight::ChangeTheme()
{
  if (FastLED.getBrightness() == 0)
    FastLED.setBrightness(BrightnessSettings[0]); // Re-enable if lights are "off"
  autoChangePatterns = false;
  themeIndex = (themeIndex + 1) % 6;
}

void DeckLight::StartAutoMode()
{
  autoChangePatterns = true;
}

void DeckLight::ChangeBrightness()
{
  if (FastLED.getBrightness() == BrightnessSettings[2])
    FastLED.setBrightness(BrightnessSettings[0]);
  else if (FastLED.getBrightness() == BrightnessSettings[0])
    FastLED.setBrightness(BrightnessSettings[1]);
  else if (FastLED.getBrightness() == BrightnessSettings[1])
    FastLED.setBrightness(BrightnessSettings[2]);
  else if (FastLED.getBrightness() == 0)
    FastLED.setBrightness(BrightnessSettings[0]); // Re-enable if lights are "off"
}

void DeckLight::BrightnessOff()
{
  FastLED.setBrightness(0); // Lights out
}

// Patterns
void DeckLight::RainbowBars(int band, int barHeight)
{
  int xStart = BarWidth * band;
  for (int x = xStart; x < xStart + BarWidth; x++)
  {
    for (int y = DeckLightMatrixHeight; y >= DeckLightMatrixHeight - barHeight; y--)
    {
      matrix->drawPixel(x, y, CHSV((x / BarWidth) * (255 / NUM_BANDS), 255, 255));
    }
  }
}

void DeckLight::PurpleBars(int band, int barHeight)
{
  int xStart = BarWidth * band;
  for (int x = xStart; x < xStart + BarWidth; x++)
  {
    for (int y = DeckLightMatrixHeight; y >= DeckLightMatrixHeight - barHeight; y--)
    {
      matrix->drawPixel(x, y, ColorFromPalette(purplePalette, y * (255 / (barHeight + 1))));
    }
  }
}

void DeckLight::ChangingBars(int band, int barHeight)
{
  int xStart = BarWidth * band;
  for (int x = xStart; x < xStart + BarWidth; x++)
  {
    for (int y = DeckLightMatrixHeight; y >= DeckLightMatrixHeight - barHeight; y--)
    {
      matrix->drawPixel(x, y, CHSV(y * (255 / DeckLightMatrixHeight) + colorTimer, 255, 255));
    }
  }
}

void DeckLight::CenterBars(int band, int barHeight)
{
  int xStart = BarWidth * band;
  for (int x = xStart; x < xStart + BarWidth; x++)
  {
    if (barHeight % 2 == 0)
      barHeight--;
    int yStart = ((DeckLightMatrixHeight - barHeight) / 2);
    for (int y = yStart; y <= (yStart + barHeight); y++)
    {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      matrix->drawPixel(x, y, ColorFromPalette(heatPalette, colorIndex));
    }
  }
}

void DeckLight::WhitePeak(int band)
{
  int xStart = BarWidth * band;
  int peakHeight = DeckLightMatrixHeight - peak[band] - 1;
  for (int x = xStart; x < xStart + BarWidth; x++)
  {
    matrix->drawPixel(x, peakHeight, CHSV(0, 0, 255));
  }
}

void DeckLight::OutrunPeak(int band)
{
  int xStart = BarWidth * band;
  int peakHeight = DeckLightMatrixHeight - peak[band] - 1;
  for (int x = xStart; x < xStart + BarWidth; x++)
  {
    matrix->drawPixel(x, peakHeight, ColorFromPalette(outRunPalette, peakHeight * (255 / DeckLightMatrixHeight)));
  }
}

void DeckLight::Waterfall(int bandValues[], int band)
{
  int xStart = BarWidth * band;
  double highestBandValue = 60000; // Set this to calibrate your waterfall

  // Draw bottom line
  for (int x = xStart; x < xStart + BarWidth; x++)
  {
    matrix->drawPixel(x, 0, CHSV(constrain(map(bandValues[band], 0, highestBandValue, 160, 0), 0, 160), 255, 255));
  }

  // Move screen up starting at 2nd row from top
  if (band == NUM_BANDS - 1)
  {
    for (int y = DeckLightMatrixHeight - 2; y >= 0; y--)
    {
      for (int x = 0; x < DeckLightMatrixWidth; x++)
      {
        int pixelIndexY = matrix->XY(x, y + 1);
        int pixelIndex = matrix->XY(x, y);
        matrixLeds[pixelIndexY] = matrixLeds[pixelIndex];
      }
    }
  }
}
