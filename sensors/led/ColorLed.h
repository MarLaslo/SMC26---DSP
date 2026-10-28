#pragma once

#include "daisy_seed.h"

using namespace daisy;

// Inolux HV-5RGB60 RGB LED (common cathode: long leg to GND).
// Each color leg goes through a resistor to a digital pin. Pin high = on.
class ColorLed
{
  public:
    void Init(Pin red_pin, Pin green_pin, Pin blue_pin)
    {
        red_.Init(red_pin, GPIO::Mode::OUTPUT);
        green_.Init(green_pin, GPIO::Mode::OUTPUT);
        blue_.Init(blue_pin, GPIO::Mode::OUTPUT);
        Off();
    }

    // Turn each color on (true) or off (false)
    void Set(bool red, bool green, bool blue)
    {
        red_.Write(red);
        green_.Write(green);
        blue_.Write(blue);
    }

    void Off() { Set(false, false, false); }
    void Red() { Set(true, false, false); }
    void Green() { Set(false, true, false); }
    void Blue() { Set(false, false, true); }
    void Yellow() { Set(true, true, false); }
    void Cyan() { Set(false, true, true); }
    void Magenta() { Set(true, false, true); }
    void White() { Set(true, true, true); }

  private:
    GPIO red_, green_, blue_;
};
