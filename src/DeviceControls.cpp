#include "DeviceControls.h"
#include "AudioOut.h"

DeviceControls::DeviceControls() :
    _audioOut(nullptr),
    _currentChannel(0),
    _pendingChannel(-1),
    _lastPositionChangeTime(0),
    _hasPendingChange(false),
    _encoder(nullptr)
{
}

void DeviceControls::Setup(AudioOut* audioOut, DeckLight* deckLight)
{
    _audioOut = audioOut;
    _deckLight = deckLight;

    Serial.println("=== Setting up DeviceControls ===");

    // Max value is maxChannels - 1 since we're using 0-based indexing
    _currentChannel = _audioOut != nullptr ? _audioOut->GetCurrentChannel() : 0;
    int maxChannel = _audioOut != nullptr ? _audioOut->GetChannelCount() - 1 : 0;

    _encoder = new OneRotaryEncoder(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_PIN_SWITCH);
    _encoder->SetRange(0, maxChannel, 4, _currentChannel);

    // Set bluetooth amp pin to input with pull-down (reads LOW when floating, HIGH when driven to 3.3V)
    pinMode(AMP_BLUETOOTH_PIN, INPUT_PULLDOWN);

    // Start first radio channel by default
    Serial.print("Starting initial channel: ");
    Serial.println(_currentChannel);
    _deckLight->DisplayLine(_currentChannel);
    _audioOut->Start(_currentChannel);
}

void DeviceControls::Tick()
{
    if (_encoder == nullptr) return;
    if (_audioOut == nullptr) return;

    // Check if in bluetooth mode
    if (digitalRead(AMP_BLUETOOTH_PIN) == HIGH)
    {
        // Bluetooth mode active - stop audio
        if (_audioOut->GetMode() == AUDIO_MODE_RADIO)
        {
            Serial.println("Bluetooth mode active - stopping audio");
            _audioOut->Stop();
            _deckLight->DrawBluetoothBar();
        }
        return;
    }

    if (_audioOut->GetMode() == AUDIO_MODE_OFF) {
        Serial.println("Exiting bluetooth mode - starting audio");
        _deckLight->DisplayLine(_currentChannel);
        _audioOut->Start(_currentChannel);
    }

    _encoder->Tick();

    // Check for position changes
    EncoderPositionState posState = _encoder->GetPosition();
    if (posState.hasNewPosition && posState.position != _pendingChannel)
    {
        // New position detected - update pending channel and reset timer
        _pendingChannel = posState.position;
        _lastPositionChangeTime = millis();
        _hasPendingChange = true;
        _deckLight->DisplayLine(_pendingChannel);
    }

    // Debounce check if we should apply the pending channel change
    if (_hasPendingChange && (millis() - _lastPositionChangeTime >= ChannelChangeDelayMs))
    {
        _currentChannel = _pendingChannel;
        _hasPendingChange = false;
        _deckLight->DisplayLine(_currentChannel);
        _audioOut->Start(_currentChannel);
    }

    // Check for button presses
    EncoderSwitchState switchState = _encoder->GetSwitchState();
    if (switchState.hasNewState)
    {
        switch (switchState.state)
        {
        case EncoderSwitchPress::Clicked:
            Serial.println("Encoder button clicked");
            break;
        case EncoderSwitchPress::DoubleClicked:
            Serial.println("Encoder button double-clicked");
            break;
        case EncoderSwitchPress::LongPressed:
            Serial.println("Encoder button long-pressed");
            break;
        default:
            break;
        }
    }
}
