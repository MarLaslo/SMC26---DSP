#pragma once

#include "Synthesis/fm2.h"

namespace dsp
{
/** FM synthesis: carrier oscillator frequency-modulated by a modulator
 *  oscillator. Built on top of DaisySP's Fm2.
 *  Usage:
 *      FM voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      float sample = voice.Process();
 */
class FM
{
  public:
    void Init(float sample_rate)
    {
        fm_.Init(sample_rate);
        SetFreq(440.f);
    }

    void SetFreq(float freq_hz) { fm_.SetFrequency(freq_hz); }

    /** Modulator frequency relative to the carrier (1 = same pitch). */
    void SetModRatio(float ratio) { fm_.SetRatio(ratio); }

    /** How much the modulator pushes the carrier's pitch around. */
    void SetModDepth(float depth) { fm_.SetIndex(depth); }

    /** Returns the next audio sample. */
    float Process() { return fm_.Process(); }

  private:
    daisysp::Fm2 fm_;
};

} // namespace dsp
