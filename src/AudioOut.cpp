#include "AudioOut.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

AudioOut::AudioOut(bool supportAac)
{
    _supportAac = supportAac;
    _volume = 0.5f;
    _streamKind = STREAM_KIND_DIRECT;
    _currentChannel = 0;
    _pendingChannel = 0;
    _mode = AUDIO_MODE_OFF;
    _isPlaying = false;
    _usingDynamicChannels = false;
    _channels = nullptr;
    _channelCount = 0;
    _urlStream = nullptr;
    _hlsStream = nullptr;
    _audioSourceUrl = nullptr;
    _i2sOut = nullptr;
    _mp3Decoder = nullptr;
    _aacDecoder = nullptr;
    _mtsDecoder = nullptr;
    _multiDecoder = nullptr;
    _audioPlayer = nullptr;
}

AudioOut::~AudioOut()
{
    DestroyPipeline();
    // Note: Dynamic channel memory is managed by RadioConfig
}

void AudioOut::Setup(RadioConfig* config)
{
    Serial.println("=== Setting up AudioOut ===");

    if (config == nullptr)
    {
        Serial.println("AudioOut setup failed: null config");
        _channels = nullptr;
        _channelCount = 0;
        return;
    }

    if (config->channels != nullptr && config->channelCount > 0)
    {
        _channels = config->channels;
        _channelCount = config->channelCount;
        Serial.print("Using ");
        Serial.print(_channelCount);
        Serial.println(" dynamically loaded channels");
    }
    else
    {
        Serial.println("No channels provided!");
        _channels = nullptr;
        _channelCount = 0;
    }
    if (config->defaultChannel >= 0 && config->defaultChannel < _channelCount)
    {
        _currentChannel = config->defaultChannel;
        _pendingChannel = config->defaultChannel;
    }

    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);
    _volume = config->volume;

    if (!BuildPipelineForChannel(_currentChannel))
    {
        Serial.println("AudioOut setup failed: could not build playback pipeline");
        return;
    }

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

const char* AudioOut::GetChannelName(int channel) const
{
    if (_channels == nullptr || channel < 0 || channel >= _channelCount) return nullptr;
    return _channels[channel].name;
}

