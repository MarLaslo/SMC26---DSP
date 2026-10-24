/** Hardware setup.
 *
 *  Declare one named sensor per line, wherever you use it:
 *
 *      AnalogIn  flexSensor(seed::D15, "flex sensor");
 *      DigitalIn startButton(seed::D9, "start button");
 *
 *  Then read it anywhere with:
 *
 *      flexSensor.Value();    // 0..1
 *      startButton.Value();   // true while held / closed
 *
 *  Analog fits: potentiometers, light sensors, flex sensors,
 *  distance sensors, anything giving a varying 0..3.3V signal.
 *  Digital fits: buttons, switches, contact sensors, anything
 *  giving a debounced on/off signal.
 *
 *  Call InitHardware() once at the top of main(), after all sensors
 *  are declared, and UpdateControls() once per loop iteration.
 */
#pragma once

#include "daisy_seed.h"
#include <cstddef>

extern daisy::DaisySeed hw;

constexpr size_t kMaxAnalogInputs  = 16;
constexpr size_t kMaxDigitalInputs = 16;

/** An analog sensor. Extends daisy::AnalogControl, so every reading
 *  is already slew-limited (2 ms by default) instead of the raw,
 *  jittery ADC value - pass a longer slew_seconds for a pot that
 *  feeds something especially sensitive to abrupt changes (e.g. an
 *  FM carrier frequency), or SetCoeff() directly for finer control.
 */
class AnalogIn : public daisy::AnalogControl
{
  public:
    AnalogIn(daisy::Pin pin, const char* name, float slew_seconds = 0.002f);

    const char* Name() const { return name_; }

    static size_t    Count();
    static AnalogIn* At(size_t index);

  private:
    friend void InitHardware();
    friend void UpdateControls();

    daisy::Pin  pin_;
    const char* name_;
    size_t      index_;
    float       slew_seconds_;

    static AnalogIn* instances_[kMaxAnalogInputs];
    static size_t    count_;
};

class DigitalIn
{
  public:
    DigitalIn(daisy::Pin pin, const char* name);

    bool        Value() const;
    const char* Name() const { return name_; }

    static size_t     Count();
    static DigitalIn* At(size_t index);

  private:
    friend void InitHardware();
    friend void UpdateControls();

    daisy::Pin    pin_;
    const char*   name_;
    daisy::Switch switch_;

    static DigitalIn* instances_[kMaxDigitalInputs];
    static size_t      count_;
};

// Call once per main-loop iteration to debounce digital inputs.
void UpdateControls();

// Call once per main-loop iteration. Prints every declared sensor's
// name and current value over serial (one line each, e.g.
// "flex sensor: 512"), throttled to roughly 10 times a second.
void PrintInputs();

// Sets up the board and all declared AnalogIn/DigitalIn sensors.
// Call once at the top of main().
void InitHardware();
