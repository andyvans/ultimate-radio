#pragma once
#include <Arduino.h>
#include "Constants.h"
#include <AudioTools.h>
#include <AudioTools/Communication/AudioHttp.h>

using namespace audio_tools;

struct RadioChannel
{
    const char* url;
    const char* mimeType;
};

class AudioOut
{
public:
    AudioOut();
    void Setup();
    void StartRadio(int channel = 0);
    void Stop();
    void Tick();
    int GetChannelCount();

private:
    AudioInfo* infoFrom;
    URLStream* urlStream;
    ResampleStream* resampler;
    EncodedAudioStream* decodedStream;

    AudioStream* in;
    I2SStream* out;
    StreamCopy* copier;

    static const RadioChannel channels[];
    static const int channelCount;
};