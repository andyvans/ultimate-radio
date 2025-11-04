#include "AudioOut.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "_Secrets.h"
#include <WiFi.h>

// WiFi event handler for detailed debugging
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("[WiFi] Station Started");
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.print("[WiFi] Connected to AP: ");
        Serial.println(WiFi.SSID());
        Serial.print("[WiFi] BSSID: ");
        Serial.println(WiFi.BSSIDstr());
        Serial.print("[WiFi] Channel: ");
        Serial.println(WiFi.channel());
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("[WiFi] Got IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("[WiFi] Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("[WiFi] Subnet: ");
        Serial.println(WiFi.subnetMask());
        Serial.print("[WiFi] DNS: ");
        Serial.println(WiFi.dnsIP());
        Serial.print("[WiFi] Signal Strength (RSSI): ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("[WiFi] Lost IP");
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.print("[WiFi] Disconnected from AP: ");
        Serial.println((char*)info.wifi_sta_disconnected.ssid);
        Serial.print("[WiFi] Reason code: ");
        Serial.print(info.wifi_sta_disconnected.reason);
        Serial.print(" - ");
        switch (info.wifi_sta_disconnected.reason) {
        case WIFI_REASON_UNSPECIFIED: Serial.println("UNSPECIFIED"); break;
        case WIFI_REASON_AUTH_EXPIRE: Serial.println("AUTH_EXPIRE - Authentication expired"); break;
        case WIFI_REASON_AUTH_LEAVE: Serial.println("AUTH_LEAVE - Authentication left"); break;
        case WIFI_REASON_ASSOC_EXPIRE: Serial.println("ASSOC_EXPIRE - Association expired"); break;
        case WIFI_REASON_ASSOC_TOOMANY: Serial.println("ASSOC_TOOMANY - Too many associations"); break;
        case WIFI_REASON_NOT_AUTHED: Serial.println("NOT_AUTHED - Not authenticated"); break;
        case WIFI_REASON_NOT_ASSOCED: Serial.println("NOT_ASSOCED - Not associated"); break;
        case WIFI_REASON_ASSOC_LEAVE: Serial.println("ASSOC_LEAVE - Association leave"); break;
        case WIFI_REASON_ASSOC_NOT_AUTHED: Serial.println("ASSOC_NOT_AUTHED - Association not authenticated"); break;
        case WIFI_REASON_DISASSOC_PWRCAP_BAD: Serial.println("DISASSOC_PWRCAP_BAD - Power capability bad"); break;
        case WIFI_REASON_DISASSOC_SUPCHAN_BAD: Serial.println("DISASSOC_SUPCHAN_BAD - Supported channels bad"); break;
        case WIFI_REASON_IE_INVALID: Serial.println("IE_INVALID - Invalid IE"); break;
        case WIFI_REASON_MIC_FAILURE: Serial.println("MIC_FAILURE - MIC failure"); break;
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT: Serial.println("4WAY_HANDSHAKE_TIMEOUT - 4-way handshake timeout"); break;
        case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT: Serial.println("GROUP_KEY_UPDATE_TIMEOUT - Group key update timeout"); break;
        case WIFI_REASON_IE_IN_4WAY_DIFFERS: Serial.println("IE_IN_4WAY_DIFFERS - IE in 4-way differs"); break;
        case WIFI_REASON_GROUP_CIPHER_INVALID: Serial.println("GROUP_CIPHER_INVALID - Group cipher invalid"); break;
        case WIFI_REASON_PAIRWISE_CIPHER_INVALID: Serial.println("PAIRWISE_CIPHER_INVALID - Pairwise cipher invalid"); break;
        case WIFI_REASON_AKMP_INVALID: Serial.println("AKMP_INVALID - AKMP invalid"); break;
        case WIFI_REASON_UNSUPP_RSN_IE_VERSION: Serial.println("UNSUPP_RSN_IE_VERSION - Unsupported RSN IE version"); break;
        case WIFI_REASON_INVALID_RSN_IE_CAP: Serial.println("INVALID_RSN_IE_CAP - Invalid RSN IE capabilities"); break;
        case WIFI_REASON_802_1X_AUTH_FAILED: Serial.println("802_1X_AUTH_FAILED - 802.1X auth failed"); break;
        case WIFI_REASON_CIPHER_SUITE_REJECTED: Serial.println("CIPHER_SUITE_REJECTED - Cipher suite rejected"); break;
        case WIFI_REASON_BEACON_TIMEOUT: Serial.println("BEACON_TIMEOUT - Beacon timeout"); break;
        case WIFI_REASON_NO_AP_FOUND: Serial.println("NO_AP_FOUND - No AP found"); break;
        case WIFI_REASON_AUTH_FAIL: Serial.println("AUTH_FAIL - Authentication failed"); break;
        case WIFI_REASON_ASSOC_FAIL: Serial.println("ASSOC_FAIL - Association failed"); break;
        case WIFI_REASON_HANDSHAKE_TIMEOUT: Serial.println("HANDSHAKE_TIMEOUT - Handshake timeout"); break;
        case WIFI_REASON_CONNECTION_FAIL: Serial.println("CONNECTION_FAIL - Connection failed"); break;
        default: Serial.println("UNKNOWN REASON"); break;
        }
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("[WiFi] Auth mode changed");
        break;
    default:
        Serial.print("[WiFi] Unknown event: ");
        Serial.println(event);
        break;
    }
}

