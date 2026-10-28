/** SMC26 DSP course example: drum sequencer.
 *
 *  An 8-step drum sequencer, playing kick/snare/hat on its own.
 *  tempoKnob speeds/slows it; playStopButton stops/resumes it;
 *  resetButton jumps it back to the first step.
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    playStopButton - seed::D9     kickDecayKnob   - seed::A0
 *    resetButton    - seed::D8     hatToneKnob     - seed::A1
 *                                  snareSnappyKnob - seed::A2
 *                                  tempoKnob       - seed::A3
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

Button playStopButton;
Button resetButton;

Potentiometer kickDecayKnob;
Potentiometer hatToneKnob;
Potentiometer snareSnappyKnob;
Potentiometer tempoKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    playStopButton.Init(seed::D9);
    resetButton.Init(seed::D8);

    kickDecayKnob.Init(hw, seed::A0);
    hatToneKnob.Init(hw, seed::A1);
    snareSnappyKnob.Init(hw, seed::A2);
    tempoKnob.Init(hw, seed::A3);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    PrintValue("playStop", playStopButton.IsPressed());
    PrintValue("reset", resetButton.IsPressed());
    PrintValue("kickDecay", kickDecayKnob.Read());
    PrintValue("hatTone", hatToneKnob.Read());
    PrintValue("snareSnappy", snareSnappyKnob.Read());
    PrintValue("tempo", tempoKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// The three drums. Each one makes a single hit when you call Hit().
dsp::SynthBassDrum kick;
dsp::SynthSnareDrum snare;
dsp::HiHat hat;

// Which drum each step hits - Sequencer just hands back whatever
// number you gave it via SetStep, so here we use it as a drum ID
// instead of a frequency. See dsp/Other/Sequencer.h.
constexpr float kKick = 0.f;
constexpr float kSnare = 1.f;
constexpr float kHat = 2.f;

// Plays itself: an 8-step pattern - Kick, Hat, Hat, Hat, Snare, Hat,
// Hat, Hat - looping at a tempo set by tempoKnob. Edit kPattern below
// (and sequencer.SetNumSteps if you change how many hits are in it)
// to make your own.
dsp::Sequencer sequencer;

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
        // Process() moves the sequencer on by one sample and is true
        // only when a new step starts. If that step isn't a rest,
        // hit the drum it holds.
        if (sequencer.Process() && sequencer.CurrentGate())
        {
            float drum = sequencer.CurrentFreq();
            if (drum == kKick)
                kick.Hit();
            else if (drum == kSnare)
                snare.Hit();
            else
                hat.Hit();
        }

        // Mix the three drums together into one sample
        float sample = kick.Process() + snare.Process() + hat.Process();

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
    // true = print the sensor values over USB - open a serial monitor
    // to see them. Change to false to turn printing off.
    InitHardware(true);

    // Set up the drums - they need to know the sample rate
    kick.Init(hw.AudioSampleRate());
    snare.Init(hw.AudioSampleRate());
    hat.Init(hw.AudioSampleRate());

    // Set up the sequencer and load the pattern into its 8 steps
    sequencer.Init(hw.AudioSampleRate());
    sequencer.SetTempo(120.f);
    static const float kPattern[] = {kKick, kHat, kHat, kHat, kSnare, kHat, kHat, kHat};
    for (int i = 0; i < 8; i++)
        sequencer.SetStep(i, kPattern[i]);
    sequencer.SetNumSteps(8);

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // Runs forever, alongside the sound: reads the knobs and buttons
    // and passes their values on to the drums and the sequencer
    while (1)
    {
        // Read the buttons first, so JustPressed() below sees the
        // latest press
        playStopButton.Update();
        resetButton.Update();

        // Knobs go from 0 to 1, which is what these settings expect
        kick.SetDecay(kickDecayKnob.Read());
        hat.SetTone(hatToneKnob.Read());
        snare.SetSnappy(snareSnappyKnob.Read());

        // 60 - 240 BPM; see dsp/Utility.h's MapExponential for why time
        // knobs want the exponential mapping rather than MapLinear.
        sequencer.SetTempo(dsp::MapExponential(tempoKnob.Read(), 60.f, 240.f));

        // Each press of playStopButton stops the pattern if it is
        // playing, or starts it if it is stopped
        if (playStopButton.JustPressed())
        {
            if (sequencer.IsRunning())
                sequencer.Stop();
            else
                sequencer.Start();
        }

        // resetButton jumps back to the first step
        if (resetButton.JustPressed())
            sequencer.Reset();

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
