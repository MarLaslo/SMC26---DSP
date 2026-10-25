#pragma once

#include <cmath>
#include "Synthesis/oscillator.h"
#include "Control/adsr.h"

namespace dsp
{
/** Additive synthesis: sums a stack of harmonically related
 *  sine oscillators to build up a more complex tone. Harmonic n
 *  (1-indexed) sits at freq * ratio[n] (ratio[n] defaults to n) with
 *  amplitude rolloff^(n-1), normalized so the stack's total amplitude
 *  stays around 1.
 *  Usage:
 *      Additive voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      voice.SetHarmonicRatio(2, 2.76f); // make the 3rd partial inharmonic
 *      voice.NoteOn();
 *      float sample = voice.Process();
 *      ...
 *      voice.NoteOff();
 */
class Additive
{
  public:
    static constexpr int kMaxHarmonics = 16;

    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;

        for(int h = 0; h < kMaxHarmonics; h++)
        {
            harmonics_[h].Init(sample_rate);
            harmonics_[h].SetWaveform(daisysp::Oscillator::WAVE_SIN);
            ratios_[h] = static_cast<float>(h + 1);
        }

        SetFreq(440.f);
        UpdateAmplitudes();

        amp_env_.Init(sample_rate);
        amp_env_.SetAttackTime(0.01f);
        amp_env_.SetDecayTime(0.3f);
        amp_env_.SetSustainLevel(0.8f);
        amp_env_.SetReleaseTime(0.3f);
    }

    /** Starts a note: triggers the amp envelope from the attack stage. */
    void NoteOn() { gate_ = true; }

    /** Releases the note: the amp envelope moves to its release stage. */
    void NoteOff() { gate_ = false; }

    void SetAmpAttack(float time_s) { amp_env_.SetAttackTime(time_s); }
    void SetAmpDecay(float time_s) { amp_env_.SetDecayTime(time_s); }
    void SetAmpSustain(float level) { amp_env_.SetSustainLevel(level); }
    void SetAmpRelease(float time_s) { amp_env_.SetReleaseTime(time_s); }

    void SetFreq(float freq_hz)
    {
        freq_ = freq_hz;
        for(int h = 0; h < kMaxHarmonics; h++)
            harmonics_[h].SetFreq(freq_ * ratios_[h]);
    }

    /** Overrides harmonic h's (0-indexed) frequency ratio relative to
     *  the base freq, so it no longer has to sit at the integer
     *  multiple h+1. Takes effect immediately, tracking SetFreq(). */
    void SetHarmonicRatio(int h, float ratio)
    {
        if(h < 0 || h >= kMaxHarmonics)
            return;
        ratios_[h] = ratio;
        harmonics_[h].SetFreq(freq_ * ratios_[h]);
    }

    void SetNumHarmonics(int num_harmonics)
    {
        if(num_harmonics < 1)
            num_harmonics = 1;
        if(num_harmonics > kMaxHarmonics)
            num_harmonics = kMaxHarmonics;
        num_harmonics_ = num_harmonics;
        UpdateAmplitudes();
    }

    void SetRolloff(float rolloff)
    {
        rolloff_ = rolloff;
        UpdateAmplitudes();
    }

    /** Returns the next audio sample. */
    float Process()
    {
        float sum = 0.f;
        for(int h = 0; h < num_harmonics_; h++)
            sum += harmonics_[h].Process();
        return sum * amp_env_.Process(gate_);
    }

  private:
    /** Recomputes each active harmonic's amplitude so the stack's
     *  total amplitude stays normalized as num_harmonics_/rolloff_ change. */
    void UpdateAmplitudes()
    {
        float sum = 0.f;
        for(int h = 0; h < num_harmonics_; h++)
            sum += std::pow(rolloff_, static_cast<float>(h));

        if(sum < 1e-9f)
            sum = 1e-9f;

        for(int h = 0; h < num_harmonics_; h++)
            harmonics_[h].SetAmp(std::pow(rolloff_, static_cast<float>(h)) / sum);
    }

    daisysp::Oscillator harmonics_[kMaxHarmonics];
    float               ratios_[kMaxHarmonics];
    daisysp::Adsr       amp_env_;
    float               sample_rate_   = 48000.f;
    float               freq_          = 440.f;
    int                 num_harmonics_ = 8;
    float               rolloff_       = 0.5f;
    bool                gate_          = false;
};

} // namespace dsp
