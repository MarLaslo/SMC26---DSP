#pragma once

#include "Synthesis/oscillator.h"

namespace dsp
{
/** A slow, free-running oscillator for modulating a parameter instead
 *  of making sound - vibrato on a pitch, a wobbling filter cutoff, a
 *  tremolo on amplitude. Unlike dsp::Oscillator it has no amp envelope
 *  or NoteOn/NoteOff - an LFO just keeps cycling on its own for as
 *  long as the patch runs.
 *
 *  Process() returns a bipolar value in -1..1; multiply it by how far
 *  you want the parameter to swing (the modulation "depth") and add it
 *  to the parameter's base value.
 *
 *  Usage:
 *      Lfo vibrato;
 *      vibrato.Init(sample_rate);
 *      vibrato.SetFreq(5.f);              // 5 wobbles per second
 *      vibrato.SetWaveform(Lfo::WAVE_SINE);
 *      ...
 *      float depth = 10.f;                // +/- 10 Hz of wobble
 *      voice.SetFreq(440.f + depth * vibrato.Process());
 */
class Lfo
{
  public:
    enum Waveform
    {
        WAVE_SINE,
        WAVE_TRIANGLE,
        WAVE_SQUARE,
        WAVE_SAW,
        WAVE_RAMP,
    };

    void Init(float sample_rate)
    {
        osc_.Init(sample_rate);
        osc_.SetAmp(1.f);
        SetWaveform(WAVE_SINE);
        SetFreq(1.f);
    }

    /** LFO rate, in Hz. Typical range is well under 20 Hz - above
     *  that it starts to sound more like an audio tone than a wobble. */
    void SetFreq(float freq_hz) { osc_.SetFreq(freq_hz); }

    void SetWaveform(Waveform wave)
    {
        switch(wave)
        {
            case WAVE_TRIANGLE:
                osc_.SetWaveform(daisysp::Oscillator::WAVE_TRI);
                break;
            case WAVE_SQUARE:
                osc_.SetWaveform(daisysp::Oscillator::WAVE_SQUARE);
                break;
            case WAVE_SAW:
                osc_.SetWaveform(daisysp::Oscillator::WAVE_SAW);
                break;
            case WAVE_RAMP:
                osc_.SetWaveform(daisysp::Oscillator::WAVE_RAMP);
                break;
            case WAVE_SINE:
            default:
                osc_.SetWaveform(daisysp::Oscillator::WAVE_SIN);
                break;
        }
    }

    /** Jumps back to the start of its cycle - call on a NoteOn so the
     *  wobble starts from the same place every note instead of
     *  wherever it happened to be free-running. */
    void Reset() { osc_.Reset(); }

    /** Returns the next LFO value, in -1..1. */
    float Process() { return osc_.Process(); }

  private:
    daisysp::Oscillator osc_;
};

} // namespace dsp
