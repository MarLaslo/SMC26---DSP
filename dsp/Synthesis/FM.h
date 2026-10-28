#pragma once

#include "Synthesis/fm2.h"
#include "Control/adsr.h"

namespace dsp
{
/** FM synthesis: carrier oscillator frequency-modulated by a modulator
 *  oscillator. Built on top of DaisySP's Fm2, with two envelopes:
 *  one shapes the volume, the other shapes the modulation depth, so
 *  the timbre can open up on the attack and mellow out over the note
 *  (classic FM/DX7-style behavior).
 *  Usage:
 *      FM voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      voice.NoteOn();
 *      float sample = voice.Process();
 *      ...
 *      voice.NoteOff();
 */
class FM
{
  public:
    void Init(float sample_rate)
    {
        fm_.Init(sample_rate);
        SetFreq(440.f);
        SetModRatio(2.f);
        SetModDepth(1.f);

        amp_env_.Init(sample_rate);
        amp_env_.SetAttackTime(0.01f);
        amp_env_.SetDecayTime(0.3f);
        amp_env_.SetSustainLevel(0.8f);
        amp_env_.SetReleaseTime(0.3f);

        mod_env_.Init(sample_rate);
        mod_env_.SetAttackTime(0.01f);
        mod_env_.SetDecayTime(0.15f);
        mod_env_.SetSustainLevel(0.3f);
        mod_env_.SetReleaseTime(0.2f);
    }

    void SetFreq(float freq_hz) { fm_.SetFrequency(freq_hz); }

    /** Modulator frequency relative to the carrier (1 = same pitch). */
    void SetModRatio(float ratio) { fm_.SetRatio(ratio); }

    /** Peak amount the modulator pushes the carrier's pitch around,
     *  reached at the top of the modulation envelope. */
    void SetModDepth(float depth) { mod_depth_ = depth; }

    /** Starts a note: triggers both envelopes from the attack stage.
     *  On a note that's already playing, both start again from their
     *  current level, so every NoteOn() sounds like a new note. */
    void NoteOn()
    {
        gate_      = true;
        retrigger_ = true;
    }

    /** Releases the note: both envelopes move to their release stage. */
    void NoteOff() { gate_ = false; }

    void SetAmpAttack(float time_s) { amp_env_.SetAttackTime(time_s); }
    void SetAmpDecay(float time_s) { amp_env_.SetDecayTime(time_s); }
    void SetAmpSustain(float level) { amp_env_.SetSustainLevel(level); }
    void SetAmpRelease(float time_s) { amp_env_.SetReleaseTime(time_s); }

    void SetModAttack(float time_s) { mod_env_.SetAttackTime(time_s); }
    void SetModDecay(float time_s) { mod_env_.SetDecayTime(time_s); }
    void SetModSustain(float level) { mod_env_.SetSustainLevel(level); }
    void SetModRelease(float time_s) { mod_env_.SetReleaseTime(time_s); }

    /** Returns the next audio sample. */
    float Process()
    {
        // NoteOn() on a note that's already on: restart both attacks
        // from their current level (no click). The envelopes can't spot
        // that themselves, as their gate never went from off to on.
        // Skipped if NoteOff() came straight after.
        if(retrigger_)
        {
            retrigger_ = false;
            if(gate_)
            {
                amp_env_.Retrigger(false);
                mod_env_.Retrigger(false);
            }
        }

        float amp_level = amp_env_.Process(gate_);
        float mod_level = mod_env_.Process(gate_);

        fm_.SetIndex(mod_depth_ * mod_level);
        return fm_.Process() * amp_level;
    }

  private:
    daisysp::Fm2  fm_;
    daisysp::Adsr amp_env_;
    daisysp::Adsr mod_env_;
    float         mod_depth_ = 1.f;
    bool          gate_      = false;
    bool          retrigger_ = false;
};

} // namespace dsp
