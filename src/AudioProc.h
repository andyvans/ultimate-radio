#pragma once

#include "constants.h"
#include <Arduino.h>
#include <arduinoFFT.h>

#define AudioSamples 1024 // Must be a power of 2
#define AudioSamplingFreq 40000 // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define AudioNoise 500 // Used as a crude noise filter, values below this are ignored

typedef void (*tickCallbackFunction)(void);

class AudioProc
{
public:
  AudioProc();

  void Analyse(tickCallbackFunction tickCallback);
  int bandValues[NUM_BANDS] = {0};

private:
  void SampleAudio(tickCallbackFunction tickCallback);
  void AnalyseAudio();
    
  unsigned int sampling_period_us;  
  float vReal[AudioSamples] = {0};
  float vImag[AudioSamples] = {0};
  unsigned long newTime;
  ArduinoFFT<float> FFT;
};