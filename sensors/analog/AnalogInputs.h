#pragma once

#include "daisy_seed.h"

using namespace daisy;

// The Daisy ADC must be set up once for all analog pins.
// Analog sensors call AddAnalogInput() in their Init(), then main.cpp
// calls StartAnalogInputs() once after all of them are initialized.

const int kMaxAnalogInputs = 12;

inline AdcChannelConfig* AnalogConfigs()
{
    static AdcChannelConfig configs[kMaxAnalogInputs];
    return configs;
}

inline int& AnalogCount()
{
    static int count = 0;
    return count;
}

// Registers a pin and returns its channel number
inline int AddAnalogInput(Pin pin)
{
    int channel = AnalogCount()++;
    AnalogConfigs()[channel].InitSingle(pin);
    return channel;
}

inline void StartAnalogInputs(DaisySeed& hw)
{
    hw.adc.Init(AnalogConfigs(), AnalogCount());
    hw.adc.Start();
}
