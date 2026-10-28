#pragma once

#include <stdint.h>
#include "Utility/samplehold.h"
#include "Utility/metro.h"

namespace dsp
{
/** Sample and hold: grabs ("samples") a value at one moment and keeps
 *  outputting it ("holds") until the next grab, turning any smooth
 *  signal into a staircase of steps. Fed with random values it gives
 *  the classic "random robot" bleeps of analog synths - a new random
 *  pitch or filter cutoff on every step.
 *
 *  Something has to tell it when to grab. Use any of these:
 *    - its own clock:  SetRate(4.f) grabs 4 times a second
 *    - Trigger():      grabs once, e.g. when a button is pressed
 *    - a gate signal:  Process(gate, input), grabs when gate turns on
 *
 *  What it grabs:
 *    - Process()       a random value in -1..1 (no input needed)
 *    - Process(input)  your signal, e.g. an Lfo or a sensor
 *
 *  Built on DaisySP's SampleHold, plus a Metro for the clock.
 *  Usage (random steps, inside AudioCallback):
 *      SampleHold steps;
 *      steps.Init(sample_rate);
 *      steps.SetRate(4.f);                // 4 new values a second
 *      ...
 *      float value = steps.Process();     // -1..1, steps 4 times a second
 *      synth.SetFreq(440.f * powf(2.f, value)); // +/- one octave
 */
class SampleHold
{
  public:
    enum Mode
    {
        /** Grabs one value when triggered, holds it until the next. */
        MODE_SAMPLE_HOLD,
        /** Follows the input while the gate is on, freezes it when the
         *  gate turns off. Only differs from MODE_SAMPLE_HOLD with
         *  Process(gate, input) and a gate held for a while. */
        MODE_TRACK_HOLD,
    };

    void Init(float sample_rate)
    {
        clock_.Init(1.f, sample_rate);
        SetRate(0.f);
        SetMode(MODE_SAMPLE_HOLD);
        trigger_ = false;
    }

    /** How many times a second the built-in clock grabs a new value.
     *  0 turns the clock off - then only Trigger() or a gate grabs. */
    void SetRate(float rate_hz)
    {
        clock_on_ = rate_hz > 0.f;
        if(clock_on_)
            clock_.SetFreq(rate_hz);
    }

    void SetMode(Mode mode)
    {
        mode_ = mode == MODE_TRACK_HOLD ? daisysp::SampleHold::MODE_TRACK_HOLD
                                        : daisysp::SampleHold::MODE_SAMPLE_HOLD;
    }

    /** Grabs a new value on the next Process(). Safe to call from
     *  main()'s loop, e.g. when a button is pressed. */
    void Trigger() { trigger_ = true; }

    /** Restarts the built-in clock, so the next step lands right now
     *  and the rhythm lines up with e.g. a NoteOn. */
    void Reset()
    {
        clock_.Reset();
        trigger_ = true;
    }

    /** Grabs a random value in -1..1 on each step. */
    float Process() { return Process(Random()); }

    /** Grabs "input" on each step (clock or Trigger()). */
    float Process(float input)
    {
        bool step = trigger_;
        trigger_  = false;
        if(clock_on_ && clock_.Process())
            step = true;
        stepped_ = step;

        // A trigger is a single sample of "on". Pulling it low again
        // straight after lets the next one count as a new edge.
        float out = sh_.Process(step, input, mode_);
        sh_.Process(false, input, mode_);
        return out;
    }

    /** True if the last Process() grabbed a new value - e.g. to start
     *  a new note on every step. */
    bool Stepped() const { return stepped_; }

    /** Grabs "input" when "gate" turns on (and, in MODE_TRACK_HOLD,
     *  follows it for as long as gate stays on). The clock and
     *  Trigger() are ignored here. */
    float Process(bool gate, float input)
    {
        stepped_   = gate && !last_gate_;
        last_gate_ = gate;
        return sh_.Process(gate, input, mode_);
    }

  private:
    // A fast pseudo-random number generator (xorshift), -1..1
    float Random()
    {
        seed_ ^= seed_ << 13;
        seed_ ^= seed_ >> 17;
        seed_ ^= seed_ << 5;
        return (seed_ / 2147483648.f) - 1.f;
    }

    daisysp::SampleHold       sh_;
    daisysp::SampleHold::Mode mode_     = daisysp::SampleHold::MODE_SAMPLE_HOLD;
    daisysp::Metro            clock_;
    bool                      clock_on_ = false;
    volatile bool             trigger_  = false;
    bool                      stepped_   = false;
    bool                      last_gate_ = false;
    uint32_t                  seed_     = 2463534242u;
};

} // namespace dsp
