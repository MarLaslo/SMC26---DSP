#pragma once

#include "PhysicalModeling/stringvoice.h"

namespace dsp
{
/** Physical modeling synthesis: an extended plucked string, built on
 *  top of DaisySP's StringVoice (Karplus-Strong with the extra
 *  character knobs from Rings).
 *  Usage:
 *      StringVoice voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      voice.Strike();
 *      float sample = voice.Process();
 */
class StringVoice
{
  public:
    void Init(float sample_rate)
    {
        string_.Init(sample_rate);
        SetFreq(440.f);
        SetStructure(0.5f);
        SetBrightness(0.5f);
        SetDamping(0.5f);
        SetAccent(0.5f);
    }

    void SetFreq(float freq_hz) { string_.SetFreq(freq_hz); }

    /** The string's nonlinearity (string type), 0-1: 0-0.26 is a
     *  curved bridge, 0.26-1 is dispersion. */
    void SetStructure(float structure) { string_.SetStructure(structure); }

    /** Tone color of the string and the noise exciter's density, 0-1. */
    void SetBrightness(float brightness) { string_.SetBrightness(brightness); }

    /** How long the string takes to decay, relative to the accent
     *  level, 0-1 (full damp only reached with full accent). */
    void SetDamping(float damping) { string_.SetDamping(damping); }

    /** How hard the next strike hits, 0-1. */
    void SetAccent(float accent) { string_.SetAccent(accent); }

    /** Strikes the string, like a pluck. */
    void Strike() { string_.Trig(); }

    /** Keeps exciting the string with continuous noise instead of
     *  discrete strikes, for a bowed character. */
    void SetSustain(bool sustain) { string_.SetSustain(sustain); }

    /** Returns the next audio sample. */
    float Process() { return string_.Process(); }

  private:
    daisysp::StringVoice string_;
};

} // namespace dsp
