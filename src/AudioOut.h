#pragma once
#include <Arduino.h>
#include "Constants.h"

// Audio-Tools: Buffer sizes adjusted for memory constraints
#define DEFAULT_BUFFER_SIZE 1536 // Default is 1024
#define I2S_BUFFER_SIZE 512 // Default is 512
#define I2S_BUFFER_COUNT 8 // Default is 6
#define USE_AUDIO_LOGGING false

#include <AudioTools.h>
#include <AudioTools/Communication/AudioHttp.h>
#include <AudioTools/Disk/AudioSourceURL.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioCodecs/CodecAACHelix.h>
#include <AudioTools/AudioCodecs/MultiDecoder.h>
#include <AudioTools/Communication/HLSStream.h>
#include <AudioTools/AudioCodecs/CodecMTS.h>
#include <AudioTools/Concurrency/RTOS.h>
#include "ConfigLoader.h"

using namespace audio_tools;

class AudioOut
{
public:
    static constexpr int kUrlBufferSize = 1024;
    static constexpr int kHlsBufferSize = 50 * 1024;
    static constexpr int kFallbackBufferSize = 10 * 1024;
    static constexpr int kPlayerCopyBufferSize = 4096;
    static constexpr int kRetryDelayMs = 2000;

    AudioOut(bool supportAac);
    ~AudioOut();
    void Setup(RadioConfig* config);
    void Stop();
    void Start(int channel);
    void Tick();
    int GetChannelCount();
    int GetCurrentChannel();
    const char* GetChannelName(int channel) const;
    bool IsStarted();
    bool IsPlaying();

private:
    volatile bool _isStarted;
    volatile int _currentChannel;
    volatile int _pendingChannel;
    volatile bool _isPlaying;
    bool _usingDynamicChannels;
    bool _supportAac;

    HLSStream* _urlStream = nullptr;
    I2SStream* _i2sOut = nullptr;
    MP3DecoderHelix* _mp3Decoder = nullptr;
    AACDecoderHelix* _aacDecoder = nullptr;
    MTSDecoder* _mtsDecoder = nullptr;
    MultiDecoder* _multiDecoder = nullptr;
    VolumeStream* _volumeOut = nullptr;
    EncodedAudioOutput* _decodedOut = nullptr;
    BufferRTOS<uint8_t> _buffer{0};
    QueueStream<uint8_t> _queue{_buffer};
    StreamCopy* _copierPlay = nullptr;
    Task _writeTask{"hls-write", 8 * 1024, 10, 0};
    MutexRTOS _writeMutex;
    uint8_t _writeBuffer[kUrlBufferSize];
    // Channel requested by Tick(); -1 means stopped
    volatile int _requestedChannel = -1;
    // Channel the writer task has open; -1 means none
    volatile int _streamChannel = -1;
    // Incremented on every request so the writer can drop stale data
    volatile uint32_t _requestGen = 0;

    // Channel storage
    ChannelConfig* _channels;
    int _channelCount;
    static const int _defaultChannelCount;

    void WriteLoop();
};