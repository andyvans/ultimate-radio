#include "AudioOut.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "_Secrets.h"
#include <WiFi.h>

// Define the array of radio channels
const char* AudioOut::channels[] = {
    "http://stream.srg-ssr.ch/m/rsj/mp3_128",
    "http://stream.srg-ssr.ch/m/rsc_de/mp3_128",
    "http://stream.srg-ssr.ch/m/couleur3/mp3_128",
    "http://stream.srg-ssr.ch/m/rr/mp3_128"
};

const int AudioOut::channelCount = sizeof(AudioOut::channels) / sizeof(AudioOut::channels[0]);

AudioOut::AudioOut()
{
    _currentChannel = -1;
    _mode = AUDIO_MODE_OFF;
    _isPlaying = false;
}

void AudioOut::Setup()
{
    Serial.println("Setting up AudioOut");
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);

    _urlStream = new URLStream(WIFI_SSID, WIFI_PASSWORD);
    _audioSourceUrl = new AudioSourceURL(*_urlStream, channels, "audio/mp3");
    _mp3Decoder = new MP3DecoderHelix();
    _i2sOut = new I2SStream();

    auto configOut = _i2sOut->defaultConfig(TX_MODE);
    configOut.port_no = 0;
    configOut.pin_bck = I2S_BCLK_OUT;
    configOut.pin_ws = I2S_LRC_OUT;
    configOut.pin_data = I2S_DATA_OUT;
    configOut.channels = 2;

    _i2sOut->begin(configOut);
    _audioPlayer = new AudioPlayer(*_audioSourceUrl, *_i2sOut, *_mp3Decoder);
}

int AudioOut::GetChannelCount()
{
    return channelCount;
}

void AudioOut::Start(int channel)
{    
    if (channel < 0) channel = 0;
    if (channel >= channelCount) channel = channelCount - 1;
    if (channel != _currentChannel)
    {
        Serial.print("Changing to channel: ");
        Serial.println(_currentChannel);
        _currentChannel = channel;
        _audioPlayer->setIndex(_currentChannel);
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

    if (_audioPlayer != nullptr)_audioPlayer->copy();
}