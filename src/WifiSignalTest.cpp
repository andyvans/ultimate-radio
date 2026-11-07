#include <Arduino.h>
#include <WiFi.h>
#include "WifiSignalTest.h"

static const unsigned long _rssiLogIntervalMs = 2000;
static unsigned long _lastRssiLogMs = 0;

void LogWifiSignal()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi status: disconnected");
    return;
  }

  long rssiDbm = WiFi.RSSI();
  Serial.print("WiFi RSSI: ");
  Serial.print(rssiDbm);
  Serial.println(" dBm");
}

void WifiSignalTestTick()
{
  unsigned long now = millis();
  if (now - _lastRssiLogMs >= _rssiLogIntervalMs)
  {
    _lastRssiLogMs = now;
    LogWifiSignal();
  }
}
