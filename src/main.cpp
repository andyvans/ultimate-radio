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

  Serial.println("Creating tasks");

  xTaskCreatePinnedToCore(
    ProcessDevices,
    "Device",
    2048,
    NULL,
    1,
    &DeviceTask,
    0); // Core 0 (shared with WiFi & system tasks)

  xTaskCreatePinnedToCore(
    ProcessAudio,
    "Audio",
    16384, // Stack size increased to 16KB for better audio processing
    NULL,
    12, // High priority for smooth audio
    &AudioTask,
    1); // Core 1 for audio processing 

  Serial.println("=== Audio task created on Core 1 ===");
}

void loop()
{
  // Empty. All processing is done in tasks.
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

void ProcessAudio(void* parameter)
{
  for (;;)
  {
    if (audioOut != nullptr)
    {
      audioOut->Tick();
    }
    // No delay - audio processing needs to run continuously for smooth playback
    taskYIELD();
  }
}