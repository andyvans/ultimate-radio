#include "AudioOut.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "_Secrets.h"
#include <WiFi.h>

AudioOut::AudioOut()
{
    _currentChannel = 0;
    _pendingChannel = 0;
    _mode = AUDIO_MODE_OFF;
    _isPlaying = false;
    _usingDynamicChannels = false;
    _channels = nullptr;
    _channelCount = 0;
}

AudioOut::~AudioOut()
{
    // Note: Dynamic channel memory is managed by RadioConfig
}

void AudioOut::Setup(char** urls, int count, int defaultChannel)
{
    Serial.println("=== Setting up AudioOut ===");

    if (urls != nullptr && count > 0)
    {
        _channels = (const char**)urls;
        _channelCount = count;
        Serial.print("Using ");
        Serial.print(count);
        Serial.println(" dynamically loaded channels");
    }
    else
    {
        Serial.println("No channels provided!");
        _channels = nullptr;
        _channelCount = 0;
    }
    if (defaultChannel >= 0 && defaultChannel < _channelCount)
    {
        _currentChannel = defaultChannel;
        _pendingChannel = defaultChannel;
    }

    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);

    Serial.println("Creating URLStream (WiFi connecting)...");
    _urlStream = new URLStreamBuffered(WIFI_SSID, WIFI_PASSWORD);

    Serial.println("WiFi connected! Creating AudioSourceURL...");
    _audioSourceUrl = new AudioSourceDynamicURL(*_urlStream, "audio/mp3", _currentChannel);

    // Add all the URLs to the dynamic source
    for (int i = 0; i < _channelCount; i++)
    {
        _audioSourceUrl->addURL(_channels[i]);
    }

    Serial.println("Creating MP3 decoder...");
    _mp3Decoder = new MP3DecoderHelix();

    Serial.println("Creating I2S stream...");
    _i2sOut = new I2SStream();

    Serial.println("Configuring I2S output...");
    auto configOut = _i2sOut->defaultConfig(TX_MODE);
    configOut.pin_bck = I2S_BCLK_OUT;
    configOut.pin_ws = I2S_LRC_OUT;
    configOut.pin_data = I2S_DATA_OUT;

    Serial.println("Starting I2S stream...");
    _i2sOut->begin(configOut);

    Serial.println("Creating audio player...");
    _audioPlayer = new AudioPlayer(*_audioSourceUrl, *_i2sOut, *_mp3Decoder);

    Serial.println("=== AudioOut setup complete ===");
}

int AudioOut::GetChannelCount()
{
    return _channelCount;
}

int AudioOut::GetCurrentChannel()
{
    return _currentChannel;
}

void AudioOut::Start(int channel)
{
    if (channel < 0) channel = 0;
    if (channel >= _channelCount) channel = _channelCount - 1;
    if (channel != _pendingChannel)
    {
        Serial.print("Changing pending audio to channel: ");
        Serial.println(_channels[channel]);
        _pendingChannel = channel;
    }
    _mode = AUDIO_MODE_RADIO;
}

void AudioOut::Stop()
{
    _mode = AUDIO_MODE_OFF;
}

AudioMode AudioOut::GetMode()
{
    return _mode;
}

void AudioOut::Tick()
{
    if (_audioPlayer == nullptr) return;

    if (_mode == AUDIO_MODE_OFF && _isPlaying)
    {
        Serial.println("Stopping audio playback");
        _audioPlayer->end();
        _isPlaying = false;
    }

    if (_pendingChannel != _currentChannel)
    {
        _currentChannel = _pendingChannel;
        Serial.print("Switching to channel: ");
        Serial.println(_channels[_currentChannel]);
        _audioPlayer->setIndex(_currentChannel);
    }

    if (_mode == AUDIO_MODE_RADIO && !_isPlaying)
    {
        Serial.print("Starting channel: ");
        Serial.println(_channels[_currentChannel]);
        _audioPlayer->begin(_currentChannel);
        _isPlaying = true;
    }

    _audioPlayer->copy();
}