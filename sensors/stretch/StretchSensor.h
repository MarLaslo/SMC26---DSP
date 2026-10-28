#pragma once

#include "daisy_seed.h"
#include "../analog/AnalogInputs.h"

using namespace daisy;

// Reads an Adafruit conductive rubber cord stretch sensor in a voltage
// divider on an ADC pin (e.g. seed::A7).
class StretchSensor
{
  public:
    void Init(DaisySeed& hw, Pin pin)
    {
        hw_      = &hw;
        channel_ = AddAnalogInput(pin);
    }

    // Returns 0.0 to 1.0
    float Read() { return ReadAnalogInput(*hw_, channel_); }

  private:
    DaisySeed* hw_;
    int        channel_;
};
