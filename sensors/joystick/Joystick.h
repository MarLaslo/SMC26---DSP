#pragma once

#include "daisy_seed.h"
#include "../analog/AnalogInputs.h"
#include "../button/Button.h"

using namespace daisy;

// Reads an X/Y joystick with a push switch (e.g. KY-023 module).
// VRx and VRy go to ADC pins, SW goes to any pin and connects to GND when pressed.
//
// Call Update() once every loop for the switch.
class Joystick
{
  public:
    // The switch uses the Daisy's internal pull-up
    Button sw;

    void Init(DaisySeed& hw, Pin x_pin, Pin y_pin, Pin sw_pin)
    {
        hw_        = &hw;
        x_channel_ = AddAnalogInput(x_pin);
        y_channel_ = AddAnalogInput(y_pin);
        sw.Init(sw_pin, true);
    }

    void Update() { sw.Update(); }

    // -1.0 to 1.0, 0 in the middle
    float X() { return Center(RawX()); }
    float Y() { return Center(RawY()); }

    // 0.0 to 1.0 straight from the ADC
    float RawX() { return ReadAnalogInput(*hw_, x_channel_); }
    float RawY() { return ReadAnalogInput(*hw_, y_channel_); }

    // True while the stick is pressed down
    bool IsPressed() const { return sw.IsPressed(); }

  private:
    // Values closer than this to the middle count as 0 (stops jitter at rest)
    static constexpr float kDeadzone = 0.05f;

    DaisySeed* hw_;
    int        x_channel_;
    int        y_channel_;

    float Center(float raw)
    {
        float v = raw * 2.f - 1.f;
        if(v > -kDeadzone && v < kDeadzone)
            return 0.f;
        return v;
    }
};
