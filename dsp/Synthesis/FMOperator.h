#pragma once

#include <cmath>
#include "Utility/dsp.h"

namespace dsp
{
/** Two-operator phase modulation (DX7-style FM), used inside FM and
 *  FMSimple. Same controls as DaisySP's Fm2, which it replaces: Fm2
 *  adds the modulator into the carrier's phase every sample, so the
 *  real modulation amount grows with sample_rate / modulator pitch.
 *  That made low notes and low ratios far brighter (and noisier) than
 *  high ones. Here the modulator only offsets the carrier's phase, so
 *  SetIndex() sounds the same at every pitch and ratio.
 */
class FMOperator
{
  public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        car_phase_   = 0.f;
        mod_phase_   = 0.f;
        SetFrequency(440.f);
        SetRatio(2.f);
        SetIndex(1.f);
    }

    void SetFrequency(float freq_hz)
    {
        freq_ = fabsf(freq_hz);
        UpdateIncrements();
    }

    /** Modulator frequency relative to the carrier (1 = same pitch). */
    void SetRatio(float ratio)
    {
        ratio_ = fabsf(ratio);
        UpdateIncrements();
    }

    /** Modulation amount. Same scale as Fm2: 1 = moderate, 5 = very
     *  bright (peak phase swing of index * 0.2 cycles). */
    void SetIndex(float index) { idx_ = index * kIdxScalar; }

    float Process()
    {
        float mod = sinf(TWOPI_F * mod_phase_);
        float out = sinf(TWOPI_F * (car_phase_ + idx_ * mod));

        car_phase_ = Wrap(car_phase_ + car_inc_);
        mod_phase_ = Wrap(mod_phase_ + mod_inc_);
        return out;
    }

  private:
    static constexpr float kIdxScalar = 0.2f;

    static float Wrap(float phase) { return phase - floorf(phase); }

    void UpdateIncrements()
    {
        car_inc_ = freq_ / sample_rate_;
        mod_inc_ = freq_ * ratio_ / sample_rate_;
    }

    float sample_rate_ = 48000.f;
    float freq_        = 440.f;
    float ratio_       = 2.f;
    float idx_         = kIdxScalar;
    float car_phase_   = 0.f;
    float mod_phase_   = 0.f;
    float car_inc_     = 0.f;
    float mod_inc_     = 0.f;
};

} // namespace dsp
