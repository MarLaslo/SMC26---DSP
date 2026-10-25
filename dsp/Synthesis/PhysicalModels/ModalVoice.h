#pragma once

#include "PhysicalModeling/modalvoice.h"

namespace dsp
{
/** Physical modeling synthesis: a struck resonant body, built on top
 *  of DaisySP's ModalVoice (click/noise exciter -> modal resonator).
 *  Usage:
 *      ModalVoice voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      voice.Strike();
 *      float sample = voice.Process();
 */
class ModalVoice
{
  public:
    void Init(float sample_rate)
    {
        modal_.Init(sample_rate);
        SetFreq(440.f);
        SetStructure(0.5f);
        SetBrightness(0.5f);
        SetDamping(0.5f);
        SetAccent(0.5f);
    }

    void SetFreq(float freq_hz) { modal_.SetFreq(freq_hz); }

    /** Changes the resonator's general character (stiffness), 0-1. */
    void SetStructure(float structure) { modal_.SetStructure(structure); }

    /** Tone color of the resonator and the noise exciter's density, 0-1. */
    void SetBrightness(float brightness) { modal_.SetBrightness(brightness); }

    /** How long the resonant body takes to decay, 0-1. */
    void SetDamping(float damping) { modal_.SetDamping(damping); }

    /** How hard the next strike hits, 0-1. */
    void SetAccent(float accent) { modal_.SetAccent(accent); }

    /** Strikes the resonator, like a mallet hit. */
    void Strike() { modal_.Trig(); }

    /** Keeps exciting the resonator with continuous noise instead of
     *  discrete strikes, for a bowed/blown character. */
    void SetSustain(bool sustain) { modal_.SetSustain(sustain); }

    /** Returns the next audio sample. */
    float Process() { return modal_.Process(); }

  private:
    daisysp::ModalVoice modal_;
};

} // namespace dsp
