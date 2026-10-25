#pragma once

#include "Drums/synthbassdrum.h"
#include "Drums/synthsnaredrum.h"
#include "Drums/hihat.h"

namespace dsp
{
/** Digital-style bass drum (909-ish kick), built on top of DaisySP's
 *  SyntheticBassDrum: an FM'd sine body plus click and attack noise
 *  layers.
 *  Usage:
 *      SynthBassDrum kick;
 *      kick.Init(sample_rate);
 *      kick.SetFreq(55.f);
 *      kick.Hit();
 *      float sample = kick.Process();
 */
class SynthBassDrum
{
  public:
    void Init(float sample_rate)
    {
        drum_.Init(sample_rate);
        SetFreq(55.f);
        SetTone(0.5f);
        SetDecay(0.5f);
        SetDirtiness(0.3f);
        SetFmEnvelopeAmount(0.5f);
        SetFmEnvelopeDecay(0.5f);
        SetAccent(0.5f);
        SetSustain(false);
    }

    /** Hits the drum. */
    void Hit() { drum_.Trig(); }

    /** Root (fundamental) frequency of the drum body, Hz. */
    void SetFreq(float freq_hz) { drum_.SetFreq(freq_hz); }

    /** Overall brightness / darkness of the drum, 0-1. */
    void SetTone(float tone) { drum_.SetTone(tone); }

    /** How long the hit's volume takes to decay, 0-1. */
    void SetDecay(float decay) { drum_.SetDecay(decay); }

    /** How grimy/distorted the body tone gets, 0-1. */
    void SetDirtiness(float dirtiness) { drum_.SetDirtiness(dirtiness); }

    /** How far the pitch sweeps on attack, 0-1. */
    void SetFmEnvelopeAmount(float amount) { drum_.SetFmEnvelopeAmount(amount); }

    /** How long the initial pitch sweep takes, 0-1. */
    void SetFmEnvelopeDecay(float decay) { drum_.SetFmEnvelopeDecay(decay); }

    /** How hard the next hit lands, 0-1. */
    void SetAccent(float accent) { drum_.SetAccent(accent); }

    /** Keeps the drum ringing out forever instead of decaying. */
    void SetSustain(bool sustain) { drum_.SetSustain(sustain); }

    /** Returns the next audio sample. */
    float Process() { return drum_.Process(); }

  private:
    daisysp::SyntheticBassDrum drum_;
};

/** Digital-style snare drum (909-ish snare), built on top of DaisySP's
 *  SyntheticSnareDrum: two FM'd sine oscillators mixed with filtered
 *  noise for the "snap".
 *  Usage:
 *      SynthSnareDrum snare;
 *      snare.Init(sample_rate);
 *      snare.SetFreq(220.f);
 *      snare.Hit();
 *      float sample = snare.Process();
 */
class SynthSnareDrum
{
  public:
    void Init(float sample_rate)
    {
        drum_.Init(sample_rate);
        SetFreq(220.f);
        SetFmAmount(0.2f);
        SetDecay(0.5f);
        SetSnappy(0.7f);
        SetAccent(0.5f);
        SetSustain(false);
    }

    /** Hits the drum. */
    void Hit() { drum_.Trig(); }

    /** Root (fundamental) frequency of the drum's two tones, Hz. */
    void SetFreq(float freq_hz) { drum_.SetFreq(freq_hz); }

    /** Amount of pitch sweep on each hit, 0-1. */
    void SetFmAmount(float amount) { drum_.SetFmAmount(amount); }

    /** How long the hit takes to decay. */
    void SetDecay(float decay) { drum_.SetDecay(decay); }

    /** Mix between the drum tone and the noise "snap",
     *  0 = just the drum, 1 = just the snare. */
    void SetSnappy(float snappy) { drum_.SetSnappy(snappy); }

    /** How hard the next hit lands, 0-1. */
    void SetAccent(float accent) { drum_.SetAccent(accent); }

    /** Keeps the drum ringing out forever instead of decaying. */
    void SetSustain(bool sustain) { drum_.SetSustain(sustain); }

    /** Returns the next audio sample. */
    float Process() { return drum_.Process(); }

  private:
    daisysp::SyntheticSnareDrum drum_;
};

/** Hi-hat, built on top of DaisySP's HiHat: six square oscillators
 *  make a "metallic noise" that's band-passed and gated by an
 *  envelope, 808-style.
 *  Usage:
 *      HiHat hat;
 *      hat.Init(sample_rate);
 *      hat.SetDecay(0.2f);
 *      hat.Hit();
 *      float sample = hat.Process();
 */
class HiHat
{
  public:
    void Init(float sample_rate)
    {
        hat_.Init(sample_rate);
        SetFreq(3000.f);
        SetTone(0.5f);
        SetDecay(0.2f);
        SetNoisiness(0.8f);
        SetAccent(0.8f);
        SetSustain(false);
    }

    /** Hits the hi-hat. */
    void Hit() { hat_.Trig(); }

    /** Root frequency the metallic noise is built from, Hz. */
    void SetFreq(float freq_hz) { hat_.SetFreq(freq_hz); }

    /** Overall brightness / darkness, 0-1. */
    void SetTone(float tone) { hat_.SetTone(tone); }

    /** How long the hit takes to decay, 0-1. */
    void SetDecay(float decay) { hat_.SetDecay(decay); }

    /** Mix between tone and plain noise, 0 = just tone, 1 = just noise. */
    void SetNoisiness(float noisiness) { hat_.SetNoisiness(noisiness); }

    /** How hard the next hit lands, 0-1. */
    void SetAccent(float accent) { hat_.SetAccent(accent); }

    /** Keeps the hi-hat ringing out forever instead of decaying (an
     *  open, "cymbal-like" hold). */
    void SetSustain(bool sustain) { hat_.SetSustain(sustain); }

    /** Returns the next audio sample. */
    float Process() { return hat_.Process(); }

  private:
    daisysp::HiHat<> hat_;
};

} // namespace dsp
