#pragma once

#include "daisy_seed.h"

using namespace daisy;

// Reads a button with an external pull-up resistor (e.g. on seed::A3).
// Button connects the pin to GND when pressed.
//
// Call Update() once every loop, then use any of the functions below.
class Button
{
  public:
    // internal_pullup = true if there is no pull-up resistor in hardware
    void Init(Pin pin, bool internal_pullup = false)
    {
        gpio_.Init(pin,
                   GPIO::Mode::INPUT,
                   internal_pullup ? GPIO::Pull::PULLUP : GPIO::Pull::NOPULL);
    }

    // Reads the pin and filters out bounce. Call once every loop.
    void Update()
    {
        uint32_t now = System::GetNow();
        bool     raw = !gpio_.Read();

        just_pressed_  = false;
        just_released_ = false;

        // Restart the timer every time the raw pin changes
        if(raw != last_raw_)
        {
            last_raw_    = raw;
            last_change_ = now;
        }

        // Accept the new state once it has been stable for kDebounceMs
        if(now - last_change_ >= kDebounceMs && raw != pressed_)
        {
            pressed_ = raw;
            if(pressed_)
            {
                just_pressed_ = true;
                press_start_  = now;
                count_++;
            }
            else
            {
                just_released_ = true;
                last_held_ms_  = now - press_start_;
            }
        }
    }

    // True while the button is held down
    bool IsPressed() const { return pressed_; }

    // True only on the loop where the button went down
    bool JustPressed() const { return just_pressed_; }

    // True only on the loop where the button came up
    bool JustReleased() const { return just_released_; }

    // How long the button has been held, in ms (0 if not pressed)
    uint32_t HeldTime() const
    {
        return pressed_ ? System::GetNow() - press_start_ : 0;
    }

    // True while the button has been held for at least ms (long press)
    bool IsHeld(uint32_t ms) const { return HeldTime() >= ms; }

    // How long the last press lasted, in ms (valid after JustReleased())
    uint32_t LastPressTime() const { return last_held_ms_; }

    // Total number of presses since Init() or ResetCount()
    uint32_t Count() const { return count_; }
    void     ResetCount() { count_ = 0; }

    // Flips every time the button is pressed (on/off switch)
    bool Toggled() const { return count_ % 2 == 1; }

  private:
    static const uint32_t kDebounceMs = 10;

    GPIO     gpio_;
    bool     last_raw_      = false;
    bool     pressed_       = false;
    bool     just_pressed_  = false;
    bool     just_released_ = false;
    uint32_t last_change_   = 0;
    uint32_t press_start_   = 0;
    uint32_t last_held_ms_  = 0;
    uint32_t count_         = 0;
};
