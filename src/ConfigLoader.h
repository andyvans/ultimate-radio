#pragma once
#include <Arduino.h>

#define MAX_CHANNELS 10
#define MAX_URL_LENGTH 128

struct ChannelConfig
{
    char* url;

    ChannelConfig() : url(nullptr) {}

    ChannelConfig(char* url) : url(url) {}
};

struct RadioConfig
{
    ChannelConfig* channels;
    int channelCount;
    int defaultChannel;
    bool ownsMemory;

    RadioConfig() : channels(nullptr), channelCount(0), defaultChannel(0), ownsMemory(true) {}

    ~RadioConfig()
    {
        if (ownsMemory && channels != nullptr)
        {
            for (int i = 0; i < channelCount; i++)
            {
                if (channels[i].url != nullptr) free(channels[i].url);
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
    static bool ParseConfig(const char* data, int dataLen, RadioConfig& config);
    static bool IsLineEnding(char c);
    static int GetLineLength(const char* data, int start, int end);
    static int SkipLineEnding(const char* data, int pos, int dataLen);
    static char* AllocateString(const char* data, int start, int length, int maxLen);
};
