#pragma once

#include <cmath>
#include <cstddef>
#include "../Utility.h"
#include "Utility/delayline.h"
#include "Utility/dsp.h"

namespace dsp
{
/** Delay: a classic echo. Repeats the input after a set time, with each
 *  repeat quieter than the last.
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
    static constexpr float kMaxDelaySeconds = 1.f;
    /** Longest delay time this can hold, assuming a sample rate of up to
     *  48 kHz. */
    static constexpr size_t kMaxDelaySamples
        = static_cast<size_t>(kMaxDelaySeconds * 48000.f);

    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        // Time constant for smoothing the delay time (see SetTimeMs).
        smooth_coeff_ = 1.f - expf(-1.f / (0.12f * sample_rate_));
        line_.Init();
        SetTimeMs(350.f);
        current_samples_ = target_samples_;
        SetFeedback(0.7f);
        SetMix(0.5f);
    }

    /** Time between repeats, in milliseconds. Range: 0 - 1000 ms.
     *
     *  Expected to be called continuously (e.g. once per control-loop
     *  tick from a knob), so the actual delay time glides smoothly
     *  toward this target rather than snapping to it - snapping would
     *  jump the read position by a sample or more on every call, which
     *  clicks audibly even from the small jitter a knob reading has
     *  while sitting still. The glide is also what gives a "tape
     *  echo" its pitch-bend character when you sweep the time by hand. */
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
        target_samples_ = samples;
    }

    /** How much of the repeated signal feeds back into itself, for more
     *  repeats. Range: 0 (single repeat) - 1 (repeats forever).
     *
     *  Capped just under 1: at exactly 1 the loop's gain (roughly
     *  1 / (1 - feedback) for a held, continuous input) diverges, so
     *  a sustained note builds up without bound the longer it's held,
     *  eventually overwhelming SoftClip's headroom and distorting. */
    void SetFeedback(float feedback) { feedback_ = Clamp(feedback, 0.f, 0.97f); }

    /** Balance between the dry input and the delayed signal. Range: 0
     *  (dry only) - 1 (delayed signal only). */
    void SetMix(float mix) { mix_ = mix; }

    /** Returns the next audio sample. */
    float Process(float in)
    {
        current_samples_ += (target_samples_ - current_samples_) * smooth_coeff_;
        line_.SetDelay(current_samples_);

        float delayed = line_.Read();
        line_.Write(in + delayed * feedback_);

        float wet = mix_ * delayed;
        float dry = (1.f - mix_) * in;
        return daisysp::SoftClip(dry + wet);
    }

  private:
    float                                        sample_rate_     = 48000.f;
    float                                        feedback_        = 0.7f;
    float                                        mix_             = 0.5f;
    float                                        target_samples_  = 1.f;
    float                                        current_samples_ = 1.f;
    float                                        smooth_coeff_    = 1.f;
    daisysp::DelayLine<float, kMaxDelaySamples> line_;
};

} // namespace dsp
