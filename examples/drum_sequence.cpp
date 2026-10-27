/** SMC26 DSP course example: drum sequencer.
 *
 *  An 8-step drum sequencer, playing kick/snare/hat on its own.
 *  tempoKnob speeds/slows it; playStopButton stops/resumes it;
 *  resetButton jumps it back to the first step.
 *
 *  Declare your sensors (see sensors/sensors.h) and dsp:: objects
 *  above AudioCallback, Init() them in main(), and call their
 *  Process() methods inside AudioCallback to hear something.
 */
#include "daisy_seed.h"
#include "daisysp.h"
#include "dsp/Utility.h"
#include "dsp/Synthesis/Synthesis.h"
#include "dsp/Effects/Effects.h"
#include "dsp/Filters/Filters.h"
#include "dsp/Other/Other.h"
#include "sensors/sensors.h"

using namespace daisy;

DaisySeed hw;

Button playStopButton;
Button resetButton;

Potentiometer kickDecayKnob;
Potentiometer hatToneKnob;
Potentiometer snareSnappyKnob;
Potentiometer tempoKnob;

// Set by InitHardware() - when true, main() prints the sensor values.
bool printValues = false;

// Sets up the board and the sensors. Pass true to print the sensor
// values over USB serial (open a serial monitor to see them).
void InitHardware(bool print)
{
    // Flush denormal floats to zero. Without this, a value decaying
    // toward (but never quite reaching) zero - a delay repeat trailing
    // off, an envelope's release tail - becomes a "denormal" number
    // that this FPU takes dramatically longer to do arithmetic on,
    // long enough to blow the audio callback's time budget and cause
    // an audible click/pop with no knob involved at all.
    __set_FPSCR(__get_FPSCR() | (1 << 24) | (1 << 25));

    hw.Init();

    printValues = print;
    if (printValues)
        hw.StartLog();

    playStopButton.Init(seed::D9);
    resetButton.Init(seed::D8);

    kickDecayKnob.Init(hw, seed::D15);
    hatToneKnob.Init(hw, seed::D16);
    snareSnappyKnob.Init(hw, seed::D17);
    tempoKnob.Init(hw, seed::D18);
    StartAnalogInputs(hw); // once, after every analog sensor's Init()
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add.
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

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
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

        float sample = kick.Process() + snare.Process() + hat.Process();
        OUT_L[i] = sample;
        OUT_R[i] = sample;
    }
}

int main(void)
{
    InitHardware(true);

    kick.Init(hw.AudioSampleRate());
    snare.Init(hw.AudioSampleRate());
    hat.Init(hw.AudioSampleRate());

    sequencer.Init(hw.AudioSampleRate());
    sequencer.SetTempo(120.f);
    static const float kPattern[] = {kKick, kHat, kHat, kHat, kSnare, kHat, kHat, kHat};
    for (int i = 0; i < 8; i++)
        sequencer.SetStep(i, kPattern[i]);
    sequencer.SetNumSteps(8);

    hw.StartAudio(AudioCallback);

    while (1)
    {
        playStopButton.Update();
        resetButton.Update();

        kick.SetDecay(kickDecayKnob.Read());
        hat.SetTone(hatToneKnob.Read());
        snare.SetSnappy(snareSnappyKnob.Read());

        // 60 - 240 BPM; see dsp/Utility.h's MapExponential for why time
        // knobs want the exponential mapping rather than MapLinear.
        sequencer.SetTempo(dsp::MapExponential(tempoKnob.Read(), 60.f, 240.f));

        if (playStopButton.JustPressed())
        {
            if (sequencer.IsRunning())
                sequencer.Stop();
            else
                sequencer.Start();
        }

        if (resetButton.JustPressed())
            sequencer.Reset();

        if (printValues && TimeToPrint())
            PrintValues();

        System::Delay(1);
    }
}
