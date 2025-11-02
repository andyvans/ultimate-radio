#include "AudioOut.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "_Secrets.h"

// Define the array of radio channels
const RadioChannel AudioOut::channels[] = {
    {"http://stream.srg-ssr.ch/m/rsj/mp3_128", "audio/mp3"},
    {"http://stream.srg-ssr.ch/m/rsc_de/mp3_128", "audio/mp3"},
    {"http://jazz-wr11.ice.infomaniak.ch/jazz-wr11-128.mp3", "audio/mp3"},
    {"http://stream.srg-ssr.ch/m/drsvirus/mp3_128", "audio/mp3"},
    {"https://radionz-ice.streamguys.com/national.mp3", "audio/mp3"},
    {"http://stream.srg-ssr.ch/m/couleur3/mp3_128", "audio/mp3"},
    {"http://stream.srg-ssr.ch/m/rr/mp3_128", "audio/mp3"},
    {"http://sc2.radiocaroline.net/;?type=http&nocache=3741", "audio/mp3"}
};

const int AudioOut::channelCount = sizeof(AudioOut::channels) / sizeof(AudioOut::channels[0]);

AudioOut::AudioOut()
{
}

void AudioOut::Setup()
{
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);
}

int AudioOut::GetChannelCount()
{
    return channelCount;
}

void AudioOut::Stop()
{
    isPlaying = false;
    if (copier != nullptr)
    {
        copier->end();
        delete copier;
        copier = nullptr;
    }
    if (in != nullptr)
    {
        in->end();
        delete in;
        in = nullptr;
    }
    if (out != nullptr)
    {
        out->end();
        delete out;
        out = nullptr;
    }

    if (infoFrom != nullptr)
    {
        delete infoFrom;
        infoFrom = nullptr;
    }
}

void AudioOut::StartRadio(int channel)
{
    // Validate channel index
    if (channel < 0 || channel >= channelCount)
    {
        Serial.print("Invalid channel index: ");
        Serial.println(channel);
        channel = 0; // Default to first channel
    }

    infoFrom = new AudioInfo(44100, 1, 16);

    // start I2S output
    Serial.println("Starting I2S audio out");
    out = new I2SStream();
    auto configOut = out->defaultConfig(TX_MODE);
    configOut.copyFrom(*infoFrom);
    configOut.port_no = 1;
    //configOut.pin_bck = I2S_BCLK_OUT;
    //configOut.pin_ws = I2S_LRC_OUT;
    //configOut.pin_data = I2S_DATA_OUT;
    //configOut.channels = 1;

    out->begin(configOut);

    Serial.print("Starting radio stream (");
    Serial.print(channel);
    Serial.print(") ");
    Serial.println(channels[channel].url);

    urlStream = new URLStream(WIFI_SSID, WIFI_PASSWORD);
    resampler = new ResampleStream(*out);
    decodedStream = new EncodedAudioStream(resampler, new MP3DecoderHelix());
    copier = new StreamCopy(*decodedStream, *urlStream);

    urlStream->begin(channels[channel].url, channels[channel].mimeType);
    decodedStream->begin();
    isPlaying = true;
}

void AudioOut::Tick()
{
    if (copier == nullptr || !isPlaying) return;
    copier->copy();
}