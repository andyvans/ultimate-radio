#pragma once
#include <Arduino.h>
#include "OneRotaryEncoder.h"
#include "constants.h"

class AudioOut; // Forward declaration

class DeviceControls
{
public:
    DeviceControls();
    void Setup(AudioOut* audio);
    void Tick();
    OneRotaryEncoder* GetEncoder();

private:
    OneRotaryEncoder* encoder;
    AudioOut* audioOut;
    int currentChannel;
};
