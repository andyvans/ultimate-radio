#include "OneRotaryEncoder.h"

OneRotaryEncoder::OneRotaryEncoder(int pinA, int pinB)
{
  encoder = new RotaryEncoder(pinA, pinB, RotaryEncoder::LatchMode::TWO03);
  button = nullptr;
}

OneRotaryEncoder::OneRotaryEncoder(int pinA, int pinB, int pinSwitch)
{
  encoder = new RotaryEncoder(pinA, pinB, RotaryEncoder::LatchMode::TWO03);
  button = new OneButton(pinSwitch, true, true);
  button->attachClick([](void* scope) { ((OneRotaryEncoder*)scope)->Clicked();}, this);
  button->attachLongPressStart([](void* scope) { ((OneRotaryEncoder*)scope)->LongPressed();}, this);
}

void OneRotaryEncoder::SetRange(int minValue, int maxValue, int clicksPerValue, int initialValue)
{
  rotaryInitial = initialValue;
  rotaryMin = minValue;
  rotaryMax = maxValue;
  rotarySteps = clicksPerValue;
  SetPosition(initialValue);
}

void OneRotaryEncoder::Clicked()
{
  if (switchState != EncoderSwitchPress::Clicked)
  {
    switchState = EncoderSwitchPress::Clicked;
  }
  else
  {
    switchState = EncoderSwitchPress::None;
  }
  hasNewSwitchState = true;
}

void OneRotaryEncoder::DoubleClicked()
{
  switchState = EncoderSwitchPress::DoubleClicked;
  hasNewSwitchState = true;
}

void OneRotaryEncoder::LongPressed()
{
  switchState = EncoderSwitchPress::LongPressed;
  hasNewSwitchState = true;
}

void OneRotaryEncoder::SetPosition(int position)
{
  encoder->setPosition(position * rotarySteps);
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
  encoder->tick();
  long encoderPos = encoder->getPosition();
  long newPosition = encoderPos / rotarySteps;

  if (newPosition < rotaryMin)
  {    
    encoder->setPosition(rotaryMin * rotarySteps);
    newPosition = rotaryMin;
  }
  else if (newPosition > rotaryMax)
  {    
    encoder->setPosition(rotaryMax * rotarySteps);
    newPosition = rotaryMax;
  }

  if (lastPosition != newPosition)
  {
    Serial.print("Encoder position changed: ");
    Serial.println(newPosition);
    lastPosition = newPosition;
    hasNewPosition = true;
  }

  if (button != nullptr) button->tick();
}
