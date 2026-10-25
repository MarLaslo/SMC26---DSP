#pragma once

#include "Effects/phaser.h"

namespace dsp
{
/** Phaser: sweeps a series of allpass filters to create moving,
 *  swooshing notches in the sound.
 *  Usage:
 *      Phaser phaser;
 *      phaser.Init(sample_rate);
 *      phaser.SetLfoFreq(0.5f);
 *      phaser.SetLfoDepth(0.8f);
 *      float sample = phaser.Process(input);
 */
class Phaser
{
  public:
    void Init(float sample_rate)
    {
        phaser_.Init(sample_rate);
        SetPoles(4);
        SetLfoFreq(0.3f);
        SetLfoDepth(0.5f);
        SetFreq(1500.f);
        SetFeedback(0.f);
    }

    /** Number of allpass stages, which thickens the effect. Range: 1 - 8. */
    void SetPoles(int poles) { phaser_.SetPoles(poles); }

    /** Rate of the sweep, in Hz. */
    void SetLfoFreq(float freq_hz) { phaser_.SetLfoFreq(freq_hz); }

    /** How far the sweep travels. Range: 0 - 1. */
    void SetLfoDepth(float depth) { phaser_.SetLfoDepth(depth); }

    /** Center frequency the sweep moves around, in Hz. */
    void SetFreq(float freq_hz) { phaser_.SetFreq(freq_hz); }

    /** Feedback amount, for a sharper, more resonant sweep. Range: 0 - 1. */
    void SetFeedback(float feedback) { phaser_.SetFeedback(feedback); }

    /** Returns the next audio sample. */
    float Process(float in) { return phaser_.Process(in); }

  private:
    daisysp::Phaser phaser_;
};

} // namespace dsp
