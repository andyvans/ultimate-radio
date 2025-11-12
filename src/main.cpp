#include <Arduino.h>
#include "AudioOut.h"
#include "DeviceControls.h"
#include "DeckLight.h"

AudioOut* audioOut;
DeviceControls* deviceControls;
DeckLight* deckLight;

TaskHandle_t DeviceTask;
TaskHandle_t AudioTask;

void ProcessDevices(void* parameter);
void ProcessAudio(void* parameter);

void setup()
{
  Serial.begin(115200);

  audioOut = new AudioOut();
  audioOut->Setup();

  deckLight = new DeckLight();
  deckLight->Setup();

  deviceControls = new DeviceControls();
  deviceControls->Setup(audioOut, deckLight);

  Serial.println("Creating tasks");
  xTaskCreatePinnedToCore(ProcessAudio, "Audio", 10000, NULL, 1, &AudioTask, 0);
  xTaskCreatePinnedToCore(ProcessDevices, "Device", 10000, NULL, 1, &DeviceTask, 1);
}

void loop()
{
  // All processing is done in the tasks
}

void ProcessDevices(void* parameter)
{
  for (;;)
  {
    deviceControls->Tick();
    vTaskDelay(1);
  }
}

void ProcessAudio(void* parameter)
{
  for (;;)
  {
    audioOut->Tick();
    vTaskDelay(1);
  }
}