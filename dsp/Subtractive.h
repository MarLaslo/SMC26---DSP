#pragma once

#include "Synthesis/oscillator.h"
#include "Filters/ladder.h"
#include "Control/adsr.h"

namespace dsp
{
/** Subtractive synthesis: a harmonically rich oscillator is carved
 *  down by a resonant lowpass filter, then shaped by an amp envelope.
 *  A second envelope can sweep the filter's cutoff open on each note,
 *  for the classic "filter zap" attack.
 *  Usage:
 *      Subtractive voice;
 *      voice.Init(sample_rate);
 *      voice.SetFreq(440.f);
 *      voice.SetCutoff(800.f);
 *      voice.SetResonance(0.5f);
 *      voice.NoteOn();
 *      float sample = voice.Process();
 *      ...
 *      voice.NoteOff();
 */
class Subtractive
{
  public:
    enum Waveform
    {
        WAVE_SAW,
        WAVE_SQUARE,
        WAVE_TRIANGLE,
    };

    void Init(float sample_rate)
    {
        osc_.Init(sample_rate);
        SetWaveform(WAVE_SAW);
        SetFreq(440.f);

        filter_.Init(sample_rate);
        SetCutoff(1000.f);
        SetResonance(0.f);

        amp_env_.Init(sample_rate);
        amp_env_.SetAttackTime(0.01f);
        amp_env_.SetDecayTime(0.3f);
        amp_env_.SetSustainLevel(0.8f);
        amp_env_.SetReleaseTime(0.3f);

        filter_env_.Init(sample_rate);
        filter_env_.SetAttackTime(0.01f);
        filter_env_.SetDecayTime(0.2f);
        filter_env_.SetSustainLevel(0.f);
        filter_env_.SetReleaseTime(0.2f);
    }

    /** Starts a note: triggers the amp and filter envelopes from the
     *  attack stage. */
    void NoteOn() { gate_ = true; }

    /** Releases the note: both envelopes move to their release stage. */
    void NoteOff() { gate_ = false; }

    void SetFreq(float freq_hz) { osc_.SetFreq(freq_hz); }

    /** Picks the raw oscillator shape that the filter will carve down. */
    void SetWaveform(Waveform wave)
    {
        switch(wave)
        {
            case WAVE_SQUARE:
                osc_.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SQUARE);
                break;
            case WAVE_TRIANGLE:
                osc_.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_TRI);
                break;
            case WAVE_SAW:
            default:
                osc_.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
                break;
        }
    }

    /** Filter's base cutoff frequency, in Hz. */
    void SetCutoff(float cutoff_hz)
    {
        cutoff_ = cutoff_hz;
    }

    /** Filter resonance / emphasis around the cutoff. Range: 0 - 1. */
    void SetResonance(float resonance) { filter_.SetRes(resonance); }

    /** How far the filter envelope pushes the cutoff above its base
     *  value, in Hz, at the envelope's peak. 0 = cutoff stays fixed. */
    void SetFilterEnvAmount(float amount_hz) { filter_env_amount_ = amount_hz; }

    void SetFilterAttack(float time_s) { filter_env_.SetAttackTime(time_s); }
    void SetFilterDecay(float time_s) { filter_env_.SetDecayTime(time_s); }
    void SetFilterSustain(float level) { filter_env_.SetSustainLevel(level); }
    void SetFilterRelease(float time_s) { filter_env_.SetReleaseTime(time_s); }

    void SetAmpAttack(float time_s) { amp_env_.SetAttackTime(time_s); }
    void SetAmpDecay(float time_s) { amp_env_.SetDecayTime(time_s); }
    void SetAmpSustain(float level) { amp_env_.SetSustainLevel(level); }
    void SetAmpRelease(float time_s) { amp_env_.SetReleaseTime(time_s); }

    /** Returns the next audio sample. */
    float Process()
    {
        float filter_env_level = filter_env_.Process(gate_);
        filter_.SetFreq(cutoff_ + filter_env_amount_ * filter_env_level);

        float sample = filter_.Process(osc_.Process());
        return sample * amp_env_.Process(gate_);
    }

  private:
    daisysp::Oscillator   osc_;
    daisysp::LadderFilter filter_;
    daisysp::Adsr         amp_env_;
    daisysp::Adsr         filter_env_;
    float                 cutoff_            = 1000.f;
    float                 filter_env_amount_ = 0.f;
    bool                  gate_              = false;
};

} // namespace dsp