void AudioOut::Start(int channel)
{
    if (_channels == nullptr || _channelCount <= 0) return;
    if (channel < 0) channel = 0;
    if (channel >= _channelCount) channel = _channelCount - 1;
    if (channel != _pendingChannel)
    {
        Serial.print("Changing pending audio to channel: ");
        Serial.println(_channels[channel].url);
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
    if (_channels == nullptr || _channelCount <= 0) return;

    if (_mode == AUDIO_MODE_OFF && _isPlaying)
    {
        Serial.println("Stopping audio playback");
        _audioPlayer->end();
        _isPlaying = false;
    }

    if (_pendingChannel != _currentChannel)
    {
        bool wasPlaying = _isPlaying;
        if (!BuildPipelineForChannel(_pendingChannel))
        {
            Serial.println("Channel switch failed: pipeline build error");
            return;
        }

        _currentChannel = _pendingChannel;
        Serial.print("Switched to channel: ");
        Serial.println(_channels[_currentChannel].url);

        if (wasPlaying && _mode == AUDIO_MODE_RADIO)
        {
            _isPlaying = _audioPlayer->begin(0);
            if (!_isPlaying)
            {
                Serial.println("Audio start failed after channel switch; will retry on next tick");
            }
        }
    }

    if (_mode == AUDIO_MODE_RADIO && !_isPlaying)
    {
        Serial.print("Starting channel: ");
        Serial.println(_channels[_currentChannel].url);
        _isPlaying = _audioPlayer->begin(0);
        if (!_isPlaying)
        {
            Serial.println("Audio start failed; will retry on next tick");
        }
    }

    _audioPlayer->copy();
}

bool AudioOut::IsPlaying()
{
    return _isPlaying && _audioPlayer != nullptr && _audioPlayer->isActive();
}

void AudioOut::HandleStreamChange(Stream* stream, void* reference)
{
    (void)stream;
    AudioOut* self = static_cast<AudioOut*>(reference);
    if (self != nullptr)
    {
        self->OnStreamChanged(stream);
    }
}

void AudioOut::OnStreamChanged(Stream* stream)
{
    if (stream == nullptr) return;
    // Pipeline is built per selected channel; callback can be used for diagnostics.
    (void)stream;
}

bool AudioOut::IsHlsUrl(const char* url)
{
    if (url == nullptr) return false;
    String u(url);
    u.toLowerCase();
    return u.indexOf(".m3u8") >= 0;
}

void AudioOut::DestroyPipeline()
{
    if (_audioPlayer != nullptr)
    {
        _audioPlayer->end();
    }
    if (_urlStream != nullptr)
    {
        _urlStream->end();
    }
    if (_hlsStream != nullptr)
    {
        _hlsStream->end();
    }
    delete _multiDecoder;
    _multiDecoder = nullptr;
    delete _mtsDecoder;
    _mtsDecoder = nullptr;
    delete _aacDecoder;
    _aacDecoder = nullptr;
    delete _mp3Decoder;
    _mp3Decoder = nullptr;
    delete _audioPlayer;
    _audioPlayer = nullptr;
    delete _audioSourceUrl;
    _audioSourceUrl = nullptr;
    delete _i2sOut;
    _i2sOut = nullptr;
    _isPlaying = false;
}

bool AudioOut::BuildPipelineForChannel(int channel)
{
    if (_channels == nullptr || _channelCount <= 0) return false;
    if (channel < 0 || channel >= _channelCount) return false;

    const char* url = _channels[channel].url;
    if (url == nullptr) return false;

    DestroyPipeline();

    bool useHls = IsHlsUrl(url);
    _streamKind = useHls ? STREAM_KIND_HLS : STREAM_KIND_DIRECT;

    if (useHls)
    {
        Serial.println("Building HLS playback pipeline");
        if (_hlsStream == nullptr)
        {
            _hlsStream = new HLSStream();
        }
        _hlsStream->setBufferSize(kUrlBufferSize, kUrlBufferCount);
        _audioSourceUrl = new AudioSourceDynamicURL(*_hlsStream, nullptr, 0);
    }
    else
    {
        Serial.println("Building direct URL playback pipeline");
        if (_urlStream == nullptr)
        {
            _urlStream = new URLStreamBuffered();
        }
        _urlStream->setBufferSize(kUrlBufferSize, kUrlBufferCount);
        _audioSourceUrl = new AudioSourceDynamicURL(*_urlStream, nullptr, 0);
    }

    _audioSourceUrl->setTimeoutAutoNext(60000);
    _audioSourceUrl->addURL(url);

    _mp3Decoder = new MP3DecoderHelix();
    _aacDecoder = _supportAac ? new AACDecoderHelix() : nullptr;
    if (useHls)
    {
        if (_aacDecoder == nullptr)
        {
            Serial.println("HLS AAC playback requires AAC decoder support");
            DestroyPipeline();
            return false;
        }
        _mtsDecoder = new MTSDecoder(*_aacDecoder);
    }

    if (useHls)
    {
        _multiDecoder = new MultiDecoder(*_hlsStream);
    }
    else
    {
        _multiDecoder = new MultiDecoder(*_urlStream);
    }

    _multiDecoder->addDecoder(*_mp3Decoder, "audio/mp3");
    _multiDecoder->addDecoder(*_mp3Decoder, "audio/mpeg");
    if (_supportAac)
    {
        _multiDecoder->addDecoder(*_aacDecoder, "audio/aac");
        _multiDecoder->addDecoder(*_aacDecoder, "audio/aacp");
    }
    if (useHls)
    {
        _multiDecoder->addDecoder(*_mtsDecoder, "video/mp2t");
    }

    _i2sOut = new I2SStream();
    auto configOut = _i2sOut->defaultConfig(TX_MODE);
    configOut.pin_bck = I2S_BCLK_OUT;
    configOut.pin_ws = I2S_LRC_OUT;
    configOut.pin_data = I2S_DATA_OUT;
    _i2sOut->begin(configOut);

    _audioPlayer = new AudioPlayer(*_audioSourceUrl, *_i2sOut, *_multiDecoder);
    _audioPlayer->setBufferSize(kPlayerCopyBufferSize);
    _audioPlayer->setReference(this);
    _audioPlayer->setOnStreamChangeCallback(HandleStreamChange);
    _audioPlayer->setVolume(_volume);

    return true;
}