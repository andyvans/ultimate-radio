#include "AudioOut.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "_Secrets.h"
#include <WiFi.h>

// Define the array of radio channels
const char* AudioOut::channels[] = {
    "https://stream.srg-ssr.ch/srgssr/rsc_de/mp3/128",
    "https://stream.srg-ssr.ch/m/couleur3/mp3_128",
    "https://stream.srg-ssr.ch/m/rsj/mp3_128",
    "http://bigriver.broadcast.co.nz/bigriverfm.mp3",
    "https://streaming.brol.tech/rtfmlounge",
    "https://live1.lankaradio.com:8010/128kbps.mp3",
    "https://s1-webradio.antenne.de/top-40",
    "http://streaming.swisstxt.ch/m/drsvirus/mp3_128",
    "http://hip-hop.channel.whff.radio:8046/stream"
};

#define DEFAULT_CHANNEL 2

const int AudioOut::channelCount = sizeof(AudioOut::channels) / sizeof(AudioOut::channels[0]);

AudioOut::AudioOut()
{
    _currentChannel = DEFAULT_CHANNEL;
    _pendingChannel = DEFAULT_CHANNEL;
    _mode = AUDIO_MODE_OFF;
    _isPlaying = false;
}

void AudioOut::Setup()
{
    Serial.println("=== Setting up AudioOut ===");
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);

    Serial.println("Creating URLStream (WiFi connecting)...");
    _urlStream = new URLStreamBuffered(WIFI_SSID, WIFI_PASSWORD);

    Serial.println("WiFi connected! Creating AudioSourceURL...");
    _audioSourceUrl = new AudioSourceURL(*_urlStream, channels, "audio/mp3", _currentChannel);

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
    return channelCount;
}

int AudioOut::GetCurrentChannel()
{
    return _currentChannel;
}

void AudioOut::Start(int channel)
{
    if (channel < 0) channel = 0;
    if (channel >= channelCount) channel = channelCount - 1;
    if (channel != _pendingChannel)
    {
        Serial.print("Changing pending audio to channel: ");
        Serial.println(channels[channel]);
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
        Serial.println(channels[_currentChannel]);
        _audioPlayer->end();
        _urlStream->flush();
        _audioPlayer->begin(_currentChannel);
    }

    if (_mode == AUDIO_MODE_RADIO && !_isPlaying)
    {
        Serial.print("Starting channel: ");
        Serial.println(channels[_currentChannel]);
        _audioPlayer->begin(_currentChannel);
        _isPlaying = true;
    }

    _audioPlayer->copy();
}