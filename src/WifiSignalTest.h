#pragma once

// Set to true to log WiFi signal strength instead of starting the radio
constexpr bool WifiSignalTestMode = false;

void LogWifiSignal();
void WifiSignalTestTick();
