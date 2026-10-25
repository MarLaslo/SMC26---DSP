#pragma once

#include "Effects/flanger.h"

namespace dsp
{
/** Flanger: mixes a short, modulating delay with the dry signal for a
 *  sweeping, metallic "jet plane" sound.
 *  Usage:
 *      Flanger flanger;
 *      flanger.Init(sample_rate);
 *      flanger.SetLfoFreq(0.3f);
 *      flanger.SetLfoDepth(0.8f);
 *      float sample = flanger.Process(input);
 */
class Flanger
{
  public:
    void Init(float sample_rate)
    {
        flanger_.Init(sample_rate);
        SetLfoFreq(0.3f);
        SetLfoDepth(0.5f);
        SetDelay(0.5f);
        SetFeedback(0.f);
    }

    /** Rate of the sweep, in Hz. */
    void SetLfoFreq(float freq_hz) { flanger_.SetLfoFreq(freq_hz); }

    /** How far the sweep travels. Range: 0 - 1. */
    void SetLfoDepth(float depth) { flanger_.SetLfoDepth(depth); }

    /** Base delay time the sweep moves around. Range: 0 - 1 (maps to
     *  .1 - 7 ms). */
    void SetDelay(float delay) { flanger_.SetDelay(delay); }

    /** Base delay time in milliseconds, .1 - 7 ms. */
    void SetDelayMs(float ms) { flanger_.SetDelayMs(ms); }

    /** How much delayed signal feeds back into itself, for a sharper
     *  effect. Range: 0 - 1. */
    void SetFeedback(float feedback) { flanger_.SetFeedback(feedback); }

    /** Returns the next audio sample. */
    float Process(float in) { return flanger_.Process(in); }

  private:
    daisysp::Flanger flanger_;
};

} // namespace dsp
