#pragma once

namespace dsp
{
/** Additive synthesis: sums a stack of harmonically related
 *  sine oscillators to build up a more complex tone.
 *  Usage:
 *      Additive voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      float sample = voice.Process();
 */
class Additive
{
  public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        freq_        = 440.f;
    }

    void SetFreq(float freq_hz) { freq_ = freq_hz; }

    void SetNumHarmonics(int num_harmonics) { num_harmonics_ = num_harmonics; }

    void SetRolloff(float rolloff) { rolloff_ = rolloff; }

    /** Returns the next audio sample. */
    float Process()
    {
        // TODO: sum a bank of harmonic sine oscillators
        return 0.f;
    }

  private:
    float sample_rate_   = 48000.f;
    float freq_          = 440.f;
    int   num_harmonics_ = 8;
    float rolloff_       = 0.5f;
};

} // namespace dsp
