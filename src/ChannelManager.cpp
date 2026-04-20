#include "ChannelManager.h"
#include <WiFi.h>

// Define the array of fallback radio channels
const ChannelConfig ChannelManager::_defaultChannels[] = {
    {(char*)"https://admin.stream.rinse.fm/proxy/rinse_uk/stream", (char*)"Rinse FM UK"},
    {(char*)"https://radio10.pro-fhi.net/flux-trmqtiat/stream", (char*)"Rinse FM France"},
    {(char*)"http://stream.srg-ssr.ch/srgssr/rsj/mp3/128", (char*)"Radio Swiss Jazz"},
    {(char*)"https://streaming.brol.tech/rtfmlounge", (char*)"RTFM Lounge"},
    {(char*)"http://streaming.swisstxt.ch/m/drsvirus/mp3_128", (char*)"Dr Virus"},
    {(char*)"http://livestreaming-node-1.srg-ssr.ch/srgssr/couleur3/mp3/128", (char*)"Couleur Swiss Radio"}
};

const int ChannelManager::_defaultChannelCount = sizeof(ChannelManager::_defaultChannels) / sizeof(ChannelManager::_defaultChannels[0]);
const int ChannelManager::_defaultChannel = 0;

RadioConfig* ChannelManager::LoadChannels(const char* ssid, const char* password, const char* configUrl)
{
    RadioConfig* radioConfig = nullptr;

    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    int attempts = 0;
    int maxAttempts = 2; // 10 seconds timeout

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
    config->channels = (ChannelConfig*)_defaultChannels;
    config->channelCount = _defaultChannelCount;
    config->defaultChannel = _defaultChannel;
    config->ownsMemory = false;
    return config;
}
