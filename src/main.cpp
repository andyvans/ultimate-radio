#include <Arduino.h>
#include "AudioOut.h"
#include "DeviceControls.h"
#include "DeckLight.h"

AudioOut* audioOut;
DeviceControls* deviceControls;
DeckLight* deckLight;

TaskHandle_t AudioTask = NULL;
TaskHandle_t DeviceTask = NULL;


void ProcessDevices(void* parameter);
void ProcessAudio(void* parameter);

void setup()
{
  // Initialize UART0 explicitly with TX=43, RX=44 (ESP32-S3 defaults)
  Serial.begin(115200, SERIAL_8N1, -1, -1);
  delay(200);

  Serial.println("\n\n=== Ultimate Radio Starting ===");

  audioOut = new AudioOut();
  audioOut->Setup();

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