/** SMC26 DSP course - beginner step 1 of 3: hello sound.
 *
 *  The smallest possible instrument: one tone that plays as soon as
 *  the board turns on. No knobs, no buttons - just sound.
 *
 *  What you learn here:
 *    - the four parts every patch has (Sensors, Sound, AudioCallback,
 *      main) - every other example uses the same four
 *    - how to make a sound object, set it up, and hear it
 *
 *  Try changing:
 *    - 220.f in main() to another number - higher = higher pitch
 *      (try 110, 440, 880)
 *    - WAVE_SINE to WAVE_SAW, WAVE_SQUARE or WAVE_TRIANGLE
 *
 *  Next step: 02_one_knob.cpp - change the pitch with a knob.
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring: nothing - just plug in headphones or a speaker.
 */

// The building blocks: synthesis, effects, filters, utilities, and
// the board itself (hardware.h). You do not need to change these.
#include "daisysp.h"
#include "dsp/Utility.h"
#include "dsp/Synthesis/Synthesis.h"
#include "dsp/Effects/Effects.h"
#include "dsp/Filters/Filters.h"
#include "dsp/Other/Other.h"
#include "hardware.h"

// ---------------------------------------------------------------------
// 1. Sensors
// ---------------------------------------------------------------------

// No sensors yet - these two stay empty. They still have to be here,
// because the board calls them (you'll fill them in, in step 2).
void InitInputs()
{
}

void PrintValues()
{
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// An oscillator makes a tone: a wave that repeats over and over.
// "dsp::Oscillator" is the kind of thing, "osc" is the name we give
// it - you could call it anything.
dsp::Oscillator osc;

// ---------------------------------------------------------------------
// 3. AudioCallback
// ---------------------------------------------------------------------

// The board calls this over and over, very fast, to ask "what's the
// next bit of sound?". Sound is a long list of numbers between -1 and
// 1 (called samples) - about 48000 of them every second.
void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    // The board asks for a few samples at a time - "size" of them -
    // so work them out one by one
    for (size_t i = 0; i < size; i++)
    {
        // Ask the oscillator for its next sample
        float sample = osc.Process();

        // Send it to the left and right outputs
        OUT_L[i] = sample;
        OUT_R[i] = sample;
    }
}

// ---------------------------------------------------------------------
// 4. main()
// ---------------------------------------------------------------------

// main() is where the board starts. Everything in here runs once, top
// to bottom, then the while loop at the end runs forever.
int main(void)
{
    // Sets up the board
    InitHardware(false);

    // Set up the oscillator. It needs to know the sample rate (how
    // many samples per second) to play at the right pitch.
    osc.Init(hw.AudioSampleRate());

    // The shape of the wave - this changes the sound's color
    osc.SetWaveform(dsp::Oscillator::WAVE_SINE);

    // The pitch, in Hz (vibrations per second). 220 Hz is the note A3.
    osc.SetFreq(220.f);

    // Start the note. It keeps playing until NoteOff() - which we
    // never call, so it plays forever.
    osc.NoteOn();

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // Nothing else to do - just wait forever while the sound plays
    while (1)
    {
        System::Delay(1);
    }
}
