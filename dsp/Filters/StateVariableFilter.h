#pragma once

#include "Filters/svf.h"

namespace dsp
{
/** State variable filter: a versatile filter that can act as a
 *  lowpass, highpass, bandpass, notch, or peak filter, with resonance
 *  and drive.
 *  Usage:
 *      StateVariableFilter filter;
 *      filter.Init(sample_rate);
 *      filter.SetFreq(800.f);
 *      filter.SetResonance(0.3f);
 *      filter.SetMode(StateVariableFilter::MODE_BANDPASS);
 *      float sample = filter.Process(input);
 */
class StateVariableFilter
{
  public:
    enum Mode
    {
        MODE_LOWPASS,
        MODE_HIGHPASS,
        MODE_BANDPASS,
        MODE_NOTCH,
        MODE_PEAK,
    };

    void Init(float sample_rate)
    {
        filter_.Init(sample_rate);
        SetMode(MODE_LOWPASS);
        SetFreq(1000.f);
        SetResonance(0.f);
        SetDrive(0.f);
    }

    /** Cutoff/center frequency, in Hz. */
    void SetFreq(float freq_hz) { filter_.SetFreq(freq_hz); }

    /** Resonance / emphasis around the cutoff. Range: 0 - 1. */
    void SetResonance(float resonance) { filter_.SetRes(resonance); }

    /** Amount of drive/saturation into the filter. */
    void SetDrive(float drive) { filter_.SetDrive(drive); }

    /** Which output of the filter to use. */
    void SetMode(Mode mode) { mode_ = mode; }

    /** Returns the next audio sample. */
    float Process(float in)
    {
        filter_.Process(in);
        switch(mode_)
        {
            case MODE_HIGHPASS: return filter_.High();
            case MODE_BANDPASS: return filter_.Band();
            case MODE_NOTCH: return filter_.Notch();
            case MODE_PEAK: return filter_.Peak();
            case MODE_LOWPASS:
            default: return filter_.Low();
        }
    }

  private:
    daisysp::Svf filter_;
    Mode         mode_ = MODE_LOWPASS;
};

} // namespace dsp
