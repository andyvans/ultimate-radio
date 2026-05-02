#include "ChannelManager.h"
#include <WiFi.h>

// Define the array of fallback radio channels
const ChannelConfig ChannelManager::_defaultChannels[] = {
    {const_cast<char*>("https://admin.stream.rinse.fm/proxy/rinse_uk/stream"), const_cast<char*>("Rinse FM UK")},
    {const_cast<char*>("http://stream.srg-ssr.ch/srgssr/rsj/mp3/128"), const_cast<char*>("Radio Swiss Jazz")},
    {const_cast<char*>("https://streaming.brol.tech/rtfmlounge"), const_cast<char*>("RTFM Lounge")},
    {const_cast<char*>("http://streaming.swisstxt.ch/m/drsvirus/mp3_128"), const_cast<char*>("Dr Virus")},
    {const_cast<char*>("http://livestreaming-node-1.srg-ssr.ch/srgssr/couleur3/mp3/128"), const_cast<char*>("Couleur Swiss Radio")}
};

const int ChannelManager::_defaultChannelCount = sizeof(ChannelManager::_defaultChannels) / sizeof(ChannelManager::_defaultChannels[0]);
const int ChannelManager::_defaultChannel = 0;
const float ChannelManager::_defaultVolume = 0.5f;

RadioConfig* ChannelManager::LoadChannels(const char* configUrl)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi not connected, cannot load config");
        return nullptr;
    }

    Serial.println("Loading radio configuration...");
    RadioConfig* radioConfig = new RadioConfig();
    if (ConfigLoader::LoadConfig(configUrl, *radioConfig))
    {
        Serial.println("Config loaded successfully");
        return radioConfig;
    }
    else
    {
        Serial.println("Config load failed");
        delete radioConfig;
        return nullptr;
    }
}

RadioConfig* ChannelManager::GetDefaultChannels()
{
    RadioConfig* config = new RadioConfig();
    config->channels = (ChannelConfig*)_defaultChannels;
    config->channelCount = _defaultChannelCount;
    config->defaultChannel = _defaultChannel;
    config->volume = _defaultVolume;
    config->ownsMemory = false;
    return config;
}
