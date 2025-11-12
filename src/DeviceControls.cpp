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
    _currentChannel = 0;

    Serial.println("Setting up DeviceControls");

    // Create rotary encoder instance
    // Max value is maxChannels - 1 since we're using 0-based indexing
    int initialChannel = 0;
    int maxChannel = _audioOut != nullptr ? _audioOut->GetChannelCount() - 1 : 0;

    /*
    _encoder = new OneRotaryEncoder(
        ENCODER_PIN_A,
        ENCODER_PIN_B,
        ENCODER_PIN_SWITCH,
        initialChannel, // Set initial value
        0, // Set min value
        maxChannel // Set max value
    );
    */
    // Start first radio channel by default
    Serial.print("Starting initial channel: ");
    Serial.println(initialChannel);
    _audioOut->Start(initialChannel);
}

void DeviceControls::Tick()
{
    if (_encoder == nullptr) return;

    _encoder->Tick();

    // Check for position changes
    EncoderPositionState posState = _encoder->GetPosition();
    if (posState.hasNewPosition && posState.position != _currentChannel)
    {
        // New position detected - update pending channel and reset timer
        _pendingChannel = posState.position;
        _lastPositionChangeTime = millis();
        _hasPendingChange = true;
        _deckLight->DisplayLine(_pendingChannel);
    }

    // Check if we should apply the pending channel change (500ms stability)
    if (_hasPendingChange && (millis() - _lastPositionChangeTime >= 500))
    {
        _currentChannel = _pendingChannel;
        _hasPendingChange = false;
        ChangeChannel(_currentChannel);
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

void DeviceControls::ChangeChannel(int channel)
{
    if (_audioOut == nullptr) return;
    _audioOut->Start(_currentChannel);
}

OneRotaryEncoder* DeviceControls::GetEncoder()
{
    return _encoder;
}
