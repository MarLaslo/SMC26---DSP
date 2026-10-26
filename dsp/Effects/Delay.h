#pragma once

#include "Utility/delayline.h"

namespace dsp
{
/** Delay: a classic echo. Repeats the input after a set time, with each
 *  repeat quieter than the last.
 *  Usage:
 *      Delay delay;
 *      delay.Init(sample_rate);
 *      delay.SetTimeMs(350.f);
 *      delay.SetFeedback(0.4f);
 *      delay.SetMix(0.5f);
 *      float sample = delay.Process(input);
 */
class Delay
{
  public:
    /** Longest delay time this can hold, in seconds. */
    static constexpr float kMaxDelaySeconds = 2.f;
    /** Longest delay time this can hold, assuming a sample rate of up to
     *  96 kHz. */
    static constexpr size_t kMaxDelaySamples
        = static_cast<size_t>(kMaxDelaySeconds * 96000.f);

    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        line_.Init();
        SetTimeMs(350.f);
        SetFeedback(0.4f);
        SetMix(0.5f);
    }

    /** Time between repeats, in milliseconds. Range: 0 - 2000 ms. */
    void SetTimeMs(float ms)
    {
        float samples = ms * 0.001f * sample_rate_;
        if(samples < 1.f)
        {
            samples = 1.f;
        }
        else if(samples > static_cast<float>(kMaxDelaySamples - 1))
        {
            samples = static_cast<float>(kMaxDelaySamples - 1);
        }
        line_.SetDelay(samples);
    }

    /** How much of the repeated signal feeds back into itself, for more
     *  repeats. Range: 0 (single repeat) - 1 (repeats forever). */
    void SetFeedback(float feedback) { feedback_ = feedback; }

    /** Balance between the dry input and the delayed signal. Range: 0
     *  (dry only) - 1 (delayed signal only). */
    void SetMix(float mix) { mix_ = mix; }

    /** Returns the next audio sample. */
    float Process(float in)
    {
        float delayed = line_.Read();
        line_.Write(in + delayed * feedback_);
        return in + (delayed - in) * mix_;
    }

  private:
    float sample_rate_ = 48000.f;
    float feedback_    = 0.4f;
    float mix_         = 0.5f;
    daisysp::DelayLine<float, kMaxDelaySamples> line_;
};

} // namespace dsp
