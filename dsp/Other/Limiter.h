#pragma once

#include "Dynamics/limiter.h"

namespace dsp
{
/** Peak limiter: smoothly clamps the signal so it never goes above
 *  0 dBFS, without the harsh clicks of plain clipping. Useful as a
 *  final safety net on the output, or to tame a signal before it hits
 *  another effect.
 *  Usage:
 *      Limiter lim;
 *      lim.Init();
 *      lim.SetPreGain(1.f);
 *      float sample = lim.Process(input);
 */
class Limiter
{
  public:
    void Init() { lim_.Init(); }

    /** Gain applied before limiting. Raise it to drive the limiter
     *  harder; 1 = no extra gain. */
    void SetPreGain(float pre_gain) { pre_gain_ = pre_gain; }

    /** Returns the next audio sample. */
    float Process(float in)
    {
        float sample = in;
        lim_.ProcessBlock(&sample, 1, pre_gain_);
        return sample;
    }

  private:
    daisysp::Limiter lim_;
    float             pre_gain_ = 1.f;
};

} // namespace dsp
