#pragma once

#include "Utility/metro.h"

namespace dsp
{
/** Step sequencer: a loop of up to kMaxSteps notes that play themselves
 *  at a set tempo, instead of waiting on a button press. Each step
 *  holds a frequency and whether it plays at all (its "gate") - a
 *  false gate is a rest.
 *
 *  Decoupled from any particular voice/VoiceManager, same as Lfo: it
 *  just tells you when a new step starts and what it is, you decide
 *  what to do with that (usually voices.NoteOn(CurrentFreq())).
 *
 *  Usage:
 *      Sequencer seq;
 *      seq.Init(sample_rate);
 *      seq.SetTempo(120.f);
 *      seq.SetStep(0, 220.f);
 *      seq.SetStep(1, 330.f);
 *      seq.SetNumSteps(2);
 *      ...
 *      if(seq.Process() && seq.CurrentGate())
 *          voices.NoteOn(seq.CurrentFreq());
 */
class Sequencer
{
  public:
    static constexpr int kMaxSteps = 16;

    void Init(float sample_rate)
    {
        sample_rate_ = sample_rate;
        for(int i = 0; i < kMaxSteps; i++)
        {
            freq_[i] = 220.f;
            gate_[i] = true;
        }
        UpdateMetroFreq();
        metro_.Init(metro_freq_, sample_rate_);
        Reset();
    }

    /** Playback speed, in quarter notes per minute. */
    void SetTempo(float bpm)
    {
        bpm_ = bpm;
        UpdateMetroFreq();
        metro_.SetFreq(metro_freq_);
    }

    /** How many steps make up one beat (one quarter note). 4 (the
     *  default) means each step is a 16th note. */
    void SetStepsPerBeat(int steps_per_beat)
    {
        steps_per_beat_ = steps_per_beat;
        UpdateMetroFreq();
        metro_.SetFreq(metro_freq_);
    }

    /** How many steps the pattern loops over before returning to step
     *  0. Clamped to 1..kMaxSteps. */
    void SetNumSteps(int num_steps)
    {
        if(num_steps < 1)
            num_steps = 1;
        if(num_steps > kMaxSteps)
            num_steps = kMaxSteps;
        num_steps_ = num_steps;
    }

    /** Programs step `index` (0..kMaxSteps-1) to play `freq_hz`.
     *  Pass gate = false to make it a rest instead. */
    void SetStep(int index, float freq_hz, bool gate = true)
    {
        if(index < 0 || index >= kMaxSteps)
            return;
        freq_[index] = freq_hz;
        gate_[index] = gate;
    }

    /** Resumes advancing through the pattern. */
    void Start() { running_ = true; }

    /** Freezes the pattern on whatever step it's currently on. */
    void Stop() { running_ = false; }

    /** True while the pattern is advancing. */
    bool IsRunning() const { return running_; }

    /** Jumps back to step 0 and resyncs the clock, so the next
     *  Process() call starts a fresh, clean step rather than landing
     *  mid-beat. */
    void Reset() { restart_ = true; }

    /** Advances the sequencer by one sample. Returns true exactly on
     *  the sample a new step begins - check CurrentFreq()/
     *  CurrentGate() then and act on it (e.g. trigger a voice). */
    bool Process()
    {
        if(!running_)
            return false;

        // The first call after Init()/Reset() plays step 0 straight
        // away. Done here rather than in Reset(), so the audio
        // callback can't run halfway through a Reset() from main().
        if(restart_)
        {
            restart_      = false;
            current_step_ = 0;
            metro_.Reset();
            return true;
        }

        if(metro_.Process())
        {
            current_step_ = (current_step_ + 1) % num_steps_;
            return true;
        }
        return false;
    }

    /** Index (0..NumSteps-1) of the step currently playing. */
    int CurrentStep() const { return current_step_; }

    /** Frequency of the step currently playing. */
    float CurrentFreq() const { return freq_[current_step_]; }

    /** Whether the step currently playing should sound (false = rest). */
    bool CurrentGate() const { return gate_[current_step_]; }

  private:
    void UpdateMetroFreq() { metro_freq_ = (bpm_ / 60.f) * steps_per_beat_; }

    daisysp::Metro metro_;
    float          sample_rate_    = 48000.f;
    float          bpm_            = 120.f;
    int            steps_per_beat_ = 4;
    float          metro_freq_     = 8.f;
    int            num_steps_      = 8;
    int            current_step_   = 0;
    bool           running_        = true;
    bool           restart_        = true;
    float          freq_[kMaxSteps];
    bool           gate_[kMaxSteps];
};

} // namespace dsp
