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

    bool success = ParseCSV(buffer, totalRead, config);
    free(buffer);

    return success;
}

bool ConfigLoader::ParseCSV(const char* data, int dataLen, RadioConfig& config)
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
        if (i == dataLen || IsLineEnding(data[i]))
        {
            int lineLen = GetLineLength(data, lineStart, i);

            if (lineLen > 0)
            {
                // First line is the default channel index
                if (lineNum == 0)
                {
                    char* numStr = AllocateAndCopyLine(data, lineStart, lineLen);
                    if (numStr != nullptr)
                    {
                        config.defaultChannel = atoi(numStr);
                        Serial.print("Default channel: ");
                        Serial.println(config.defaultChannel);
                        free(numStr);
                    }
                }
                else if (config.channelCount < MAX_CHANNELS && data[lineStart] != '#')
                {
                    // Skip comment lines starting with #
                    config.urls[config.channelCount] = AllocateAndCopyLine(data, lineStart, lineLen);
                    if (config.urls[config.channelCount] != nullptr)
                    {
                        Serial.print("Channel ");
                        Serial.print(config.channelCount);
                        Serial.print(": ");
                        Serial.println(config.urls[config.channelCount]);
                        config.channelCount++;
                    }
                }
                lineNum++;
            }

            i = SkipLineEnding(data, i, dataLen);
            lineStart = i + 1;
        }
    }

    Serial.print("Loaded ");
    Serial.print(config.channelCount);
    Serial.println(" channels");

    return config.channelCount > 0;
}

bool ConfigLoader::IsLineEnding(char c)
{
    return c == '\n' || c == '\r';
}

int ConfigLoader::GetLineLength(const char* data, int start, int end)
{
    int length = end - start;
    // Strip trailing \r if present
    if (length > 0 && data[end - 1] == '\r')
    {
        length--;
    }
    return length;
}

int ConfigLoader::SkipLineEnding(const char* data, int pos, int dataLen)
{
    // Skip \r\n sequence
    if (pos < dataLen && data[pos] == '\r' && pos + 1 < dataLen && data[pos + 1] == '\n')
    {
        return pos + 1;
    }
    return pos;
}

char* ConfigLoader::AllocateAndCopyLine(const char* data, int start, int length)
{
    int copyLen = min(length, MAX_URL_LENGTH - 1);
    char* str = (char*)malloc(copyLen + 1);
    if (str != nullptr)
    {
        strncpy(str, data + start, copyLen);
        str[copyLen] = '\0';
    }
    return str;
}