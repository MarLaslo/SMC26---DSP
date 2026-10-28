/** SMC26 DSP course example: modal sequencer.
 *
 *  An 8-step sequencer plays an example pattern on its own, through
 *  a small pool of modal-synth voices shared via dsp::VoiceManager.
 *  structureKnob/brightnessKnob/dampingKnob shape the voices' timbre,
 *  tempoKnob speeds/slows the sequencer, gainKnob sets the output
 *  level, playStopButton stops/resumes it, and resetButton jumps it
 *  back to the first step.
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    playStopButton - seed::D9     structureKnob  - seed::A0
 *    resetButton    - seed::D8     brightnessKnob - seed::A1
 *                                  dampingKnob    - seed::A2
 *                                  tempoKnob      - seed::A3
 *                                  gainKnob       - seed::A4
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

Potentiometer structureKnob;
Potentiometer brightnessKnob;
Potentiometer dampingKnob;
Potentiometer tempoKnob;
Potentiometer gainKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    playStopButton.Init(seed::D9);
    resetButton.Init(seed::D8);

    structureKnob.Init(hw, seed::A0);
    brightnessKnob.Init(hw, seed::A1);
    dampingKnob.Init(hw, seed::A2);
    tempoKnob.Init(hw, seed::A3);
    gainKnob.Init(hw, seed::A4);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
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

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// How many notes can ring at the same time. A new note takes a free
// voice, or cuts off the oldest one if they are all busy.
constexpr int kNumVoices = 2;

dsp::VoiceManager<dsp::ModalVoice, kNumVoices> voices;

// A free-running LFO, here used for vibrato on each voice's pitch.
// Swap SetFreq/the depth below, or point a Process() call at any
// other parameter instead (amplitude for tremolo, cutoff for a filter
// wobble, ...) - see dsp/Other/Lfo.h.
dsp::Lfo vibrato;
constexpr float kVibratoDepthHz = 8.f; // how far the pitch wobbles

// Plays itself: an 8-step example pattern, looping at a tempo set by
// tempoKnob. Edit kPattern below (and sequencer.SetNumSteps if you
// change how many notes are in it) to make your own. See
// dsp/Other/Sequencer.h.
dsp::Sequencer sequencer;

// Read from gainKnob in the control loop below, applied to the output
// in AudioCallback via dsp::ApplyGain.
float outputGainDb = 0.f;
constexpr float kMinGainDb = -48.f; // gainKnob fully down
constexpr float kMaxGainDb = 12.f;  // gainKnob fully up

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
        // Vibrato: move every voice's pitch a little above and below
        // the note it is playing
        float vibratoOffset = kVibratoDepthHz * vibrato.Process();
        for (int v = 0; v < kNumVoices; v++)
            voices.Voice(v).SetFreq(voices.Freq(v) + vibratoOffset);

        // Process() moves the sequencer on by one sample and is true
        // only when a new step starts. If that step isn't a rest,
        // play its note.
        if (sequencer.Process() && sequencer.CurrentGate())
            voices.NoteOn(sequencer.CurrentFreq());

        // Mix all the voices into one sample and set its volume
        float sample = dsp::ApplyGain(voices.Process(), outputGainDb);

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

    // Set up the voices and the vibrato (5 wobbles per second) - they
    // need to know the sample rate
    voices.Init(hw.AudioSampleRate());
    vibrato.Init(hw.AudioSampleRate());
    vibrato.SetFreq(5.f);

    // Set up the sequencer and load the pattern into its 8 steps
    sequencer.Init(hw.AudioSampleRate());
    sequencer.SetTempo(120.f);
    // Note names instead of Hz - see dsp::NoteToFreq in dsp/Utility.h.
    static const char *kPattern[] = {"A3", "C4", "D4", "E4", "G4", "E4", "D4", "C4"};
    for (int i = 0; i < 8; i++)
        sequencer.SetStep(i, dsp::NoteToFreq(kPattern[i]));
    sequencer.SetNumSteps(8);

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // Runs forever, alongside the sound: reads the knobs and buttons
    // and passes their values on to the voices and the sequencer
    while (1)
    {
        // Read the buttons first, so JustPressed() below sees the
        // latest press
        playStopButton.Update();
        resetButton.Update();

        // Knobs go from 0 to 1, which is what these settings expect
        float structure = structureKnob.Read();
        float brightness = brightnessKnob.Read();
        float damping = dampingKnob.Read();

        // Give the same sound settings to every voice
        for (int v = 0; v < kNumVoices; v++)
        {
            voices.Voice(v).SetStructure(structure);
            voices.Voice(v).SetBrightness(brightness);
            voices.Voice(v).SetDamping(damping);
        }

        // 60 - 240 BPM; see dsp/Utility.h's MapExponential for why time
        // knobs want the exponential mapping rather than MapLinear.
        sequencer.SetTempo(dsp::MapExponential(tempoKnob.Read(), 60.f, 240.f));

        // Turn gainKnob's 0 to 1 into a volume from -48 dB to +12 dB
        outputGainDb = dsp::MapLinear(gainKnob.Read(), kMinGainDb, kMaxGainDb);

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
