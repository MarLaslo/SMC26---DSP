#pragma once

#include "Noise/whitenoise.h"
#include "Noise/fractal_noise.h"
#include "Noise/clockednoise.h"

namespace dsp
{
/** Plain white noise: every sample is an independent random value.
 *  The rawest noise source - useful on its own as a percussion /
 *  wind / hiss layer, or as the input to a filter for shaped noise.
 *  Usage:
 *      Noise noise;
 *      noise.Init();
 *      noise.SetAmp(0.5f);
 *      float sample = noise.Process();
 */
class Noise
{
  public:
    void Init() { noise_.Init(); }

    /** Output level, 0-1. */
    void SetAmp(float amp) { noise_.SetAmp(amp); }

    /** Returns the next audio sample. */
    float Process() { return noise_.Process(); }

  private:
    daisysp::WhiteNoise noise_;
};

/** Colored noise: stacks several white-noise generators an octave
 *  apart to tilt the spectrum from dark/rumbly to bright/hissy.
 *  Usage:
 *      ColoredNoise noise;
 *      noise.Init(sample_rate);
 *      noise.SetColor(0.3f);
 *      float sample = noise.Process();
 */
class ColoredNoise
{
  public:
    void Init(float sample_rate)
    {
        noise_.Init(sample_rate);
        SetColor(0.5f);
    }

    /** Tilts the noise's tone, 0 = darkest/rumbliest, 1 = brightest/hissiest. */
    void SetColor(float color) { noise_.SetColor(color); }

    /** Returns the next audio sample. */
    float Process() { return noise_.Process(); }

  private:
    daisysp::FractalRandomGenerator<daisysp::ClockedNoise, 4> noise_;
};

/** Clocked noise: a new random value is held at a set frequency
 *  instead of changing every sample, giving a grainier, pitched
 *  noise (handy for lo-fi textures or as a sample-and-hold source).
 *  Usage:
 *      SteppedNoise noise;
 *      noise.Init(sample_rate);
 *      noise.SetFreq(2000.f);
 *      float sample = noise.Process();
 */
class SteppedNoise
{
  public:
    void Init(float sample_rate) { noise_.Init(sample_rate); }

    /** Rate at which a new random value is held, in Hz. */
    void SetFreq(float freq_hz) { noise_.SetFreq(freq_hz); }

    /** Forces a new random value immediately, outside of the clock. */
    void Sync() { noise_.Sync(); }

    /** Returns the next audio sample. */
    float Process() { return noise_.Process(); }

  private:
    daisysp::ClockedNoise noise_;
};

} // namespace dsp
