/** SMC26 DSP course example: voice manager.
 *
 *  The simplest way to play more than one note at a time. A synth
 *  voice (like dsp::Subtractive) can only play one note. To play a
 *  chord you need several voices - dsp::VoiceManager holds them for
 *  you and hands each new note to a free one.
 *
 *  Hold button1 to play an A, hold button2 to play an E. Hold both
 *  and you hear both notes together. cutoffKnob makes every voice
 *  darker or brighter. bendKnob bends the pitch of every note up or
 *  down by up to 2 semitones (knob in the middle = no bend).
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    button1 - seed::D9     cutoffKnob - seed::A0
 *    button2 - seed::D8     bendKnob   - seed::A1
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

Button button1;
Button button2;

Potentiometer cutoffKnob;
Potentiometer bendKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    button1.Init(seed::D9);
    button2.Init(seed::D8);

    cutoffKnob.Init(hw, seed::A0);
    bendKnob.Init(hw, seed::A1);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    PrintValue("button1", button1.IsPressed());
    PrintValue("button2", button2.IsPressed());
    PrintValue("cutoff", cutoffKnob.Read());
    PrintValue("bend", bendKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// How many notes can play at the same time. With 2 voices and 2
// buttons, every button always gets its own voice.
constexpr int kNumVoices = 2;

// The voice manager: kNumVoices copies of dsp::Subtractive in one
// object. Swap dsp::Subtractive for another synth from
// dsp/Synthesis (e.g. dsp::FMSimple, dsp::ModalVoice) to change the
// sound - the rest of the code stays the same.
dsp::VoiceManager<dsp::Subtractive, kNumVoices> voices;

// The note each button plays - note names instead of Hz, see
// dsp::NoteToFreq in dsp/Utility.h
float note1 = dsp::NoteToFreq("A3");
float note2 = dsp::NoteToFreq("E4");

// ---------------------------------------------------------------------
// 3. AudioCallback
// ---------------------------------------------------------------------

// Called by the board over and over, very fast, to get the next few
// samples of sound. Keep it quick: no printing and no System::Delay()
// in here, or the sound will click and drop out.
void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    // Runs once per sample - "size" samples each time
    for (size_t i = 0; i < size; i++)
    {
        // Process() adds all the voices together into one sample.
        // Two loud voices added together can go over 1 and distort,
        // so divide by the number of voices to keep it clean.
        float sample = voices.Process() / kNumVoices;

        // Send it to the left and right outputs
        OUT_L[i] = sample;
        OUT_R[i] = sample;
    }
}

// ---------------------------------------------------------------------
// 4. main()
// ---------------------------------------------------------------------

int main(void)
{
    // Sets up the board and the sensors (InitInputs() above).
    // Change false to true to print the sensor values over USB -
    // open a serial monitor to see them.
    InitHardware(false);

    // Sets up every voice at once - they need to know the sample rate
    voices.Init(hw.AudioSampleRate());

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // Runs forever, alongside the sound: reads the buttons and the
    // knob and tells the voice manager what to play
    while (1)
    {
        // Read the buttons first, so JustPressed() and JustReleased()
        // below see the latest change
        button1.Update();
        button2.Update();

        // Button goes down -> start its note. Button comes up -> stop
        // it. NoteOff() needs the same frequency as NoteOn(), so the
        // voice manager knows which voice to stop.
        if (button1.JustPressed())
            voices.NoteOn(note1);
        if (button1.JustReleased())
            voices.NoteOff(note1);

        if (button2.JustPressed())
            voices.NoteOn(note2);
        if (button2.JustReleased())
            voices.NoteOff(note2);

        // Settings like the cutoff belong to each voice, not to the
        // voice manager. voices.Voice(v) gives you voice number v, so
        // loop over all of them and give each the same setting.
        float cutoff = dsp::MapExponential(cutoffKnob.Read(), 200.f, 5000.f);
        for (int v = 0; v < kNumVoices; v++)
            voices.Voice(v).SetCutoff(cutoff);

        // Pitch bend. voices.Freq(v) is the note voice v was given by
        // NoteOn() - it never changes, so we always bend from the
        // original note. MapPitchBend turns the knob into a frequency
        // up to 2 semitones below (knob down) or above (knob up) that
        // note, and Voice(v).SetFreq() makes the voice play it.
        float bend = bendKnob.Read();
        for (int v = 0; v < kNumVoices; v++)
            voices.Voice(v).SetFreq(dsp::MapPitchBend(bend, voices.Freq(v), 2.f));

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
