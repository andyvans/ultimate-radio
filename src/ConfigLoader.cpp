#include "ConfigLoader.h"
#include <WiFiClientSecure.h>
#include <AudioTools.h>
#include <ctype.h>

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

    bool success = ParseConfig(buffer, totalRead, config);
    free(buffer);

    return success;
}

bool ConfigLoader::ParseConfig(const char* data, int dataLen, RadioConfig& config)
{
    if (data == nullptr || dataLen == 0)
    {
        Serial.println("No data to parse");
        return false;
    }

    config.channels = new ChannelConfig[MAX_CHANNELS];
    if (config.channels == nullptr)
    {
        Serial.println("Failed to allocate channel array");
        return false;
    }

    int lineStart = 0;
    int lineNum = 0;
    config.channelCount = 0;
    config.defaultChannel = 0;

    for (int i = 0; i <= dataLen; i++)
    {
        if (i != dataLen && !IsLineEnding(data[i]))
            continue;

        int lineLen = GetLineLength(data, lineStart, i);
        int trimmedStart = lineStart;
        int trimmedLen = lineLen;
        TrimRange(data, trimmedStart, trimmedLen);

        bool skip = (trimmedLen == 0 || data[trimmedStart] == '#');

        if (!skip && lineNum == 0)
        {
            char* numStr = AllocateString(data, trimmedStart, trimmedLen, 10);
            if (numStr != nullptr)
            {
                config.defaultChannel = atoi(numStr);
                Serial.print("Default channel: ");
                Serial.println(config.defaultChannel);
                free(numStr);
            }
        }
        else if (!skip && config.channelCount < MAX_CHANNELS)
        {
            int commaPos = -1;
            int lineEnd = trimmedStart + trimmedLen;
            for (int j = trimmedStart; j < lineEnd; j++)
            {
                if (data[j] == ',')
                {
                    commaPos = j;
                    break;
                }
            }

            int urlStart = trimmedStart;
            int urlLen = trimmedLen;
            int nameStart = 0;
            int nameLen = 0;

            if (commaPos >= 0)
            {
                urlLen = commaPos - trimmedStart;
                nameStart = commaPos + 1;
                nameLen = lineEnd - nameStart;
            }

            TrimRange(data, urlStart, urlLen);
            if (nameLen > 0)
            {
                TrimRange(data, nameStart, nameLen);
                TrimQuotes(data, nameStart, nameLen);
            }

            char* url = nullptr;
            char* name = nullptr;

            if (urlLen > 0)
            {
                url = AllocateString(data, urlStart, urlLen, MAX_URL_LENGTH);
            }

            if (url != nullptr)
            {
                if (nameLen > 0)
                {
                    name = AllocateString(data, nameStart, nameLen, MAX_NAME_LENGTH);
                }

                // Backward-compatible fallback when no name is provided.
                if (name == nullptr)
                {
                    name = AllocateString(data, urlStart, urlLen, MAX_NAME_LENGTH);
                }

                config.channels[config.channelCount].url = url;
                config.channels[config.channelCount].name = name;

                Serial.print("Channel ");
                Serial.print(config.channelCount);
                Serial.print(" URL: ");
                Serial.println(url);

                Serial.print("Channel ");
                Serial.print(config.channelCount);
                Serial.print(" Name: ");
                Serial.println(name != nullptr ? name : "");

                config.channelCount++;
            }
        }

        if (!skip) lineNum++;

        i = SkipLineEnding(data, i, dataLen);
        lineStart = i + 1;
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

char* ConfigLoader::AllocateString(const char* data, int start, int length, int maxLen)
{
    int copyLen = min(length, maxLen - 1);
    char* str = (char*)malloc(copyLen + 1);
    if (str != nullptr)
    {
        strncpy(str, data + start, copyLen);
        str[copyLen] = '\0';
    }
    return str;
}

void ConfigLoader::TrimRange(const char* data, int& start, int& length)
{
    while (length > 0 && isspace((unsigned char)data[start]))
    {
        start++;
        length--;
    }

    while (length > 0 && isspace((unsigned char)data[start + length - 1]))
    {
        length--;
    }
}

void ConfigLoader::TrimQuotes(const char* data, int& start, int& length)
{
    if (length < 2) return;

    char first = data[start];
    char last = data[start + length - 1];

    if ((first == '"' && last == '"') || (first == '\'' && last == '\''))
    {
        start++;
        length -= 2;
    }
}