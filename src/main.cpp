#include <Arduino.h>
#include "AudioOut.h"
#include "DeviceControls.h"
#include "DeckLight.h"
#include "ChannelManager.h"
#include "_Secrets.h"

// Configuration URL - change this to your config file location
#define CONFIG_URL "https://raw.githubusercontent.com/andyvans/ultimate-radio/main/radio-config.txt"

AudioOut* audioOut;
DeviceControls* deviceControls;
DeckLight* deckLight;
RadioConfig* radioConfig;

TaskHandle_t DeviceTask = NULL;

void ProcessDevices(void* parameter);

void setup()
{
  Serial.begin(115200);
  delay(200);

  Serial.println("\n\n=== Ultimate Radio Starting ===");

  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("PSRAM size: %u bytes\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %u bytes\n", ESP.getFreePsram());

  // AAC support requires PSRAM due to the larger buffers
  bool supportAac = ESP.getPsramSize() > 0;
  if (!supportAac)
    Serial.println("No PSRAM detected - AAC support disabled");

  radioConfig = ChannelManager::LoadChannels(WIFI_SSID, WIFI_PASSWORD, CONFIG_URL);
  if (radioConfig == nullptr)
  {
    Serial.println("Using default channels");
    radioConfig = ChannelManager::GetDefaultChannels();
  }

  audioOut = new AudioOut(supportAac);
  audioOut->Setup(radioConfig->channels, radioConfig->channelCount, radioConfig->defaultChannel);

  deckLight = new DeckLight();
  deckLight->Setup();

  deviceControls = new DeviceControls();
  deviceControls->Setup(audioOut, deckLight);

  // Create task for device controls
  Serial.println("Creating tasks");
  xTaskCreatePinnedToCore(
    ProcessDevices,
    "Device",
    4096, // Reduced for just wled and device controls
    NULL,
    1,
    &DeviceTask,
    0); // Core 0 (shared with WiFi & system tasks)  
}

void loop()
{
  // Only process audio in the main loop (core 1)
  if (audioOut != nullptr) audioOut->Tick();
}

// Task to process device controls (core 0)
void ProcessDevices(void* parameter)
{
  for (;;)
  {
    if (deviceControls != nullptr)
    {
      deviceControls->Tick();
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}