#pragma once

#include <cmath>
#include "Synthesis/oscillator.h"
#include "Sampling/granularplayer.h"
#include "Control/adsr.h"

namespace dsp
{
/** Granular synthesis: wraps daisysp::GranularPlayer around a small,
 *  self-generated waveform so it works as a standalone synth voice,
 *  no sample recording/loading required. The buffer is filled once at
 *  Init() with a few cycles of a bandlimited sawtooth that tile it
 *  seamlessly; the granulator then reads overlapping, windowed grains
 *  out of that buffer to produce the sound.
 *  Pitch and grain-scan-speed are independent controls (that's the
 *  whole point of granular synthesis): SetFreq() shifts the pitch of
 *  the grains, SetSpeed() controls how fast the read position drifts
 *  through the buffer (0 = frozen on one spot, for a pure drone-like
 *  grain tone).
 *  An ADSR amp envelope shapes each note: NoteOn() opens it, NoteOff()
 *  releases it.
 *  Usage:
 *      Granular voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      voice.SetGrainSize(40.f);
 *      voice.SetSpeed(0.1f);
 *      voice.NoteOn();
 *      float sample = voice.Process();
 *      ...
 *      voice.NoteOff();
 */
class Granular
{
  public:
    static constexpr int kBufferSize = 4096; // ~85ms of grain source at 48kHz

    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        FillBuffer();
        player_.Init(buffer_, kBufferSize, sample_rate_);

        SetFreq(440.f);
        SetGrainSize(40.f);
        SetSpeed(0.f);

        amp_env_.Init(sample_rate_);
        amp_env_.SetAttackTime(0.01f);
        amp_env_.SetDecayTime(0.3f);
        amp_env_.SetSustainLevel(0.8f);
        amp_env_.SetReleaseTime(0.3f);
    }

    /** Starts a note: triggers the amp envelope from the attack stage.
     *  On a note that's already playing, the attack starts again from
     *  the current level, so every NoteOn() sounds like a new note. */
    void NoteOn()
    {
        gate_      = true;
        retrigger_ = true;
    }

    /** Releases the note: the amp envelope moves to its release stage. */
    void NoteOff() { gate_ = false; }

    void SetAmpAttack(float time_s) { amp_env_.SetAttackTime(time_s); }
    void SetAmpDecay(float time_s) { amp_env_.SetDecayTime(time_s); }
    void SetAmpSustain(float level) { amp_env_.SetSustainLevel(level); }
    void SetAmpRelease(float time_s) { amp_env_.SetReleaseTime(time_s); }

    /** Pitches the grains relative to the buffer's own fundamental. */
    void SetFreq(float freq_hz)
    {
        freq_             = freq_hz;
        transposition_cts_ = 1200.f * log2f(freq_ / root_freq_);
    }

    /** Length of each grain in milliseconds. Range: 1 ms up to the
     *  buffer's length (about 85 ms at 48 kHz) - a grain can't be
     *  longer than the sound it's cut from, so longer values are
     *  clamped to that. */
    void SetGrainSize(float size_ms)
    {
        // GranularPlayer wraps its read position only once, so a
        // grain longer than the buffer would read past its end
        float max_ms = 1000.f * (kBufferSize - 1) / sample_rate_;
        if(size_ms < 1.f)
            size_ms = 1.f;
        if(size_ms > max_ms)
            size_ms = max_ms;
        grain_size_ms_ = size_ms;
    }

    /** How fast the grain read position scans through the buffer.
     *  0 freezes it in place; 1 scans through at normal speed;
     *  negative values scan backwards. */
    void SetSpeed(float speed) { speed_ = speed; }

    /** Returns the next audio sample. */
    float Process()
    {
        // NoteOn() on a note that's already on: restart the attack from
        // the current level (no click). The envelope can't spot that
        // itself, as its gate never went from off to on. Skipped if
        // NoteOff() came straight after.
        if(retrigger_)
        {
            retrigger_ = false;
            if(gate_)
                amp_env_.Retrigger(false);
        }

        float grain = player_.Process(speed_, transposition_cts_, grain_size_ms_);
        return grain * amp_env_.Process(gate_);
    }

  private:
    /** Fills the buffer with cycles_ cycles of a bandlimited sawtooth,
     *  sized so it loops back to its start with no discontinuity. */
    void FillBuffer()
    {
        root_freq_ = cycles_ * sample_rate_ / static_cast<float>(kBufferSize);

        daisysp::Oscillator osc;
        osc.Init(sample_rate_);
        osc.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
        osc.SetFreq(root_freq_);
        osc.SetAmp(1.f);

        for(int i = 0; i < kBufferSize; i++)
            buffer_[i] = osc.Process();

        // GranularPlayer's wrap can land exactly on index kBufferSize
        // (one past the end), so repeat the first sample there - the
        // buffer loops seamlessly, so that's the sample it should read
        buffer_[kBufferSize] = buffer_[0];
    }

    daisysp::GranularPlayer player_;
    daisysp::Adsr           amp_env_;
    float                   buffer_[kBufferSize + 1]; // +1: see FillBuffer()
    float                   sample_rate_       = 48000.f;
    float                   cycles_            = 8.f;
    float                   root_freq_         = 93.75f;
    float                   freq_              = 440.f;
    float                   transposition_cts_ = 0.f;
    float                   grain_size_ms_     = 40.f;
    float                   speed_             = 0.f;
    bool                    gate_              = false;
    bool                    retrigger_         = false;
};

} // namespace dsp
