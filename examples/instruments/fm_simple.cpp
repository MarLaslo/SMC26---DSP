/** SMC26 DSP course example: simple FM synth.
 *
 *  FM (frequency modulation) makes rich, bright sounds from just two
 *  sine waves: a "modulator" wobbles the pitch of a "carrier" so fast
 *  that you no longer hear a wobble - you hear a new tone instead.
 *  dsp::FMSimple does this with only a few settings to learn.
 *
 *  Hold playButton to play a note. Press modeButton to switch between
 *  two playing styles (each press flips it):
 *    - pluck: the note fades out on its own, even while held
 *    - pad:   the note holds as long as the button is down
 *
 *  pitchKnob      picks the note (C major scale, one octave up/down)
 *  ratioKnob      the modulator's pitch compared to the note. Whole
 *                 numbers (1, 2, 3, ...) sound musical; values in
 *                 between sound metallic, like bells
 *  depthKnob      how much FM - from a pure sine to a bright, buzzy tone
 *  brightnessKnob 0 = the tone stays the same for the whole note,
 *                 1 = bright at the start, then mellows out
 *  lengthKnob     how long the note takes to fade, short to long
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    playButton - seed::D9     pitchKnob      - seed::A0
 *    modeButton - seed::D8     ratioKnob      - seed::A1
 *                              depthKnob      - seed::A2
 *                              brightnessKnob - seed::A3
 *                              lengthKnob     - seed::A4
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

Button playButton;
Button modeButton;

Potentiometer pitchKnob;
Potentiometer ratioKnob;
Potentiometer depthKnob;
Potentiometer brightnessKnob;
Potentiometer lengthKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    playButton.Init(seed::D9);
    modeButton.Init(seed::D8);

    pitchKnob.Init(hw, seed::A0);
    ratioKnob.Init(hw, seed::A1);
    depthKnob.Init(hw, seed::A2);
    brightnessKnob.Init(hw, seed::A3);
    lengthKnob.Init(hw, seed::A4);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    PrintValue("play", playButton.IsPressed());
    PrintValue("pad", modeButton.Toggled());
    PrintValue("pitch", pitchKnob.Read());
    PrintValue("ratio", ratioKnob.Read());
    PrintValue("depth", depthKnob.Read());
    PrintValue("brightness", brightnessKnob.Read());
    PrintValue("length", lengthKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// The FM voice. It has one envelope built in, which shapes both the
// volume and (with SetBrightness) the tone.
dsp::FMSimple fm;

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
        // The next sample of the FM voice, a bit quieter to leave
        // some headroom
        float sample = fm.Process() * 0.7f;

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

    // The FM voice needs to know the sample rate. A 10 ms attack
    // starts each note quickly without a click.
    fm.Init(hw.AudioSampleRate());
    fm.SetAttack(0.01f);

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // The note pitchKnob is centered on
    float rootNote = dsp::NoteToFreq("C3");

    // Runs forever, alongside the sound: reads the sensors and
    // passes their values on to the FM voice
    while (1)
    {
        // Read the buttons first, so JustPressed() and JustReleased()
        // below see the latest change
        playButton.Update();
        modeButton.Update();

        // Start and stop the note
        if (playButton.JustPressed())
            fm.NoteOn();
        if (playButton.JustReleased())
            fm.NoteOff();

        // Toggled() flips between false and true on every press.
        // Sustain is the level the note holds at while the button is
        // down: 0 means it fades away (pluck), 0.8 means it stays (pad).
        if (modeButton.Toggled())
            fm.SetSustain(0.8f);
        else
            fm.SetSustain(0.f);

        // Pitch: snaps to the notes of C major, an octave either way
        fm.SetFreq(dsp::MapSemitones(pitchKnob.Read(), rootNote, 12.f, dsp::SCALE_MAJOR));

        // Ratio from 0.5 (an octave below the note) to 6
        fm.SetModRatio(dsp::MapLinear(ratioKnob.Read(), 0.5f, 6.f));

        // FM depth: 0 = pure sine, 4 = bright and buzzy
        fm.SetModDepth(dsp::MapLinear(depthKnob.Read(), 0.f, 4.f));

        // Brightness is already 0 - 1, same as the knob
        fm.SetBrightness(brightnessKnob.Read());

        // Length: how long the note fades, from 0.1 to 3 seconds.
        // Decay is the fade while held (pluck mode), release is the
        // fade after letting go. MapExponential so short times get
        // as much of the knob as long ones (see dsp/Utility.h).
        float length = dsp::MapExponential(lengthKnob.Read(), 0.1f, 3.f);
        fm.SetDecay(length);
        fm.SetRelease(length);

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
