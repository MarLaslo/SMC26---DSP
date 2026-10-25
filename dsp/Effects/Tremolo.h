#pragma once

#include "Effects/tremolo.h"
#include "Synthesis/oscillator.h"

namespace dsp
{
/** Tremolo: rhythmically pulses the volume with an LFO.
 *  Usage:
 *      Tremolo trem;
 *      trem.Init(sample_rate);
 *      trem.SetFreq(4.f);
 *      trem.SetDepth(0.8f);
 *      float sample = trem.Process(input);
 */
class Tremolo
{
  public:
    enum Waveform
    {
        WAVE_SINE,
        WAVE_TRIANGLE,
        WAVE_SQUARE,
    };

    void Init(float sample_rate)
    {
        trem_.Init(sample_rate);
        SetFreq(4.f);
        SetDepth(0.5f);
        SetWaveform(WAVE_SINE);
    }

    /** Rate of the volume pulsing, in Hz. */
    void SetFreq(float freq_hz) { trem_.SetFreq(freq_hz); }

    /** How much the volume is modulated. Range: 0 (no effect) - 1 (full
     *  on/off pulsing). */
    void SetDepth(float depth) { trem_.SetDepth(depth); }

    /** Shape of the modulating LFO. */
    void SetWaveform(Waveform wave)
    {
        switch(wave)
        {
            case WAVE_TRIANGLE:
                trem_.SetWaveform(daisysp::Oscillator::WAVE_TRI);
                break;
            case WAVE_SQUARE:
                trem_.SetWaveform(daisysp::Oscillator::WAVE_SQUARE);
                break;
            case WAVE_SINE:
            default:
                trem_.SetWaveform(daisysp::Oscillator::WAVE_SIN);
                break;
        }
    }

    /** Returns the next audio sample. */
    float Process(float in) { return trem_.Process(in); }

  private:
    daisysp::Tremolo trem_;
};

} // namespace dsp
