#pragma once

namespace dsp
{
/** Wavetable synthesis: scans through a stored waveform table.
 *  Usage:
 *      Wavetable voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      float sample = voice.Process();
 */
class Wavetable
{
  public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        freq_        = 440.f;
    }

    void SetFreq(float freq_hz) { freq_ = freq_hz; }

    void SetPosition(float position) { position_ = position; }

    /** Returns the next audio sample. */
    float Process()
    {
        // TODO: read and interpolate from a wavetable, advance phase
        return 0.f;
    }

  private:
    float sample_rate_ = 48000.f;
    float freq_        = 440.f;
    float position_    = 0.f;
};

} // namespace dsp