// Define the array of radio channels
const RadioChannel AudioOut::channels[] = {
    {"http://stream.srg-ssr.ch/m/rsj/mp3_128", "audio/mp3"},
    {"http://stream.srg-ssr.ch/m/rsc_de/mp3_128", "audio/mp3"},
    {"http://jazz-wr11.ice.infomaniak.ch/jazz-wr11-128.mp3", "audio/mp3"},
    {"http://stream.srg-ssr.ch/m/drsvirus/mp3_128", "audio/mp3"},
    {"https://radionz-ice.streamguys.com/national.mp3", "audio/mp3"},
    {"http://stream.srg-ssr.ch/m/couleur3/mp3_128", "audio/mp3"},
    {"http://stream.srg-ssr.ch/m/rr/mp3_128", "audio/mp3"},
    {"http://sc2.radiocaroline.net/;?type=http&nocache=3741", "audio/mp3"}
};

const int AudioOut::channelCount = sizeof(AudioOut::channels) / sizeof(AudioOut::channels[0]);

AudioOut::AudioOut()
{
    _currentChannel = -1;
    _mode = AUDIO_MODE_OFF;
    _isPlaying = false;
}

void AudioOut::Setup()
{
    Serial.println("Setting up AudioOut");
    AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);

    /*
    // Initialize WiFi with event handler
    Serial.println("[WiFi] Initializing WiFi...");
    WiFi.onEvent(WiFiEvent);

    Serial.print("[WiFi] Connecting to SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("[WiFi] Using password: ");
    // Only show first and last character of password for security
    String password = String(WIFI_PASSWORD);
    if (password.length() > 2) {
        Serial.print(password.charAt(0));
        for (int i = 1; i < password.length() - 1; i++) {
            Serial.print("*");
        }
        Serial.println(password.charAt(password.length() - 1));
    }
    else {
        Serial.println("***");
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Wait for connection with timeout
    Serial.println("[WiFi] Waiting for connection...");
    int attempts = 0;
    const int maxAttempts = 30; // 30 seconds timeout

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(1000);
        attempts++;
        Serial.print("[WiFi] Connection attempt ");
        Serial.print(attempts);
        Serial.print("/");
        Serial.print(maxAttempts);
        Serial.print(" - Status: ");
        switch (WiFi.status()) {
        case WL_IDLE_STATUS: Serial.println("IDLE"); break;
        case WL_NO_SSID_AVAIL: Serial.println("NO_SSID_AVAILABLE"); break;
        case WL_SCAN_COMPLETED: Serial.println("SCAN_COMPLETED"); break;
        case WL_CONNECTED: Serial.println("CONNECTED"); break;
        case WL_CONNECT_FAILED: Serial.println("CONNECT_FAILED"); break;
        case WL_CONNECTION_LOST: Serial.println("CONNECTION_LOST"); break;
        case WL_DISCONNECTED: Serial.println("DISCONNECTED"); break;
        default: Serial.print("UNKNOWN("); Serial.print(WiFi.status()); Serial.println(")"); break;
        }

        // Show available networks every 10 attempts
        if (attempts % 10 == 0) {
            Serial.println("[WiFi] Scanning for available networks...");
            int n = WiFi.scanNetworks();
            if (n == 0) {
                Serial.println("[WiFi] No networks found");
            }
            else {
                Serial.print("[WiFi] Found ");
                Serial.print(n);
                Serial.println(" networks:");
                for (int i = 0; i < n; ++i) {
                    Serial.print("  ");
                    Serial.print(i + 1);
                    Serial.print(": ");
                    Serial.print(WiFi.SSID(i));
                    Serial.print(" (");
                    Serial.print(WiFi.RSSI(i));
                    Serial.print(" dBm) ");
                    Serial.print("[");
                    switch (WiFi.encryptionType(i)) {
                    case WIFI_AUTH_OPEN: Serial.print("OPEN"); break;
                    case WIFI_AUTH_WEP: Serial.print("WEP"); break;
                    case WIFI_AUTH_WPA_PSK: Serial.print("WPA"); break;
                    case WIFI_AUTH_WPA2_PSK: Serial.print("WPA2"); break;
                    case WIFI_AUTH_WPA_WPA2_PSK: Serial.print("WPA/WPA2"); break;
                    case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-ENT"); break;
                    case WIFI_AUTH_WPA3_PSK: Serial.print("WPA3"); break;
                    case WIFI_AUTH_WPA2_WPA3_PSK: Serial.print("WPA2/WPA3"); break;
                    default: Serial.print("UNKNOWN"); break;
                    }
                    Serial.println("]");
                }
            }
            WiFi.scanDelete();
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WiFi] Successfully connected!");
    }
    else {
        Serial.println("[WiFi] Failed to connect after timeout");
        Serial.print("[WiFi] Final status: ");
        Serial.println(WiFi.status());
    }
    */
}

