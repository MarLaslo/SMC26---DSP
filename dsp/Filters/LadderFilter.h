#pragma once

#include "Filters/ladder.h"

namespace dsp
{
/** Moog-style ladder filter: a classic resonant filter that can self
 *  oscillate at high resonance. Available as lowpass, bandpass, or
 *  highpass, each at a gentler 12 dB/oct or steeper 24 dB/oct slope.
 *  Usage:
 *      LadderFilter filter;
 *      filter.Init(sample_rate);
 *      filter.SetFreq(800.f);
 *      filter.SetResonance(0.5f);
 *      float sample = filter.Process(input);
 */
class LadderFilter
{
  public:
    enum Mode
    {
        MODE_LOWPASS_24,
        MODE_LOWPASS_12,
        MODE_BANDPASS_24,
        MODE_BANDPASS_12,
        MODE_HIGHPASS_24,
        MODE_HIGHPASS_12,
    };

    void Init(float sample_rate)
    {
        filter_.Init(sample_rate);
        SetMode(MODE_LOWPASS_24);
        SetFreq(1000.f);
        SetResonance(0.f);
        SetDrive(1.f);
    }

    /** Cutoff frequency, in Hz. Range: 5 - sample_rate / 2. */
    void SetFreq(float freq_hz) { filter_.SetFreq(freq_hz); }

    /** Resonance / emphasis around the cutoff. Range: 0 - 1.8 (filter
     *  self-oscillates at the top of that range). */
    void SetResonance(float resonance) { filter_.SetRes(resonance); }

    /** Amount of input drive/saturation into the filter. Range: 0 - 4,
     *  where 1 is unity gain (no added drive) and 0 mutes the input. */
    void SetDrive(float drive) { filter_.SetInputDrive(drive); }

    /** Filter response: lowpass, bandpass or highpass, each at two
     *  different slopes. */
    void SetMode(Mode mode)
    {
        switch(mode)
        {
            case MODE_LOWPASS_12:
                filter_.SetFilterMode(daisysp::LadderFilter::FilterMode::LP12);
                break;
            case MODE_BANDPASS_24:
                filter_.SetFilterMode(daisysp::LadderFilter::FilterMode::BP24);
                break;
            case MODE_BANDPASS_12:
                filter_.SetFilterMode(daisysp::LadderFilter::FilterMode::BP12);
                break;
            case MODE_HIGHPASS_24:
                filter_.SetFilterMode(daisysp::LadderFilter::FilterMode::HP24);
                break;
            case MODE_HIGHPASS_12:
                filter_.SetFilterMode(daisysp::LadderFilter::FilterMode::HP12);
                break;
            case MODE_LOWPASS_24:
            default:
                filter_.SetFilterMode(daisysp::LadderFilter::FilterMode::LP24);
                break;
        }
    }

    /** Returns the next audio sample. */
    float Process(float in) { return filter_.Process(in); }

  private:
    daisysp::LadderFilter filter_;
};

} // namespace dsp
