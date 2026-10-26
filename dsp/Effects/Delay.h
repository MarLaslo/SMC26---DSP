#pragma once

#include <cstddef>
#include "dev/sdram.h"

namespace dsp
{
namespace delay_detail
{
/** Longest delay time this can hold, in seconds. */
constexpr float kMaxDelaySeconds = 1.f;
/** Longest delay time this can hold, assuming a sample rate of up to
 *  48 kHz. */
constexpr size_t kMaxDelaySamples
    = static_cast<size_t>(kMaxDelaySeconds * 48000.f);

/** The echo buffer itself, kept outside the Delay class so the
 *  `section` attribute below can place it in the Daisy's external
 *  SDRAM instead of its scarce internal RAM (that attribute only
 *  works on variables with static storage). */
static float buffer[kMaxDelaySamples] DSY_SDRAM_BSS;
} // namespace delay_detail

/** Delay: a classic echo. Repeats the input after a set time, with each
 *  repeat quieter than the last.
 *
 *  The echo buffer lives in the Daisy's external SDRAM rather than its
 *  small internal RAM, since even a modest delay time needs far more
 *  space than the chip's internal memory has to spare.
 *
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
    static constexpr float kMaxDelaySeconds = delay_detail::kMaxDelaySeconds;
    /** Longest delay time this can hold, assuming a sample rate of up to
     *  48 kHz. */
    static constexpr size_t kMaxDelaySamples = delay_detail::kMaxDelaySamples;

    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        write_ptr_   = 0;
        for(size_t i = 0; i < kMaxDelaySamples; i++)
        {
            delay_detail::buffer[i] = 0.f;
        }
        SetTimeMs(350.f);
        SetFeedback(0.4f);
        SetMix(0.5f);
    }

    /** Time between repeats, in milliseconds. Range: 0 - 1000 ms. */
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
        delay_samples_ = static_cast<size_t>(samples);
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
        size_t read_ptr
            = (write_ptr_ + kMaxDelaySamples - delay_samples_) % kMaxDelaySamples;
        float delayed = delay_detail::buffer[read_ptr];

        delay_detail::buffer[write_ptr_] = in + delayed * feedback_;
        write_ptr_                       = (write_ptr_ + 1) % kMaxDelaySamples;

        return in + (delayed - in) * mix_;
    }

  private:
    float  sample_rate_   = 48000.f;
    float  feedback_      = 0.4f;
    float  mix_           = 0.5f;
    size_t write_ptr_      = 0;
    size_t delay_samples_ = 1;
};

} // namespace dsp
