#pragma once

#include "daisy_seed.h"

using namespace daisy;

// The Daisy ADC must be set up once for all analog pins.
// Analog sensors call AddAnalogInput() in their Init(). The ADC then starts
// by itself on the first ReadAnalogInput(), so Init() every analog sensor
// before reading any of them.

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

inline bool& AnalogStarted()
{
    static bool started = false;
    return started;
}

// Registers a pin and returns its channel number
inline int AddAnalogInput(Pin pin)
{
    int channel = AnalogCount()++;
    AnalogConfigs()[channel].InitSingle(pin);
    return channel;
}

// Starts the ADC with every pin added so far. Only does something the
// first time, and ReadAnalogInput() calls it for you.
inline void StartAnalogInputs(DaisySeed& hw)
{
    if(AnalogStarted())
        return;
    AnalogStarted() = true;
    hw.adc.Init(AnalogConfigs(), AnalogCount());
    hw.adc.Start();
}

// Returns 0.0 to 1.0, starting the ADC first if needed
inline float ReadAnalogInput(DaisySeed& hw, int channel)
{
    StartAnalogInputs(hw);
    return hw.adc.GetFloat(channel);
}
