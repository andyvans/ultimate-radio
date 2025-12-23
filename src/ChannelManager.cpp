#include "ChannelManager.h"
#include <WiFi.h>

// Define the array of fallback radio channels
const char* ChannelManager::_defaultChannels[] = {
    "https://stream.srg-ssr.ch/srgssr/rsc_de/mp3/128",
    "https://stream.srg-ssr.ch/m/couleur3/mp3_128",
    "https://stream.srg-ssr.ch/m/rsj/mp3_128",
    "http://live1.lankaradio.com:8010/128kbps.mp3"
};

const int ChannelManager::_defaultChannelCount = sizeof(ChannelManager::_defaultChannels) / sizeof(ChannelManager::_defaultChannels[0]);
const int ChannelManager::_defaultChannel = 2;

RadioConfig* ChannelManager::LoadChannels(const char* ssid, const char* password, const char* configUrl)
{
    RadioConfig* radioConfig = nullptr;

    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    int attempts = 0;
    int maxAttempts = 20; // 10 seconds timeout

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());

        // Try to load configuration from URL
        Serial.println("Loading radio configuration...");
        radioConfig = new RadioConfig();
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
    else
    {
        Serial.println("\nWiFi connection failed");
        return nullptr;
    }
}

RadioConfig* ChannelManager::GetDefaultChannels()
{
    RadioConfig* config = new RadioConfig();
    config->urls = (char**)_defaultChannels;
    config->channelCount = _defaultChannelCount;
    config->defaultChannel = _defaultChannel;
    return config;
}
