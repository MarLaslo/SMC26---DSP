#pragma once

#include "daisy_seed.h"
#include <cmath>
#include <cstdlib>

using namespace daisy;

// Prints sensor values over USB serial, all on one line, e.g.
//   button1: 1   knob1: 0.512   knob2: 0.000
// Call hw.StartLog() once first, then PrintValue() for each value
// and PrintLineEnd() to finish the line.

// For buttons and other on/off values: prints 1 or 0
inline void PrintValue(const char* name, bool value)
{
    DaisySeed::Print("%s: %d   ", name, value ? 1 : 0);
}

// For knobs and any other number: prints it with 3 decimals
template <typename T>
void PrintValue(const char* name, T value)
{
    int milli = (int)roundf((float)value * 1000.f);
    DaisySeed::Print("%s: %s%d.%03d   ",
                     name,
                     milli < 0 ? "-" : "",
                     abs(milli) / 1000,
                     abs(milli) % 1000);
}

inline void PrintLineEnd()
{
    DaisySeed::PrintLine("");
}

// True about 10 times a second - wrap your printing in it so the
// values scroll by slowly enough to read.
inline bool TimeToPrint()
{
    static uint32_t last = 0;
    uint32_t        now  = System::GetNow();
    if(now - last < 100)
        return false;
    last = now;
    return true;
}
