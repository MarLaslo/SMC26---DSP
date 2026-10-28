/** SMC26 DSP course example: additive synth.
 *
 *  Additive synthesis builds a sound by adding sine waves together.
 *  Every sound you hear is really a stack of sines (called partials
 *  or harmonics) at different pitches and volumes - here you build
 *  that stack by hand. dsp::Additive holds up to 16 of them.
 *
 *  Hold playButton to play a note. Press oddButton to switch between
 *  two kinds of stack (each press flips it):
 *    - all harmonics (1, 2, 3, 4, ... x the note): bright, buzzy,
 *      like a sawtooth wave or a bowed string
 *    - odd harmonics only (1, 3, 5, 7, ... x the note): hollow,
 *      like a square wave or a clarinet
 *
 *  pitchKnob     picks the note (A minor scale, one octave up/down)
 *  harmonicsKnob how many sines are stacked, 1 to 16. Turn it slowly
 *                from 0 and listen to each one join in
 *  rolloffKnob   how quickly the higher sines get quieter - low is
 *                dark and soft, high is bright
 *  stretchKnob   pulls the higher sines out of tune with each other.
 *                At 0 the stack sounds like one note; turn it up for
 *                piano, bell and gong sounds
 *  releaseKnob   how long the note rings after you let go
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    playButton - seed::D9     pitchKnob     - seed::A0
 *    oddButton  - seed::D8     harmonicsKnob - seed::A1
 *                              rolloffKnob   - seed::A2
 *                              stretchKnob   - seed::A3
 *                              releaseKnob   - seed::A4
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
Button oddButton;

Potentiometer pitchKnob;
Potentiometer harmonicsKnob;
Potentiometer rolloffKnob;
Potentiometer stretchKnob;
Potentiometer releaseKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    playButton.Init(seed::D9);
    oddButton.Init(seed::D8);

    pitchKnob.Init(hw, seed::A0);
    harmonicsKnob.Init(hw, seed::A1);
    rolloffKnob.Init(hw, seed::A2);
    stretchKnob.Init(hw, seed::A3);
    releaseKnob.Init(hw, seed::A4);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    PrintValue("play", playButton.IsPressed());
    PrintValue("odd", oddButton.Toggled());
    PrintValue("pitch", pitchKnob.Read());
    PrintValue("harmonics", harmonicsKnob.Read());
    PrintValue("rolloff", rolloffKnob.Read());
    PrintValue("stretch", stretchKnob.Read());
    PrintValue("release", releaseKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// The additive voice: a stack of up to 16 sine waves with a volume
// envelope built in
dsp::Additive additive;

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
        // The next sample of the whole stack, turned down by 12 dB.
        // Change -12 to set the volume: closer to 0 is louder, more
        // negative is quieter (see dsp::ApplyGain in dsp/Utility.h).
        float sample = dsp::ApplyGain(additive.Process(), -12.f);

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

    // The additive voice needs to know the sample rate. The envelope
    // starts quickly, dips a little, then holds while you play.
    additive.Init(hw.AudioSampleRate());
    additive.SetAmpAttack(0.01f);
    additive.SetAmpDecay(0.5f);
    additive.SetAmpSustain(0.6f);

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // The note pitchKnob is centered on
    float rootNote = dsp::NoteToFreq("A2");

    // Runs forever, alongside the sound: reads the sensors and
    // passes their values on to the additive voice
    while (1)
    {
        // Read the buttons first, so JustPressed() and JustReleased()
        // below see the latest change
        playButton.Update();
        oddButton.Update();

        // Start and stop the note
        if (playButton.JustPressed())
            additive.NoteOn();
        if (playButton.JustReleased())
            additive.NoteOff();

        // Pitch: snaps to the notes of A minor, an octave either way
        additive.SetFreq(dsp::MapSemitones(pitchKnob.Read(), rootNote, 12.f, dsp::SCALE_MINOR));

        // How many sines: roundf() turns the knob into a whole
        // number, since you can't have half a sine
        additive.SetNumHarmonics(roundf(dsp::MapLinear(harmonicsKnob.Read(), 1.f, 16.f)));

        // Rolloff: each sine is this much quieter than the one below
        // it. 0.3 = each one less than a third as loud (dark),
        // 0.95 = nearly as loud as the one below (bright)
        additive.SetRolloff(dsp::MapLinear(rolloffKnob.Read(), 0.3f, 0.95f));

        // Stretch: 1 = perfectly in tune, 1.3 = higher sines pushed
        // sharp, the further up the stack the more
        float stretch = dsp::MapLinear(stretchKnob.Read(), 1.f, 1.3f);

        // Work out the pitch of every sine in the stack, as a ratio
        // of the note: ratio 1 is the note itself, 2 is an octave up,
        // 3 an octave and a fifth up, and so on. "h" counts the sines
        // from 0, so h + 1 is the harmonic number.
        for (int h = 0; h < dsp::Additive::kMaxHarmonics; h++)
        {
            // All harmonics: 1, 2, 3, 4, ...  Odd only: 1, 3, 5, 7, ...
            float harmonic;
            if (oddButton.Toggled())
                harmonic = 2.f * h + 1.f;
            else
                harmonic = h + 1.f;

            // powf(harmonic, stretch) leaves the note itself (1)
            // alone, and pushes higher harmonics further sharp
            additive.SetHarmonicRatio(h, powf(harmonic, stretch));
        }

        // Release: from 0.05 to 3 seconds. MapExponential so short
        // times get as much of the knob as long ones (see
        // dsp/Utility.h).
        additive.SetAmpRelease(dsp::MapExponential(releaseKnob.Read(), 0.05f, 3.f));

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
