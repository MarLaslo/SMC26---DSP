#pragma once

namespace dsp
{
/** Granular synthesis: plays back overlapping short grains of a
 *  stored buffer at varying position, size, and rate.
 *  Usage:
 *      Granular voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      float sample = voice.Process();
 */
class Granular
{
  public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        freq_        = 440.f;
    }

    void SetFreq(float freq_hz) { freq_ = freq_hz; }

    void SetGrainSize(float size_ms) { grain_size_ = size_ms; }

    void SetPosition(float position) { position_ = position; }

    /** Returns the next audio sample. */
    float Process()
    {
        // TODO: schedule overlapping grains, window and sum them
        return 0.f;
    }

  private:
    float sample_rate_ = 48000.f;
    float freq_        = 440.f;
    float grain_size_  = 50.f;
    float position_    = 0.f;
};

} // namespace dsp
