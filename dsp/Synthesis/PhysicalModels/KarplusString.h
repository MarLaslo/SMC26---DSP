#pragma once

#include "PhysicalModeling/KarplusString.h"
#include "Noise/whitenoise.h"

namespace dsp
{
/** Physical modeling synthesis: a plucked string, built on top of
 *  DaisySP's String (Karplus-Strong style comb filter). Plucking
 *  feeds the string a short burst of noise, which the string's
 *  delay-line loop then colors and lets decay over time.
 *  Usage:
 *      KarplusString voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      voice.Pluck();
 *      float sample = voice.Process();
 */
class KarplusString
{
  public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;

        string_.Init(sample_rate);
        SetFreq(440.f);
        SetDamping(0.5f);
        SetBrightness(0.5f);
        SetNonLinearity(0.f);

        noise_.Init();

        pluck_samples_total_ = static_cast<int>(0.003f * sample_rate);
    }

    void SetFreq(float freq_hz) { string_.SetFreq(freq_hz); }

    /** Decay time: 0 dies away quickly, 1 rings out the longest. */
    void SetDamping(float damping) { string_.SetDamping(damping); }

    /** Tone color of the string, 0-1. */
    void SetBrightness(float brightness) { string_.SetBrightness(brightness); }

    /** -1 to 0 is a curved bridge, 0 to 1 is dispersion. */
    void SetNonLinearity(float amount) { string_.SetNonLinearity(amount); }

    /** Excites the string with a short burst of noise, like a pluck. */
    void Pluck(float velocity = 1.f)
    {
        pluck_velocity_   = velocity;
        pluck_samples_left_ = pluck_samples_total_;
    }

    /** Returns the next audio sample. */
    float Process()
    {
        float excitation = 0.f;
        if(pluck_samples_left_ > 0)
        {
            excitation = noise_.Process() * pluck_velocity_;
            pluck_samples_left_--;
        }
        return string_.Process(excitation);
    }

  private:
    daisysp::String     string_;
    daisysp::WhiteNoise noise_;
    float               sample_rate_          = 48000.f;
    float               pluck_velocity_       = 1.f;
    int                 pluck_samples_total_  = 144;
    int                 pluck_samples_left_   = 0;
};

} // namespace dsp
