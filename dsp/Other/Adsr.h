#pragma once

#include "Control/adsr.h"

namespace dsp
{
/** ADSR envelope: a volume (or modulation) shape with four stages -
 *  Attack, Decay, Sustain, Release. Useful on its own for driving a
 *  filter cutoff, amplitude, or any other parameter over time.
 *  Usage:
 *      Adsr env;
 *      env.Init(sample_rate);
 *      env.SetAttack(0.01f);
 *      env.SetDecay(0.2f);
 *      env.SetSustain(0.7f);
 *      env.SetRelease(0.3f);
 *      env.NoteOn();
 *      float level = env.Process();
 *      ...
 *      env.NoteOff();
 */
class Adsr
{
  public:
    void Init(float sample_rate)
    {
        env_.Init(sample_rate);
        SetAttack(0.01f);
        SetDecay(0.2f);
        SetSustain(0.7f);
        SetRelease(0.3f);
    }

    /** Starts the envelope from its attack stage. If it's already
     *  running, the attack starts again from the current level. */
    void NoteOn()
    {
        gate_      = true;
        retrigger_ = true;
    }

    /** Moves the envelope to its release stage. */
    void NoteOff() { gate_ = false; }

    /** How long the envelope takes to reach full level, in seconds. */
    void SetAttack(float time_s) { env_.SetAttackTime(time_s); }

    /** How long the envelope takes to fall from full level to the
     *  sustain level, in seconds. */
    void SetDecay(float time_s) { env_.SetDecayTime(time_s); }

    /** Level the envelope holds at while the note is held. Range: 0 - 1. */
    void SetSustain(float level) { env_.SetSustainLevel(level); }

    /** How long the envelope takes to fall to 0 after the note is
     *  released, in seconds. */
    void SetRelease(float time_s) { env_.SetReleaseTime(time_s); }

    /** True while the envelope is anywhere but fully idle at 0. */
    bool IsRunning() { return env_.IsRunning(); }

    /** Returns the next envelope value, 0 - 1. */
    float Process()
    {
        // NoteOn() while the envelope is already on: restart the attack
        // from the current level (no jump). daisysp::Adsr can't spot
        // that itself, as its gate never went from off to on. Skipped
        // if NoteOff() came straight after.
        if(retrigger_)
        {
            retrigger_ = false;
            if(gate_)
                env_.Retrigger(false);
        }
        return env_.Process(gate_);
    }

  private:
    daisysp::Adsr env_;
    bool          gate_      = false;
    bool          retrigger_ = false;
};

} // namespace dsp
