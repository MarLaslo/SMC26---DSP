#pragma once

#include <cstdint>

namespace dsp
{
/** Turns an array of single-voice synth voices into a small polyphonic
 *  instrument: NoteOn(freq) grabs a free voice (taking turns, so a
 *  released note can ring out while the next one plays), stealing the
 *  oldest note once every voice is busy, and NoteOff(freq) releases
 *  whichever voice is currently playing that frequency.
 *
 *  Works with any voice type shaped like the ones in dsp/Synthesis -
 *  anything with Init(sample_rate) / SetFreq(freq_hz) / NoteOn() /
 *  NoteOff() / Process().
 *
 *  Usage, one voice per button:
 *      dsp::VoiceManager<dsp::Subtractive, 4> voices;
 *      voices.Init(sample_rate);
 *      ...
 *      if(button1.JustPressed())  voices.NoteOn(220.f);
 *      if(button1.JustReleased()) voices.NoteOff(220.f);
 *      if(button2.JustPressed())  voices.NoteOn(330.f);
 *      if(button2.JustReleased()) voices.NoteOff(330.f);
 *      ...
 *      float sample = voices.Process();
 */
template <typename T, int N>
class VoiceManager
{
  public:
    void Init(float sample_rate)
    {
        for(int v = 0; v < N; v++)
        {
            voices_[v].Init(sample_rate);
            freq_[v]    = 0.f;
            active_[v]  = false;
            started_[v] = 0;
        }
        next_       = 0;
        note_count_ = 0;
    }

    /** Starts a note at `freq_hz` on a free voice, taking turns between
     *  the free ones. If every voice is already active, steals the one
     *  holding the oldest note, so the instrument stays playable beyond
     *  N simultaneous notes at the cost of cutting that note off. */
    void NoteOn(float freq_hz)
    {
        int v = FindVoice();
        next_ = (v + 1) % N;

        freq_[v]    = freq_hz;
        active_[v]  = true;
        started_[v] = ++note_count_;
        voices_[v].SetFreq(freq_hz);
        voices_[v].NoteOn();
    }

    /** Releases whichever voice is currently playing `freq_hz`. Safe to
     *  call even if that note was already stolen by another NoteOn -
     *  it then does nothing. */
    void NoteOff(float freq_hz)
    {
        for(int v = 0; v < N; v++)
        {
            if(active_[v] && freq_[v] == freq_hz)
            {
                voices_[v].NoteOff();
                active_[v] = false;
            }
        }
    }

    /** Sums every voice's next sample. */
    float Process()
    {
        float sum = 0.f;
        for(int v = 0; v < N; v++)
            sum += voices_[v].Process();
        return sum;
    }

    /** Direct access to voice `v` (0..N-1), for per-voice setup that
     *  doesn't fit NoteOn/NoteOff/SetFreq (e.g. SetNumHarmonics,
     *  SetAmpAttack, ...). */
    T& Voice(int v) { return voices_[v]; }

    /** The frequency voice `v` was last given via NoteOn, regardless
     *  of whether it's still active - handy for layering a vibrato LFO
     *  on top: voices.Voice(v).SetFreq(voices.Freq(v) + depth * lfo). */
    float Freq(int v) const { return freq_[v]; }

  private:
    /** The first free voice from next_ on or, if every voice is
     *  active, the one whose note started longest ago. */
    int FindVoice() const
    {
        int oldest = next_;
        for(int i = 0; i < N; i++)
        {
            int v = (next_ + i) % N;
            if(!active_[v])
                return v;
            if(started_[v] < started_[oldest])
                oldest = v;
        }
        return oldest;
    }

    T        voices_[N];
    float    freq_[N];
    bool     active_[N];
    uint32_t started_[N]; // note_count_ when each voice's note began
    uint32_t note_count_ = 0;
    int      next_       = 0;
};

} // namespace dsp
