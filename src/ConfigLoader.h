#pragma once
#include <Arduino.h>

#define MAX_CHANNELS 20
#define MAX_URL_LENGTH 256

struct RadioConfig
{
    char** urls;
    int channelCount;
    int defaultChannel;

    RadioConfig() : urls(nullptr), channelCount(0), defaultChannel(0) {}

    ~RadioConfig()
    {
        if (urls != nullptr)
        {
            for (int i = 0; i < channelCount; i++)
            {
                if (urls[i] != nullptr)
                {
                    free(urls[i]);
                }
            }
            free(urls);
        }
    }
};

class ConfigLoader
{
public:
    static bool LoadConfig(const char* configUrl, RadioConfig& config);

private:
    static bool parseCSV(const char* data, int dataLen, RadioConfig& config);
};
