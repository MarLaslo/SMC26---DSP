#pragma once

#include "Filters/onepole.h"

namespace dsp
{
/** One-pole filter: the simplest, gentlest lowpass or highpass
 *  (6 dB/oct). Good for smoothing a signal or control value, or for
 *  a subtle tone-shaping effect.
 *
 *  To smooth a sensor in the while loop in main() rather than in
 *  AudioCallback, Init() it with 1000.f (the loop runs about 1000
 *  times a second) instead of the sample rate, so SetFreq() is still
 *  in Hz.
 *  Usage:
 *      OnePoleFilter filter;
 *      filter.Init(sample_rate);
 *      filter.SetFreq(800.f);
 *      float sample = filter.Process(input);
 */
class OnePoleFilter
{
  public:
    enum Mode
    {
        MODE_LOWPASS,
        MODE_HIGHPASS,
    };

    void Init(float sample_rate)
    {
        filter_.Init();
        sample_rate_ = sample_rate;
        SetMode(MODE_LOWPASS);
        SetFreq(1000.f);
    }

    /** Cutoff frequency, in Hz. */
    void SetFreq(float freq_hz) { filter_.SetFrequency(freq_hz / sample_rate_); }

    /** Filter response: lowpass or highpass. */
    void SetMode(Mode mode)
    {
        filter_.SetFilterMode(mode == MODE_HIGHPASS
                                   ? daisysp::OnePole::FILTER_MODE_HIGH_PASS
                                   : daisysp::OnePole::FILTER_MODE_LOW_PASS);
    }

    /** Returns the next audio sample. */
    float Process(float in) { return filter_.Process(in); }

  private:
    daisysp::OnePole filter_;
    float             sample_rate_ = 48000.f;
};

} // namespace dsp
