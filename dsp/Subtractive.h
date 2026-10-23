#pragma once

namespace dsp
{
/** Subtractive synthesis: oscillator -> filter -> envelope.
 *  Usage:
 *      Subtractive voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      float sample = voice.Process();
 */
class Subtractive
{
  public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        freq_        = 440.f;
    }

    void SetFreq(float freq_hz) { freq_ = freq_hz; }

    void SetCutoff(float cutoff_hz) { cutoff_ = cutoff_hz; }

    void SetResonance(float resonance) { resonance_ = resonance; }

    /** Returns the next audio sample. */
    float Process()
    {
        // TODO: oscillator -> filter -> envelope
        return 0.f;
    }

  private:
    float sample_rate_ = 48000.f;
    float freq_        = 440.f;
    float cutoff_      = 1000.f;
    float resonance_   = 0.f;
};

} // namespace dsp
