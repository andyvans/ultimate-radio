#include <Arduino.h>
#include "constants.h"
#include "DeckLight.h"
#include "AudioProc.h"

TaskHandle_t AudioTask;
TaskHandle_t MidiTask;

DeckLight deckLight;
AudioProc audioProc;

// Task functions
void ProcessAudio(void *parameter);

void setup()
{
  Serial.begin(115200);
    
  deckLight.Setup();
  
  xTaskCreatePinnedToCore(ProcessAudio, "Audio", 10000, NULL, 1, &AudioTask, 0);  
}

void loop()
{
  // All processing in tasks
}

void ProcessAudio(void *parameter)
{
  Serial.print("Audio task running on core ");
  Serial.println(xPortGetCoreID());
  for (;;)
  {
    deckLight.Tick();
    audioProc.Analyse([](){deckLight.Tick();});    
    deckLight.DisplayAudio(audioProc.bandValues);
    vTaskDelay(1);
  }
}
