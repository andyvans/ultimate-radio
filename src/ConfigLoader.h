#pragma once
#include <Arduino.h>

#define MAX_CHANNELS 10
#define MAX_URL_LENGTH 128
#define MAX_NAME_LENGTH 64

struct ChannelConfig
{
    char* url;
    char* name;

    ChannelConfig() : url(nullptr), name(nullptr) {}
    ChannelConfig(char* url, char* name) : url(url), name(name) {}
};

struct RadioConfig
{
    ChannelConfig* channels;
    int channelCount;
    int defaultChannel;
    float volume;
    bool ownsMemory;

    RadioConfig() : channels(nullptr), channelCount(0), defaultChannel(0), volume(0.5f), ownsMemory(true) {}

    ~RadioConfig()
    {
        if (ownsMemory && channels != nullptr)
        {
            for (int i = 0; i < channelCount; i++)
            {
                if (channels[i].url != nullptr) free(channels[i].url);
                if (channels[i].name != nullptr) free(channels[i].name);
            }
            delete[] channels;
        }
    }
};

class ConfigLoader
{
public:
    static bool LoadConfig(const char* configUrl, RadioConfig& config);

private:
    static bool ParseConfig(const String& data, RadioConfig& config);
    static char* DuplicateString(const String& str, int maxLen);
};