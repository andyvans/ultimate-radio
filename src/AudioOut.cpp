#include "AudioOut.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "_Secrets.h"
#include <WiFi.h>

// Define the array of radio channels
const char* AudioOut::channels[] = {
    "http://stream.srg-ssr.ch/m/rsj/mp3_128",
    "http://stream.srg-ssr.ch/m/rsc_de/mp3_128",
    "http://stream.srg-ssr.ch/m/couleur3/mp3_128",
    "http://stream.srg-ssr.ch/m/rr/mp3_128",
    "http://stream.srg-ssr.ch/m/rsj/mp3_128",
    "http://stream.srg-ssr.ch/m/rsc_de/mp3_128",
    "http://stream.srg-ssr.ch/m/couleur3/mp3_128",
    "http://stream.srg-ssr.ch/m/rr/mp3_128"
};

const int AudioOut::channelCount = sizeof(AudioOut::channels) / sizeof(AudioOut::channels[0]);

AudioOut::AudioOut()
{
    _currentChannel = 0;
    _mode = AUDIO_MODE_OFF;
    _isPlaying = false;
}

void AudioOut::Setup()
{
    Serial.println("=== Setting up AudioOut ===");
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);

    Serial.println("Creating URLStream (WiFi connecting)...");
    _urlStream = new URLStream(WIFI_SSID, WIFI_PASSWORD);

    Serial.println("WiFi connected! Creating AudioSourceURL...");
    _audioSourceUrl = new AudioSourceURL(*_urlStream, channels, "audio/mp3");

    Serial.println("Creating MP3 decoder...");
    _mp3Decoder = new MP3DecoderHelix();

    Serial.println("Creating I2S stream...");
    _i2sOut = new I2SStream();

    Serial.println("Configuring I2S output...");
    auto configOut = _i2sOut->defaultConfig(TX_MODE);
    configOut.port_no = 0;
    configOut.pin_bck = I2S_BCLK_OUT;
    configOut.pin_ws = I2S_LRC_OUT;
    configOut.pin_data = I2S_DATA_OUT;
    configOut.channels = 2;
    configOut.buffer_count = 16;  // Increase buffer count for smoother playback
    configOut.buffer_size = 512;  // Larger buffers to prevent underruns

    Serial.println("Starting I2S stream...");
    _i2sOut->begin(configOut);

    Serial.println("Creating audio player...");
    _audioPlayer = new AudioPlayer(*_audioSourceUrl, *_i2sOut, *_mp3Decoder);

    Serial.println("=== AudioOut setup complete ===");
}

int AudioOut::GetChannelCount()
{
    return channelCount;
}

void AudioOut::Start(int channel)
{
    if (channel < 0) channel = 0;
    if (channel >= channelCount) channel = channelCount - 1;
    if (channel != _pendingChannel)
    {
        Serial.print("Changing to channel: ");
        Serial.println(channel);
        _pendingChannel = channel;
    }
    _mode = AUDIO_MODE_RADIO;
}

void AudioOut::Stop()
{
    _mode = AUDIO_MODE_OFF;
}

void AudioOut::Tick()
{
    if (_mode == AUDIO_MODE_OFF && _isPlaying)
    {
        Serial.println("Stopping audio playback");
        _audioPlayer->end();
        _isPlaying = false;
        return;
    }
    else if (_mode == AUDIO_MODE_RADIO && !_isPlaying)
    {
        Serial.println("Starting radio stream");
        _audioPlayer->begin();
        _isPlaying = true;
        return;
    }

    if (_audioPlayer != nullptr && _pendingChannel != _currentChannel)
    {
        Serial.print("Switching to new channel: ");
        Serial.println(_pendingChannel);
        _currentChannel = _pendingChannel;
        _audioPlayer->setIndex(_currentChannel);
    }

    if (_audioPlayer != nullptr) _audioPlayer->copy();
}