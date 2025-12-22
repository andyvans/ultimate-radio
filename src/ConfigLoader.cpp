#include "ConfigLoader.h"
#include <WiFiClientSecure.h>
#include <AudioTools.h>

using namespace audio_tools;

bool ConfigLoader::LoadConfig(const char* configUrl, RadioConfig& config)
{
    Serial.print("Loading config from: ");
    Serial.println(configUrl);

    WiFiClientSecure client;
    client.setInsecure(); // Skip certificate validation for simplicity

    HttpRequest http(client);
    Url url(configUrl);

    int statusCode = http.get(url, "text/plain");

    if (statusCode != 200)
    {
        Serial.print("HTTP request failed with status: ");
        Serial.println(statusCode);
        return false;
    }

    // Read response into buffer
    const int bufferSize = 4096;
    char* buffer = (char*)malloc(bufferSize);
    if (buffer == nullptr)
    {
        Serial.println("Failed to allocate buffer for config");
        http.end();
        return false;
    }

    int bytesRead = 0;
    int totalRead = 0;

    while (http.available() && totalRead < bufferSize - 1)
    {
        bytesRead = http.readBytes((uint8_t*)(buffer + totalRead), bufferSize - totalRead - 1);
        if (bytesRead > 0)
        {
            totalRead += bytesRead;
        }
        else
        {
            break;
        }
    }

    buffer[totalRead] = '\0';
    http.end();

    Serial.print("Downloaded ");
    Serial.print(totalRead);
    Serial.println(" bytes");

    bool success = parseCSV(buffer, totalRead, config);
    free(buffer);

    return success;
}

bool ConfigLoader::parseCSV(const char* data, int dataLen, RadioConfig& config)
{
    if (data == nullptr || dataLen == 0)
    {
        Serial.println("No data to parse");
        return false;
    }

    // Allocate URL array
    config.urls = (char**)malloc(MAX_CHANNELS * sizeof(char*));
    if (config.urls == nullptr)
    {
        Serial.println("Failed to allocate URL array");
        return false;
    }

    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        config.urls[i] = nullptr;
    }

    int lineStart = 0;
    int lineNum = 0;
    config.channelCount = 0;
    config.defaultChannel = 0;

    for (int i = 0; i <= dataLen; i++)
    {
        // End of line or end of data
        if (i == dataLen || data[i] == '\n' || data[i] == '\r')
        {
            if (i > lineStart)
            {
                // Calculate line length, skipping trailing \r if present
                int lineLen = i - lineStart;
                if (lineLen > 0 && data[i - 1] == '\r')
                {
                    lineLen--;
                }

                if (lineLen > 0)
                {
                    // First line is the default channel index
                    if (lineNum == 0)
                    {
                        char numStr[16];
                        int copyLen = min(lineLen, 15);
                        strncpy(numStr, data + lineStart, copyLen);
                        numStr[copyLen] = '\0';
                        config.defaultChannel = atoi(numStr);
                        Serial.print("Default channel: ");
                        Serial.println(config.defaultChannel);
                    }
                    else if (config.channelCount < MAX_CHANNELS)
                    {
                        // Skip comment lines
                        if (data[lineStart] != '#')
                        {
                            // Allocate and copy URL
                            int urlLen = min(lineLen, MAX_URL_LENGTH - 1);
                            config.urls[config.channelCount] = (char*)malloc(urlLen + 1);
                            if (config.urls[config.channelCount] != nullptr)
                            {
                                strncpy(config.urls[config.channelCount], data + lineStart, urlLen);
                                config.urls[config.channelCount][urlLen] = '\0';
                                Serial.print("Channel ");
                                Serial.print(config.channelCount);
                                Serial.print(": ");
                                Serial.println(config.urls[config.channelCount]);
                                config.channelCount++;
                            }
                        }
                    }
                    lineNum++;
                }
            }

            // Skip consecutive \r\n
            if (i < dataLen && data[i] == '\r' && i + 1 < dataLen && data[i + 1] == '\n')
            {
                i++;
            }

            lineStart = i + 1;
        }
    }

    Serial.print("Loaded ");
    Serial.print(config.channelCount);
    Serial.println(" channels");

    return config.channelCount > 0;
}
