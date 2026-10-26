#pragma once

#include "Filters/soap.h"

namespace dsp
{
/** Second-order all-pass filter, configurable as a bandpass or
 *  band-reject (notch) filter around a center frequency.
 *  Usage:
 *      AllPassFilter filter;
 *      filter.Init(sample_rate);
 *      filter.SetFreq(800.f);
 *      filter.SetBandwidth(0.5f);
 *      filter.SetMode(AllPassFilter::MODE_BANDPASS);
 *      float sample = filter.Process(input);
 */
class AllPassFilter
{
  public:
    enum Mode
    {
        MODE_BANDPASS,
        MODE_BANDREJECT,
    };

    void Init(float sample_rate)
    {
        filter_.Init(sample_rate);
        SetMode(MODE_BANDPASS);
        SetFreq(1000.f);
        SetBandwidth(0.5f);
    }

    /** Center frequency, in Hz. */
    void SetFreq(float freq_hz) { filter_.SetCenterFreq(freq_hz); }

    /** Width of the band around the center frequency. */
    void SetBandwidth(float bandwidth) { filter_.SetFilterBandwidth(bandwidth); }

    /** Which output of the filter to use. */
    void SetMode(Mode mode) { mode_ = mode; }

    /** Returns the next audio sample. */
    float Process(float in)
    {
        filter_.Process(in);
        return mode_ == MODE_BANDREJECT ? filter_.Bandreject() : filter_.Bandpass();
    }

  private:
    daisysp::Soap filter_;
    Mode          mode_ = MODE_BANDPASS;
};

} // namespace dsp
