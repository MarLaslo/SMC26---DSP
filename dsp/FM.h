#pragma once

namespace dsp
{
/** FM synthesis: carrier oscillator frequency-modulated by a modulator oscillator.
 *  Usage:
 *      FM voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      float sample = voice.Process();
 */
class FM
{
  public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        freq_        = 440.f;
    }

    void SetFreq(float freq_hz) { freq_ = freq_hz; }

    void SetModRatio(float ratio) { mod_ratio_ = ratio; }

    void SetModDepth(float depth) { mod_depth_ = depth; }

    /** Returns the next audio sample. */
    float Process()
    {
        // TODO: modulator oscillator -> carrier oscillator
        return 0.f;
    }

  private:
    float sample_rate_ = 48000.f;
    float freq_        = 440.f;
    float mod_ratio_   = 1.f;
    float mod_depth_   = 0.f;
};

} // namespace dsp
