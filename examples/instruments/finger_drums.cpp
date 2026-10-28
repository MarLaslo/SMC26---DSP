/** SMC26 DSP course example: finger drums.
 *
 *  A small drum kit you play with your fingers. The two buttons are
 *  drum pads - kick and snare. Both drums are made from scratch (no
 *  recordings): sines, noise and envelopes.
 *
 *  kickButton  hits the kick drum
 *  snareButton hits the snare drum
 *
 *  kickPitchKnob   low and boomy, up to tight and punchy
 *  kickDecayKnob   short thud, up to a long 808-style boom
 *  snareSnappyKnob 0 = mostly drum "tone", 1 = mostly noise "snap"
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    kickButton  - seed::D9     kickPitchKnob   - seed::A0
 *    snareButton - seed::D8     kickDecayKnob   - seed::A1
 *                               snareSnappyKnob - seed::A2
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

Button kickButton;
Button snareButton;

Potentiometer kickPitchKnob;
Potentiometer kickDecayKnob;
Potentiometer snareSnappyKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    kickButton.Init(seed::D9);
    snareButton.Init(seed::D8);

    kickPitchKnob.Init(hw, seed::A0);
    kickDecayKnob.Init(hw, seed::A1);
    snareSnappyKnob.Init(hw, seed::A2);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    PrintValue("kick", kickButton.IsPressed());
    PrintValue("snare", snareButton.IsPressed());
    PrintValue("kickPitch", kickPitchKnob.Read());
    PrintValue("kickDecay", kickDecayKnob.Read());
    PrintValue("snareSnappy", snareSnappyKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// The two drums
dsp::SynthBassDrum  kick;
dsp::SynthSnareDrum snare;

// Catches the loudest moments - like both drums hit at once - and
// turns them down smoothly, so the output never distorts
dsp::Limiter limiter;

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
        // Mix the drums: add them together, each at its own volume.
        // 0 dB = as loud as it comes, more negative = quieter.
        float sample = dsp::ApplyGain(kick.Process(), -3.f)
                       + dsp::ApplyGain(snare.Process(), -6.f);

        // Safety net on the final mix
        sample = limiter.Process(sample);

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

    // Every drum needs to know the sample rate
    float sampleRate = hw.AudioSampleRate();
    kick.Init(sampleRate);
    snare.Init(sampleRate);
    limiter.Init();

    // Settings no knob changes - try your own values here
    kick.SetAccent(0.8f);    // how hard each kick hits, 0 - 1
    kick.SetDirtiness(0.2f); // a little grit on the kick, 0 - 1
    snare.SetFreq(200.f);    // pitch of the snare's drum tone, Hz
    snare.SetAccent(0.8f);   // how hard each snare hits, 0 - 1

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // Runs forever, alongside the sound: plays the drums and passes
    // the knob values on to them
    while (1)
    {
        // Read the buttons first, so JustPressed() below sees the
        // latest press
        kickButton.Update();
        snareButton.Update();

        // The pads: a drum only needs to be hit - it fades out by
        // itself, so there is nothing to do when the button comes up
        if (kickButton.JustPressed())
            kick.Hit();
        if (snareButton.JustPressed())
            snare.Hit();

        // Kick: pitch from 40 to 90 Hz (MapExponential so the knob
        // feels even, see dsp/Utility.h). Decay is already 0 - 1.
        kick.SetFreq(dsp::MapExponential(kickPitchKnob.Read(), 40.f, 90.f));
        kick.SetDecay(kickDecayKnob.Read());

        // Snappy is 0 - 1, same as the knob, so it can go straight in
        snare.SetSnappy(snareSnappyKnob.Read());

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
