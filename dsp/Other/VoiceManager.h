#pragma once

namespace dsp
{
/** Turns an array of single-voice synth voices into a small polyphonic
 *  instrument: NoteOn(freq) grabs the next voice (round-robin,
 *  stealing the oldest active note once every voice is busy) and
 *  NoteOff(freq) releases whichever voice is currently playing that
 *  frequency.
 *
 *  Works with any voice type shaped like the ones in dsp/Synthesis -
 *  anything with Init(sample_rate) / SetFreq(freq_hz) / NoteOn() /
 *  NoteOff() / Process().
 *
 *  Usage, one voice per button:
 *      dsp::VoiceManager<dsp::Subtractive, 4> voices;
 *      voices.Init(sample_rate);
 *      ...
 *      if(button1.RisingEdge())  voices.NoteOn(220.f);
 *      if(button1.FallingEdge()) voices.NoteOff(220.f);
 *      if(button2.RisingEdge())  voices.NoteOn(330.f);
 *      if(button2.FallingEdge()) voices.NoteOff(330.f);
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
            freq_[v]   = 0.f;
            active_[v] = false;
        }
        next_ = 0;
    }

    /** Starts a note at `freq_hz` on the next voice (round-robin). If
     *  every voice is already active, steals the next one in line
     *  anyway, so the instrument stays playable beyond N simultaneous
     *  notes at the cost of cutting off whichever note that voice was
     *  holding. */
    void NoteOn(float freq_hz)
    {
        int v = next_;
        next_ = (next_ + 1) % N;

        freq_[v]   = freq_hz;
        active_[v] = true;
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

  private:
    T     voices_[N];
    float freq_[N];
    bool  active_[N];
    int   next_ = 0;
};

} // namespace dsp
