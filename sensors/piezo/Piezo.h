#pragma once

#include "daisy_seed.h"
#include "../analog/AnalogInputs.h"

using namespace daisy;

// Reads a piezo disc on an ADC pin (e.g. seed::A1).
class Piezo
{
  public:
    void Init(DaisySeed& hw, Pin pin)
    {
        hw_      = &hw;
        channel_ = AddAnalogInput(pin);
    }

    // Returns 0.0 to 1.0
    float Read() { return hw_->adc.GetFloat(channel_); }

  private:
    DaisySeed* hw_;
    int        channel_;
};
