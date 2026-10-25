#pragma once

#include "Effects/pitchshifter.h"

namespace dsp
{
/** Pitch shifter: transposes the incoming signal up or down without
 *  changing its speed.
 *  Usage:
 *      PitchShifter shifter;
 *      shifter.Init(sample_rate);
 *      shifter.SetTranspose(-12.f);
 *      float sample = shifter.Process(input);
 */
class PitchShifter
{
  public:
    void Init(float sample_rate)
    {
        shifter_.Init(sample_rate);
        SetTranspose(0.f);
    }

    /** How many semitones to shift the pitch by. Negative is down,
     *  positive is up. */
    void SetTranspose(float semitones) { shifter_.SetTransposition(semitones); }

    /** Amount of random pitch drift mixed in, for a tape-flutter-like
     *  wobble. Range: 0 (none) - 1 (lots). */
    void SetDrift(float amount) { shifter_.SetFun(amount); }

    /** Returns the next audio sample. */
    float Process(float in) { return shifter_.Process(in); }

  private:
    daisysp::PitchShifter shifter_;
};

} // namespace dsp
