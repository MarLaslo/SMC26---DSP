#pragma once

/** The board, and printing sensor values over USB.
 *
 *  main.cpp declares its inputs (buttons, knobs, ...) and writes two
 *  functions for them:
 *    InitInputs()  - calls Init() on every input, with its pin
 *    PrintValues() - calls PrintValue() for every input
 *  then calls InitHardware() once at the start of main().
 */
#include "daisy_seed.h"
#include "sensors/sensors.h"
#include <cmath>
#include <cstdlib>

using namespace daisy;

DaisySeed hw;

// Written in main.cpp
void InitInputs();
void PrintValues();

// ---------------------------------------------------------------------
// Printing
// ---------------------------------------------------------------------

// Prints sensor values over USB serial, all on one line, e.g.
//   button1: 1   knob1: 0.512   knob2: 0.000
// Call PrintValue() for each value and PrintLineEnd() to finish the
// line.

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

// ---------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------

// Set by InitHardware() - when true, main() prints the sensor values.
bool printValues = false;

// Sets up the board, then your inputs (InitInputs() in main.cpp).
// Pass true to print the sensor values over USB serial (open a serial
// monitor to see them).
inline void InitHardware(bool print)
{
    // Flush denormal floats to zero. Without this, a value decaying
    // toward (but never quite reaching) zero - a delay repeat trailing
    // off, an envelope's release tail - becomes a "denormal" number
    // that this FPU takes dramatically longer to do arithmetic on,
    // long enough to blow the audio callback's time budget and cause
    // an audible click/pop with no knob involved at all.
    __set_FPSCR(__get_FPSCR() | (1 << 24) | (1 << 25));

    hw.Init();

    printValues = print;
    if (printValues)
        hw.StartLog();

    InitInputs();
    StartAnalogInputs(hw); // once, after every analog sensor's Init()
}