int AudioOut::GetChannelCount()
{
    return channelCount;
}

void AudioOut::StartRadio(int channel)
{
    // Validate channel index
    if (channel < 0 || channel >= channelCount)
    {
        Serial.print("Invalid channel index: ");
        Serial.println(channel);
        return;
    }
    _currentChannel = channel;
    _mode = AUDIO_MODE_RADIO;
}

void AudioOut::Stop()
{
    _mode = AUDIO_MODE_OFF;
}

void AudioOut::Tick()
{
    /*
    // Monitor WiFi status periodically
    static unsigned long lastWiFiCheck = 0;
    const unsigned long wifiCheckInterval = 10000; // Check every 10 seconds

    if (millis() - lastWiFiCheck > wifiCheckInterval) {
        lastWiFiCheck = millis();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.print("[WiFi] Status: CONNECTED - RSSI: ");
            Serial.print(WiFi.RSSI());
            Serial.print(" dBm, IP: ");
            Serial.println(WiFi.localIP());
        }
        else {
            Serial.print("[WiFi] Status: ");
            switch (WiFi.status()) {
            case WL_IDLE_STATUS: Serial.println("IDLE"); break;
            case WL_NO_SSID_AVAIL: Serial.println("NO_SSID_AVAILABLE"); break;
            case WL_SCAN_COMPLETED: Serial.println("SCAN_COMPLETED"); break;
            case WL_CONNECT_FAILED: Serial.println("CONNECT_FAILED"); break;
            case WL_CONNECTION_LOST: Serial.println("CONNECTION_LOST"); break;
            case WL_DISCONNECTED: Serial.println("DISCONNECTED"); break;
            default: Serial.print("UNKNOWN("); Serial.print(WiFi.status()); Serial.println(")"); break;
            }

            // Attempt reconnection if disconnected
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("[WiFi] Attempting reconnection...");
                WiFi.reconnect();
            }
        }
    }
    return;
    */

    if (_mode == AUDIO_MODE_OFF && _isPlaying)
    {
        Serial.println("Stopping audio playback");
        Stop();
        _isPlaying = false;
        return;
    }
    else if (_mode == AUDIO_MODE_RADIO && !_isPlaying)
    {
        Serial.println("Starting radio stream");
        StartRadioStream();
        _isPlaying = true;
        return;
    }

    if (_copier != nullptr)
    {
        _copier->copy();
    }
}

void AudioOut::StopAudio()
{
    if (_copier != nullptr)
    {
        _copier->end();
        delete _copier;
        _copier = nullptr;
    }
    if (_in != nullptr)
    {
        _in->end();
        delete _in;
        _in = nullptr;
    }
    if (_out != nullptr)
    {
        _out->end();
        delete _out;
        _out = nullptr;
    }

    if (_infoFrom != nullptr)
    {
        delete _infoFrom;
        _infoFrom = nullptr;
    }
}

void AudioOut::StartRadioStream()
{
    Serial.println("Starting I2S output");
    _infoFrom = new AudioInfo(44100, 2, 16);

    _out = new I2SStream();
    auto configOut = _out->defaultConfig(TX_MODE);
    configOut.copyFrom(*_infoFrom);
    configOut.port_no = 1;
    configOut.pin_bck = I2S_BCLK_OUT;
    configOut.pin_ws = I2S_LRC_OUT;
    configOut.pin_data = I2S_DATA_OUT;
    configOut.channels = 2;

    _out->begin(configOut);

    auto channelInfo = channels[_currentChannel];
    Serial.print("Starting radio stream (");
    Serial.print(_currentChannel);
    Serial.print(") ");
    Serial.println(channelInfo.url);

    Serial.println("Starting URL stream");
    _urlStream = new URLStream(WIFI_SSID, WIFI_PASSWORD);
    _resampler = new ResampleStream(*_out);
    _decodedStream = new EncodedAudioStream(_resampler, new MP3DecoderHelix());
    _copier = new StreamCopy(*_decodedStream, *_urlStream);

    _urlStream->begin(channelInfo.url, channelInfo.mimeType);
    _decodedStream->begin();
    _isPlaying = true;
}