#pragma once

namespace dsp
{
/** Sampler: plays back a stored audio recording, pitched up or
 *  down by changing playback speed.
 *  Usage:
 *      Sampler voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      voice.Trigger();
 *      float sample = voice.Process();
 */
class Sampler
{
  public:
    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        freq_        = 440.f;
    }

    /** Pitches the sample's playback speed relative to its root freq. */
    void SetFreq(float freq_hz) { freq_ = freq_hz; }

    void SetLoop(bool loop) { loop_ = loop; }

    /** Starts playback from the beginning of the sample. */
    void Trigger()
    {
        // TODO: reset playback position to the start of the buffer
    }

    /** Returns the next audio sample. */
    float Process()
    {
        // TODO: read from sample buffer at a speed derived from freq_
        return 0.f;
    }

  private:
    float sample_rate_ = 48000.f;
    float freq_        = 440.f;
    bool  loop_        = false;
};

} // namespace dsp
