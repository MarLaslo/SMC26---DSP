#pragma once

#include "Synthesis/fm2.h"
#include "Control/adsr.h"

namespace dsp
{
/** FM synthesis, simplified: one shared envelope shapes the note, and
 *  a single Brightness knob controls how much the modulation depth
 *  follows that envelope. Brightness = 0 keeps the tone constant for
 *  the whole note; Brightness = 1 gives the full "bright attack,
 *  mellower sustain" FM character.
 *  Usage:
 *      FMSimple voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      voice.NoteOn();
 *      float sample = voice.Process();
 *      ...
 *      voice.NoteOff();
 */
class FMSimple
{
  public:
    void Init(float sample_rate)
    {
        fm_.Init(sample_rate);
        SetFreq(440.f);
        SetModDepth(1.f);
        SetBrightness(0.5f);

        env_.Init(sample_rate);
        env_.SetAttackTime(0.01f);
        env_.SetDecayTime(0.3f);
        env_.SetSustainLevel(0.8f);
        env_.SetReleaseTime(0.3f);
    }

    void SetFreq(float freq_hz) { fm_.SetFrequency(freq_hz); }

    /** Modulator frequency relative to the carrier (1 = same pitch). */
    void SetModRatio(float ratio) { fm_.SetRatio(ratio); }

    /** Peak amount the modulator pushes the carrier's pitch around. */
    void SetModDepth(float depth) { mod_depth_ = depth; }

    /** 0 = tone stays constant, 1 = tone follows the envelope (bright
     *  attack, mellower sustain). */
    void SetBrightness(float amount) { brightness_ = amount; }

    /** Starts a note: triggers the envelope from the attack stage. */
    void NoteOn() { gate_ = true; }

    /** Releases the note: the envelope moves to its release stage. */
    void NoteOff() { gate_ = false; }

    void SetAttack(float time_s) { env_.SetAttackTime(time_s); }
    void SetDecay(float time_s) { env_.SetDecayTime(time_s); }
    void SetSustain(float level) { env_.SetSustainLevel(level); }
    void SetRelease(float time_s) { env_.SetReleaseTime(time_s); }

    /** Returns the next audio sample. */
    float Process()
    {
        float env_level = env_.Process(gate_);
        float index      = mod_depth_ * (1.f - brightness_ + brightness_ * env_level);

        fm_.SetIndex(index);
        return fm_.Process() * env_level;
    }

  private:
    daisysp::Fm2  fm_;
    daisysp::Adsr env_;
    float         mod_depth_  = 1.f;
    float         brightness_ = 0.5f;
    bool          gate_       = false;
};

} // namespace dsp
