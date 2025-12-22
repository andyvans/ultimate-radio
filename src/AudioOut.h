#pragma once
#include <Arduino.h>
#include "Constants.h"

// Audio-Tools: Buffer sizes adjusted for memory constraints
#define DEFAULT_BUFFER_SIZE 1536 // Default is 1024
#define I2S_BUFFER_SIZE 512 // Default is 512
#define I2S_BUFFER_COUNT 8 // Default is 6

#include <AudioTools.h>
#include <AudioTools/Communication/AudioHttp.h>
#include <AudioTools/Disk/AudioSourceURL.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>

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
    ~AudioOut();
    void Setup(char** urls, int count, int defaultChannel);
    void Stop();
    void Start(int channel);
    void Tick();
    int GetChannelCount();
    int GetCurrentChannel();
    AudioMode GetMode();

private:
    AudioMode _mode;
    int _currentChannel;
    int _pendingChannel;
    bool _isPlaying;
    bool _usingDynamicChannels;

    URLStreamBuffered* _urlStream;
    AudioSourceDynamicURL* _audioSourceUrl;
    I2SStream* _i2sOut;
    MP3DecoderHelix* _mp3Decoder;
    AudioPlayer* _audioPlayer;

    // Channel storage
    const char** _channels;
    int _channelCount;
    static const int _defaultChannelCount;
};