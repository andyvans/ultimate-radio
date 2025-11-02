#include "DeviceControls.h"
#include "AudioOut.h"

DeviceControls::DeviceControls() : audioOut(nullptr), currentChannel(0)
{
}

void DeviceControls::Setup(AudioOut* audio)
{
    audioOut = audio;
    currentChannel = 0;
    
    // Create rotary encoder instance
    // Max value is maxChannels - 1 since we're using 0-based indexing
    int initialChannel = 0;
    int maxChannel = audioOut != nullptr ? audioOut->GetChannelCount() - 1 : 0;    
    
    encoder = new OneRotaryEncoder(
        ENCODER_PIN_A,
        ENCODER_PIN_B,
        ENCODER_PIN_SWITCH,
        initialChannel, // Set initial value
        0, // Set min value
        maxChannel // Set max value
    );

    // Start first radio channel by default
    audioOut->StartRadio(initialChannel); 
}

void DeviceControls::Tick()
{
    if (encoder == nullptr) return;

    encoder->Tick();

    // Check for position changes
    EncoderPositionState posState = encoder->GetPosition();
    if (posState.hasNewPosition && posState.position != currentChannel)
    {
        currentChannel = posState.position;
        Serial.print("Changing to channel: ");
        Serial.println(currentChannel);
            
        // Stop current stream and start new channel
        audioOut->Stop();
        audioOut->StartRadio(currentChannel);
    }

    // Check for button presses
    EncoderSwitchState switchState = encoder->GetSwitchState();
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

OneRotaryEncoder* DeviceControls::GetEncoder()
{
    return encoder;
}
