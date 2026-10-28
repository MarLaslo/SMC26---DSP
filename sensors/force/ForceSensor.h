#pragma once

#include "daisy_seed.h"
#include "../analog/AnalogInputs.h"

using namespace daisy;

// Reads a force sensing resistor (FSR) in a voltage divider on an ADC pin
// (e.g. seed::A8). 3V3 -> FSR -> pin -> 10k -> GND, so pressing harder
// gives a higher value.
class ForceSensor
{
  public:
    void Init(DaisySeed& hw, Pin pin)
    {
        hw_      = &hw;
        channel_ = AddAnalogInput(pin);
    }

    // Returns 0.0 (not pressed) to 1.0 (pressed very hard)
    float Read() { return ReadAnalogInput(*hw_, channel_); }

    // True while pressed harder than threshold
    bool IsPressed(float threshold = 0.05f) { return Read() > threshold; }

  private:
    DaisySeed* hw_;
    int        channel_;
};
