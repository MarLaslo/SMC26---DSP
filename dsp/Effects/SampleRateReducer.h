#pragma once

#include "Effects/sampleratereducer.h"

namespace dsp
{
/** Sample rate reducer: holds samples to simulate running at a lower
 *  sample rate, for lo-fi/bitcrush-style artifacts.
 *  Usage:
 *      SampleRateReducer reducer;
 *      reducer.Init();
 *      reducer.SetAmount(0.5f);
 *      float sample = reducer.Process(input);
 */
class SampleRateReducer
{
  public:
    void Init()
    {
        reducer_.Init();
        SetAmount(1.f);
    }

    /** How much to reduce the sample rate. Range: 0 (heavily reduced,
     *  crunchy) - 1 (full quality, no effect). */
    void SetAmount(float amount) { reducer_.SetFreq(amount); }

    /** Returns the next audio sample. */
    float Process(float in) { return reducer_.Process(in); }

  private:
    daisysp::SampleRateReducer reducer_;
};

} // namespace dsp
