#pragma once

#include "Control/adenv.h"

namespace dsp
{
/** AD envelope: a one-shot envelope with just Attack and Decay (no
 *  sustain), triggered on each hit. Handy for percussive sounds or
 *  "zap" style modulation (e.g. sweeping a filter cutoff on each
 *  note).
 *  Usage:
 *      AdEnvelope env;
 *      env.Init(sample_rate);
 *      env.SetAttack(0.01f);
 *      env.SetDecay(0.3f);
 *      env.Hit();
 *      float level = env.Process();
 */
class AdEnvelope
{
  public:
    void Init(float sample_rate)
    {
        env_.Init(sample_rate);
        SetAttack(0.01f);
        SetDecay(0.3f);
        SetMin(0.f);
        SetMax(1.f);
    }

    /** Starts (or restarts) the envelope from its attack stage. */
    void Hit() { env_.Trigger(); }

    /** How long the envelope takes to reach its max value, in seconds. */
    void SetAttack(float time_s) { env_.SetTime(daisysp::ADENV_SEG_ATTACK, time_s); }

    /** How long the envelope takes to fall back to its min value, in
     *  seconds. */
    void SetDecay(float time_s) { env_.SetTime(daisysp::ADENV_SEG_DECAY, time_s); }

    /** Value the envelope sits at when idle / at the start of attack. */
    void SetMin(float min) { env_.SetMin(min); }

    /** Value the envelope reaches at the peak of attack. */
    void SetMax(float max) { env_.SetMax(max); }

    /** True while the envelope is running (anywhere but idle). */
    bool IsRunning() { return env_.IsRunning(); }

    /** Returns the next envelope value. */
    float Process() { return env_.Process(); }

  private:
    daisysp::AdEnv env_;
};

} // namespace dsp
