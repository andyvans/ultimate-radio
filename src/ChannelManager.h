#pragma once
#include <Arduino.h>
#include "ConfigLoader.h"

class ChannelManager
{
public:
    static RadioConfig* LoadChannels(const char* ssid, const char* password, const char* configUrl);
    static RadioConfig* GetDefaultChannels();

private:
    static const char* _defaultChannels[];
    static const int _defaultChannelCount;
    static const int _defaultChannel;
};
