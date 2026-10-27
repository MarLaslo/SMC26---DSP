/** SMC26 DSP course example: modal sequencer.
 *
 *  An 8-step sequencer plays an example pattern on its own, through
 *  a small pool of modal-synth voices shared via dsp::VoiceManager.
 *  structureKnob/brightnessKnob/dampingKnob shape the voices' timbre,
 *  tempoKnob speeds/slows the sequencer, gainKnob sets the output
 *  level, playStopButton stops/resumes it, and resetButton jumps it
 *  back to the first step.
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

Potentiometer structureKnob;
Potentiometer brightnessKnob;
Potentiometer dampingKnob;
Potentiometer tempoKnob;
Potentiometer gainKnob;

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

    structureKnob.Init(hw, seed::D15);
    brightnessKnob.Init(hw, seed::D16);
    dampingKnob.Init(hw, seed::D17);
    tempoKnob.Init(hw, seed::D18);
    gainKnob.Init(hw, seed::D19);
    StartAnalogInputs(hw); // once, after every analog sensor's Init()
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add.
void PrintValues()
{
    PrintValue("playStop", playStopButton.IsPressed());
    PrintValue("reset", resetButton.IsPressed());
    PrintValue("structure", structureKnob.Read());
    PrintValue("brightness", brightnessKnob.Read());
    PrintValue("damping", dampingKnob.Read());
    PrintValue("tempo", tempoKnob.Read());
    PrintValue("gain", gainKnob.Read());
    PrintLineEnd();
}

constexpr int kNumVoices = 2;

dsp::VoiceManager<dsp::ModalVoice, kNumVoices> voices;

// A free-running LFO, here used for vibrato on each voice's pitch.
// Swap SetFreq/the depth below, or point a Process() call at any
// other parameter instead (amplitude for tremolo, cutoff for a filter
// wobble, ...) - see dsp/Other/Lfo.h.
dsp::Lfo vibrato;
constexpr float kVibratoDepthHz = 8.f;

// Plays itself: an 8-step example pattern, looping at a tempo set by
// tempoKnob. Edit kPattern below (and sequencer.SetNumSteps if you
// change how many notes are in it) to make your own. See
// dsp/Other/Sequencer.h.
dsp::Sequencer sequencer;

// Read from gainKnob in the control loop below, applied to the output
// in AudioCallback via dsp::ApplyGain.
float outputGainDb = 0.f;
constexpr float kMinGainDb = -48.f;
constexpr float kMaxGainDb = 12.f;

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        float vibratoOffset = kVibratoDepthHz * vibrato.Process();
        for (int v = 0; v < kNumVoices; v++)
            voices.Voice(v).SetFreq(voices.Freq(v) + vibratoOffset);

        if (sequencer.Process() && sequencer.CurrentGate())
            voices.NoteOn(sequencer.CurrentFreq());

        float sample = dsp::ApplyGain(voices.Process(), outputGainDb);
        OUT_L[i] = sample;
        OUT_R[i] = sample;
    }
}

int main(void)
{
    InitHardware(true);

    voices.Init(hw.AudioSampleRate());
    vibrato.Init(hw.AudioSampleRate());
    vibrato.SetFreq(5.f);

    sequencer.Init(hw.AudioSampleRate());
    sequencer.SetTempo(120.f);
    // Note names instead of Hz - see dsp::NoteToFreq in dsp/Utility.h.
    static const char *kPattern[] = {"A3", "C4", "D4", "E4", "G4", "E4", "D4", "C4"};
    for (int i = 0; i < 8; i++)
        sequencer.SetStep(i, dsp::NoteToFreq(kPattern[i]));
    sequencer.SetNumSteps(8);

    hw.StartAudio(AudioCallback);

    while (1)
    {
        playStopButton.Update();
        resetButton.Update();

        float structure = structureKnob.Read();
        float brightness = brightnessKnob.Read();
        float damping = dampingKnob.Read();

        for (int v = 0; v < kNumVoices; v++)
        {
            voices.Voice(v).SetStructure(structure);
            voices.Voice(v).SetBrightness(brightness);
            voices.Voice(v).SetDamping(damping);
        }

        // 60 - 240 BPM; see dsp/Utility.h's MapExponential for why time
        // knobs want the exponential mapping rather than MapLinear.
        sequencer.SetTempo(dsp::MapExponential(tempoKnob.Read(), 60.f, 240.f));

        outputGainDb = dsp::MapLinear(gainKnob.Read(), kMinGainDb, kMaxGainDb);

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
