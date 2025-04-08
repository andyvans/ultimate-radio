#include "OneRotaryEncoder.h"

OneRotaryEncoder::OneRotaryEncoder(int pinA, int pinB, int initialValue, int minValue, int maxValue)
    : encoder(pinA, pinB, RotaryEncoder::LatchMode::TWO03)
{
  rotaryInitial = initialValue;
  rotaryMin = minValue;
  rotaryMax = maxValue;
  SetPosition(initialValue);
}

OneRotaryEncoder::OneRotaryEncoder(int pinA, int pinB, int pinSwitch, int initialValue, int minValue, int maxValue)
    : encoder(pinA, pinB, RotaryEncoder::LatchMode::TWO03), button(pinSwitch)
{
  rotaryInitial = initialValue;
  rotaryMin = minValue;
  rotaryMax = maxValue;
  SetPosition(initialValue);

  button.attachClick([](void *scope) { ((OneRotaryEncoder *) scope)->Clicked();}, this);
  button.attachLongPressStart([](void *scope) { ((OneRotaryEncoder *) scope)->LongPressed();}, this);  
}

void OneRotaryEncoder::Clicked()
{
  if (switchState != EncoderSwitchPress::Clicked)
  {
    switchState = EncoderSwitchPress::Clicked;
    SetPosition(rotaryMin);    
  }
  else
  {
    switchState = EncoderSwitchPress::None;
    SetPosition(rotaryInitial);
  }  
  hasNewSwitchState = true;
}

void OneRotaryEncoder::DoubleClicked()
{
  switchState = EncoderSwitchPress::DoubleClicked;
  SetPosition(rotaryMax);
  hasNewSwitchState = true;
}

void OneRotaryEncoder::LongPressed()
{
  switchState = EncoderSwitchPress::LongPressed;
  SetPosition(rotaryMax);
  hasNewSwitchState = true;
}

void OneRotaryEncoder::SetPosition(int position)
{
  encoder.setPosition(position/rotarySteps);
  lastPosition = position;
  hasNewPosition = true;
}

EncoderPositionState OneRotaryEncoder::GetPosition()
{
  EncoderPositionState state;
  state.position = lastPosition;
  state.hasNewPosition = hasNewPosition;
  hasNewPosition = false;
  return state;  
}

EncoderSwitchState OneRotaryEncoder::GetSwitchState()
{
  EncoderSwitchState state;
  state.state = switchState;
  state.hasNewState = hasNewSwitchState;
  hasNewSwitchState = false;
  return state;
}

void OneRotaryEncoder::Tick()
{  
  encoder.tick();   
  long newPosition = encoder.getPosition() * rotarySteps;
  if (newPosition < rotaryMin)
  {
    encoder.setPosition(rotaryMin/rotarySteps);
    newPosition = rotaryMin;
  }

  if (newPosition > rotaryMax)
  {
    encoder.setPosition(rotaryMax/rotarySteps);
    newPosition = rotaryMax;
  }
  
  if (lastPosition != newPosition)
  {
    lastPosition = newPosition;
    hasNewPosition = true;    
  }

  button.tick();
}
