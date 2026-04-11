#pragma once
#include <Arduino.h>

#define MAX_CHANNELS 10
#define MAX_URL_LENGTH 80
#define MAX_MIME_LENGTH 32

struct ChannelConfig
{
    char* url;
    char* mimeType;

    ChannelConfig() : url(nullptr), mimeType(nullptr) {}
    ChannelConfig(char* url, char* mimeType) : url(url), mimeType(mimeType) {}
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
                if (channels[i].mimeType != nullptr) free(channels[i].mimeType);
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
    static bool ParseCSV(const char* data, int dataLen, RadioConfig& config);
    static bool IsLineEnding(char c);
    static int GetLineLength(const char* data, int start, int end);
    static int SkipLineEnding(const char* data, int pos, int dataLen);
    static char* AllocateAndCopyLine(const char* data, int start, int length);
    static void ParseChannelLine(const char* data, int start, int length, char** outUrl, char** outMime);
};
