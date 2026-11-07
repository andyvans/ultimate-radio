#include "AudioOut.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

AudioOut::AudioOut(bool supportAac)
{
    _supportAac = supportAac;
    _currentChannel = 0;
    _pendingChannel = 0;
    _isStarted = false;
    _isPlaying = false;
    _usingDynamicChannels = false;
    _channels = nullptr;
    _channelCount = 0;
}

AudioOut::~AudioOut()
{
    _writeTask.remove();
    delete _copierPlay;
    delete _decodedOut;
    delete _volumeOut;
    delete _multiDecoder;
    delete _mtsDecoder;
    delete _aacDecoder;
    delete _mp3Decoder;
    delete _i2sOut;
    delete _urlStream;
}

void AudioOut::Setup(RadioConfig* config)
{
    Serial.println("=== Setting up AudioOut ===");

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

#if USE_AUDIO_LOGGING
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);
#endif   

    Serial.println("Creating buffered HLS stream...");
    _urlStream = new HLSStream();
    _buffer.setReadMaxWait(0);
    _buffer.setWriteMaxWait(0);
    if (!_buffer.resize(_supportAac ? kHlsBufferSize : kFallbackBufferSize))
    {
        Serial.println("Could not allocate HLS buffer");
        return;
    }

    Serial.println("Creating decoders...");
    _mp3Decoder = new MP3DecoderHelix();
    _aacDecoder = _supportAac ? new AACDecoderHelix() : nullptr;
    _mtsDecoder = _supportAac ? new MTSDecoder(*_aacDecoder) : nullptr;

    Serial.println("Creating MultiDecoder...");
    _multiDecoder = new MultiDecoder(*_urlStream);
    _multiDecoder->addDecoder(*_mp3Decoder, "audio/mp3");
    _multiDecoder->addDecoder(*_mp3Decoder, "audio/mpeg");
    if (_supportAac)
    {
        _multiDecoder->addDecoder(*_aacDecoder, "audio/aac");
        _multiDecoder->addDecoder(*_aacDecoder, "audio/aacp");
        _multiDecoder->addDecoder(*_mtsDecoder, "video/mp2t");
    }

    Serial.println("Creating I2S stream...");
    _i2sOut = new I2SStream();

    Serial.println("Configuring I2S output...");
    auto configOut = _i2sOut->defaultConfig(TX_MODE);
    configOut.pin_bck = I2S_BCLK_OUT;
    configOut.pin_ws = I2S_LRC_OUT;
    configOut.pin_data = I2S_DATA_OUT;

    Serial.println("Starting I2S stream...");
    _i2sOut->begin(configOut);

    Serial.println("Creating buffered playback pipeline...");
    _volumeOut = new VolumeStream(*_i2sOut);
    auto volumeConfig = _volumeOut->defaultConfig();
    volumeConfig.volume = config->volume;
    _volumeOut->begin(volumeConfig);
    _decodedOut = new EncodedAudioOutput(_volumeOut, _multiDecoder);
    _copierPlay = new StreamCopy(*_decodedOut, _queue);
    _copierPlay->resize(kPlayerCopyBufferSize);

    if (!_writeTask.begin([this]() { WriteLoop(); }))
    {
        Serial.println("Could not start HLS writer task");
        delete _copierPlay;
        _copierPlay = nullptr;
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
    _isStarted = true;
}

void AudioOut::Stop()
{
    _isStarted = false;
}

bool AudioOut::IsStarted()
{
    return _isStarted;
}

void AudioOut::Tick()
{
    if (_copierPlay == nullptr) return;
    if (_channels == nullptr || _channelCount <= 0) return;

    int target = _isStarted ? _pendingChannel : -1;
    if (target != _requestedChannel)
    {
        if (_requestedChannel >= 0) _decodedOut->end();

        // The writer only holds this lock while copying into the queue, never during network I/O
        _writeMutex.lock();
        _requestGen++;
        _requestedChannel = target;
        _queue.end();
        _buffer.reset();
        if (target >= 0) _queue.begin(100);
        _writeMutex.unlock();

        if (target >= 0)
        {
            _currentChannel = target;
            Serial.print("Starting channel: ");
            Serial.println(_channels[target].url);
            if (!_decodedOut->begin()) Serial.println("Could not start decoder");
            _copierPlay->begin();
        }
    }

    _isPlaying = _requestedChannel >= 0 && _streamChannel == _requestedChannel;
    if (_requestedChannel >= 0) _copierPlay->copy();
}

bool AudioOut::IsPlaying()
{
    return _isPlaying;
}

void AudioOut::WriteLoop()
{
    _writeMutex.lock();
    int channel = _requestedChannel;
    uint32_t gen = _requestGen;
    _writeMutex.unlock();

    if (channel != _streamChannel)
    {
        if (_streamChannel >= 0)
        {
            _urlStream->end();
            _streamChannel = -1;
        }
        if (channel < 0)
        {
            delay(10);
            return;
        }
        if (!_urlStream->begin(_channels[channel].url))
        {
            _urlStream->end();
            Serial.println("Audio start failed; retrying");
            delay(kRetryDelayMs);
            return;
        }
        _streamChannel = channel;
    }

    if (channel < 0 || _buffer.availableForWrite() < kUrlBufferSize)
    {
        delay(1);
        return;
    }

    size_t len = _urlStream->readBytes(_writeBuffer, kUrlBufferSize);
    if (len > 0)
    {
        _writeMutex.lock();
        // Drop data read for a channel that has since been changed or stopped
        if (gen == _requestGen)
        {
            _queue.write(_writeBuffer, len);
            // Writes stop with less than kUrlBufferSize free, so the queue's 100% threshold is never reached
            if (!_queue && _buffer.availableForWrite() < kUrlBufferSize) _queue.begin();
        }
        _writeMutex.unlock();
    }
    delay(1);
}