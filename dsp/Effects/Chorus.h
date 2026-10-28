#pragma once

#include "Effects/chorus.h"

namespace dsp
{
/** Chorus: layers a modulating, slightly detuned delay over the dry
 *  signal for a thicker, "many voices" sound.
 *  Usage:
 *      Chorus chorus;
 *      chorus.Init(sample_rate);
 *      chorus.SetLfoFreq(0.5f);
 *      chorus.SetLfoDepth(0.8f);
 *      float sample = chorus.Process(input);
 */
class Chorus
{
  public:
    void Init(float sample_rate)
    {
        chorus_.Init(sample_rate);
        SetLfoFreq(0.3f);
        SetLfoDepth(0.5f);
        SetDelay(0.5f);
        SetFeedback(0.f);
    }

    /** Rate of the modulation, in Hz. */
    void SetLfoFreq(float freq_hz) { chorus_.SetLfoFreq(freq_hz); }

    /** How far the modulation travels. Range: 0 - 1. */
    void SetLfoDepth(float depth) { chorus_.SetLfoDepth(depth); }

    /** Base delay time the modulation moves around. Range: 0 - 1
     *  (maps to .1 - 8 ms). */
    void SetDelay(float delay) { chorus_.SetDelay(delay); }

    /** Base delay time in milliseconds, .1 - 50 ms. */
    void SetDelayMs(float ms) { chorus_.SetDelayMs(ms); }

    /** How much delayed signal feeds back into itself. Range: 0 - 1. */
    void SetFeedback(float feedback) { chorus_.SetFeedback(feedback); }

    /** Returns the next audio sample (mono, left channel). */
    float Process(float in) { return chorus_.Process(in); }

  private:
    daisysp::Chorus chorus_;
};

} // namespace dsp
