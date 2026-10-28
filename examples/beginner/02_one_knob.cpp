/** SMC26 DSP course - beginner step 2 of 3: one knob.
 *
 *  The tone from step 1, but now a knob changes its pitch while it
 *  plays.
 *
 *  What you learn here:
 *    - how to add a sensor: declare it, Init() it on a pin, Read() it
 *    - a knob gives a number from 0 to 1 - and how to turn that into
 *      a useful range (here: a pitch in Hz)
 *    - the while loop in main(): it runs forever, reading the knob
 *      and updating the sound
 *
 *  Try changing:
 *    - 110.f and 880.f - the lowest and highest pitch the knob reaches
 *    - MapExponential to MapLinear, and listen to how the knob feels
 *    - InitHardware(false) to true, and open a serial monitor to see
 *      the knob's value
 *
 *  Next step: 03_one_button.cpp - play notes with a button.
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    pitchKnob - seed::A0
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

// NEW: a knob (also called a potentiometer). Like the oscillator,
// "Potentiometer" is the kind of thing and "pitchKnob" is its name.
Potentiometer pitchKnob;

// Tells the board which pin the knob is plugged into. Every sensor
// needs one Init() line here.
void InitInputs()
{
    pitchKnob.Init(hw, seed::A0);
}

// Prints the knob's value - only when InitHardware(true) in main()
void PrintValues()
{
    PrintValue("pitchKnob", pitchKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// The oscillator from step 1
dsp::Oscillator osc;

// ---------------------------------------------------------------------
// 3. AudioCallback
// ---------------------------------------------------------------------

// The same as step 1: ask the oscillator for each sample and send it
// out. The knob is read in main(), not here - see below.
void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        float sample = osc.Process();

        OUT_L[i] = sample;
        OUT_R[i] = sample;
    }
}

// ---------------------------------------------------------------------
// 4. main()
// ---------------------------------------------------------------------

int main(void)
{
    // Sets up the board and the knob (InitInputs() above). Change
    // false to true to print the knob's value over USB.
    InitHardware(false);

    // Set up the oscillator and start the note, the same as step 1.
    // No SetFreq() here - the knob sets it below.
    osc.Init(hw.AudioSampleRate());
    osc.SetWaveform(dsp::Oscillator::WAVE_SINE);
    osc.NoteOn();

    hw.StartAudio(AudioCallback);

    // NEW: this loop runs forever, about 1000 times a second. Each
    // time round it reads the knob and sets the pitch.
    while (1)
    {
        // Read() gives the knob's position: 0 = all the way left,
        // 1 = all the way right, 0.5 = in the middle
        float knob = pitchKnob.Read();

        // Turn 0 - 1 into 110 - 880 Hz. MapExponential makes every
        // bit of the knob move the pitch by the same musical amount -
        // the same way a piano keyboard works (see dsp/Utility.h).
        float pitch = dsp::MapExponential(knob, 110.f, 880.f);

        // Tell the oscillator its new pitch
        osc.SetFreq(pitch);

        // About 10 times a second, print the knob's value - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
