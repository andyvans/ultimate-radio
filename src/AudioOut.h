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

enum AudioMode
{
    AUDIO_MODE_OFF,
    AUDIO_MODE_RADIO,
};

class AudioOut
{
public:
    AudioOut();
    void Setup();
    void StopAudio();
    void StartRadio(int channel);
    void Tick();
    int GetChannelCount();

private:
    void StartRadioStream();
    void Stop();

    AudioMode _mode;
    int _currentChannel;
    bool _isPlaying;

    AudioInfo* _infoFrom;
    URLStream* _urlStream;
    ResampleStream* _resampler;
    EncodedAudioStream* _decodedStream;
    AudioStream* _in;
    I2SStream* _out;
    StreamCopy* _copier;

    static const RadioChannel channels[];
    static const int channelCount;
};