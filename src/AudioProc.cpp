#include "AudioProc.h"

AudioProc::AudioProc()
{
  sampling_period_us = round(1000000 * (1.0 / AudioSamplingFreq));
  FFT = ArduinoFFT<float>(vReal, vImag, AudioSamples, AudioSamplingFreq);
}

void AudioProc::Analyse(tickCallbackFunction tickCallback)
{
  SampleAudio(tickCallback);
  AnalyseAudio();
}

void AudioProc::SampleAudio(tickCallbackFunction tickCallback)
{
  // Sample the audio pin
  for (int i = 0; i < AudioSamples; i++)
  {
    newTime = micros();
    vReal[i] = analogRead(AUDIO_IN_PIN); // A conversion takes about 9.7uS on an ESP32
    vImag[i] = 0;
    while ((micros() - newTime) < sampling_period_us)
    { 
      // Wait for more audio. Use tick callback to process hardware events
      tickCallback();
    }
  }
}

void AudioProc::AnalyseAudio()
{
  // Reset bandValues[]
  for (int i = 0; i < NUM_BANDS; i++)
  {
    bandValues[i] = 0;
  }
  
  // Compute FFT
  FFT.dcRemoval();
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward); // Weigh data
  FFT.compute(FFTDirection::Forward); // Compute FFT
  FFT.complexToMagnitude(); // Compute magnitudes

  // Analyse FFT results
  for (int i = 2; i < (AudioSamples / 2); i++)
  {
    // Don't use sample 0 and only first SAMPLES/2 are usable
    if (vReal[i] > AudioNoise)
    {
      // 4 bands
      if (i <= 13)
        bandValues[0] += (int)vReal[i];
      if (i > 13 && i <= 27)
        bandValues[1] += (int)vReal[i];
      if (i > 27 && i <= 55)
        bandValues[2] += (int)vReal[i];
      if (i > 55 && i <= 255)
        bandValues[3] += (int)vReal[i]*0.5;
    }
  }
}