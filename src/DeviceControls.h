#pragma once
#include <Arduino.h>
#include "OneRotaryEncoder.h"
#include "constants.h"
#include "DeckLight.h"
#include "AudioOut.h"

class DeviceControls
{
public:
    DeviceControls();
    void Setup(AudioOut* audio, DeckLight* deckLight);
    void Tick();

private:
    int _currentChannel;
    int _pendingChannel;
    unsigned long _lastPositionChangeTime;
    bool _hasPendingChange;
    OneRotaryEncoder* _encoder;
    AudioOut* _audioOut;
    DeckLight* _deckLight;

    int ChannelChangeDelayMs = 750;
};
