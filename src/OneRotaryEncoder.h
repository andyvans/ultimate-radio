#pragma once
#include <functional> // For std::function
#include <OneButton.h>
#include <RotaryEncoder.h>

enum EncoderSwitchPress {
    None,
    Clicked,
    DoubleClicked,
    LongPressed
};

struct EncoderSwitchState {
    EncoderSwitchPress state;
    bool hasNewState;
};

struct EncoderPositionState {
    int position;
    bool hasNewPosition;
};

class OneRotaryEncoder {
public:
    OneRotaryEncoder(int pinA, int pinB);
    OneRotaryEncoder(int pinA, int pinB, int pinSwitch);
    void SetRange(int minValue, int maxValue, int clicksPerValue, int initialValue);
    void Tick();
    void SetPosition(int position);
    EncoderPositionState GetPosition();
    EncoderSwitchState GetSwitchState();

private:
    OneButton* button;
    RotaryEncoder* encoder;

    int lastPosition = -1;
    EncoderSwitchPress switchState = EncoderSwitchPress::None;

    bool hasNewSwitchState = false;
    bool hasNewPosition = false;

    void Clicked();
    void DoubleClicked();
    void LongPressed();
    int CalculateAcceleration(int oldPos, int newPos);

    int rotarySteps = 1;
    int rotaryInitial = 0;
    int rotaryMin = 0;
    int rotaryMax = 128;
};
