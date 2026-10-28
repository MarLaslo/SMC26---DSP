#pragma once

#include "daisy_seed.h"

using namespace daisy;

// HC-SR04 ultrasonic distance sensor (e.g. Trig on seed::D0, Echo on seed::D1).
//
// Runs by itself in the background on hardware timer TIM5, so nothing ever
// waits. Read() just returns the latest distance.
//
// Up to kMaxSensors can be used. They take turns (one pings at a time) so
// they don't hear each other's echoes. Each one updates about every 60 ms
// times the number of sensors.
class Ultrasonic
{
  public:
    static const int kMaxSensors = 4;

    void Init(Pin trig_pin, Pin echo_pin)
    {
        trig_.Init(trig_pin, GPIO::Mode::OUTPUT);
        echo_.Init(echo_pin, GPIO::Mode::INPUT);
        trig_.Write(false);

        Shared& s = GetShared();
        if(s.count >= kMaxSensors)
            return;
        s.sensors[s.count] = this;
        s.count++;

        // The first sensor starts the timer that runs all of them
        if(s.count == 1)
        {
            s.ticks_per_us = System::GetTickFreq() / 1000000;

            // The period must be set here, before Init(). Setting it after
            // only takes effect once the timer's first (~18 s long) period ends.
            TimerHandle::Config cfg;
            cfg.periph     = TimerHandle::Config::Peripheral::TIM_5;
            cfg.enable_irq = true;
            cfg.period     = System::GetTickFreq() / kTickRateHz - 1;
            s.timer.Init(cfg);
            s.timer.SetCallback(Tick);
            s.timer.Start();
        }
    }

    // Latest distance in cm, or -1 if nothing was detected
    float Read() const { return distance_; }

  private:
    enum class State
    {
        IDLE,
        TRIGGER,
        WAIT_ECHO_HIGH,
        WAIT_ECHO_LOW,
    };

    // Checking every 50 us gives ~1 cm accuracy
    static const uint32_t kTickRateHz = 20000;
    // ~4 m range is ~23 ms of echo
    static const uint32_t kTimeoutUs = 25000;
    // Pause before each ping so old echoes die out
    static const uint32_t kGapUs = 30000;

    // Shared by all sensors: the list, whose turn it is, and the timer
    struct Shared
    {
        Ultrasonic* sensors[kMaxSensors];
        int         count  = 0;
        int         active = 0;
        uint32_t    ticks_per_us = 1;
        TimerHandle timer;
    };

    static Shared& GetShared()
    {
        static Shared shared;
        return shared;
    }

    GPIO           trig_, echo_;
    State          state_       = State::IDLE;
    uint32_t       state_start_ = 0; // in timer ticks
    volatile float distance_    = -1.f;

    // Called by the timer in the background: moves the active sensor
    // forward, and hands over to the next one when it's done
    static void Tick(void* data)
    {
        Shared& s = GetShared();
        if(s.count == 0)
            return;
        if(s.sensors[s.active]->Update())
        {
            s.active = (s.active + 1) % s.count;
            s.sensors[s.active]->Go(State::IDLE, System::GetTick());
        }
    }

    // Moves the measurement one step forward. Returns true when finished.
    bool Update()
    {
        // Timer ticks (not System::GetUs(), which jumps back to 0 every ~18 s)
        uint32_t now     = System::GetTick();
        uint32_t elapsed = (now - state_start_) / GetShared().ticks_per_us; // in us

        switch(state_)
        {
            // Wait a bit, then start the measurement
            case State::IDLE:
                if(elapsed >= kGapUs)
                {
                    trig_.Write(true);
                    Go(State::TRIGGER, now);
                }
                break;

            // Keep Trig high for at least 10 us
            case State::TRIGGER:
                if(elapsed >= 10)
                {
                    trig_.Write(false);
                    Go(State::WAIT_ECHO_HIGH, now);
                }
                break;

            // Wait for Echo to go high
            case State::WAIT_ECHO_HIGH:
                if(echo_.Read())
                    Go(State::WAIT_ECHO_LOW, now);
                else if(elapsed > kTimeoutUs)
                    return Finish(-1.f, now);
                break;

            // Echo stays high as long as the sound takes to come back
            case State::WAIT_ECHO_LOW:
                if(!echo_.Read())
                    return Finish(elapsed / 58.f, now); // ~58 us per cm
                else if(elapsed > kTimeoutUs)
                    return Finish(-1.f, now);
                break;
        }
        return false;
    }

    void Go(State next, uint32_t now)
    {
        state_       = next;
        state_start_ = now;
    }

    bool Finish(float distance, uint32_t now)
    {
        distance_ = distance;
        Go(State::IDLE, now);
        return true;
    }
};
