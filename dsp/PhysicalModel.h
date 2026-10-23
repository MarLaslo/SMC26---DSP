#pragma once

namespace dsp
{
/** Physical modeling synthesis (Karplus-Strong style plucked string).
 *  Usage:
 *      PhysicalModel voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      voice.Pluck();
 *      float sample = voice.Process();
 */
class PhysicalModel
{
  public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        freq_        = 440.f;
    }

    void SetFreq(float freq_hz) { freq_ = freq_hz; }

    void SetDamping(float damping) { damping_ = damping; }

    /** Excites the model, like plucking a string. */
    void Pluck()
    {
        // TODO: fill delay line with noise burst
    }

    /** Returns the next audio sample. */
    float Process()
    {
        // TODO: delay line + damping filter feedback loop
        return 0.f;
    }

  private:
    float sample_rate_ = 48000.f;
    float freq_        = 440.f;
    float damping_     = 0.5f;
};

} // namespace dsp
