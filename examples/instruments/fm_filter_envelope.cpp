/** SMC26 DSP course example: FM synth with a filter envelope.
 *
 *  An FM (frequency modulation) voice played through a resonant
 *  ladder filter. The filter has its own ADSR envelope: every time
 *  you press the button, the cutoff jumps up and then falls back
 *  down, giving the "squelchy" / "plucky" sound of classic synths.
 *
 *  Hold playButton to play a note. Press ratioButton to step through
 *  four FM sounds - each press picks the next one, then wraps around:
 *    1. ratio 1    - warm and round, like a soft brass
 *    2. ratio 2    - hollow, a bit like a clarinet
 *    3. ratio 3.5  - bell / metallic
 *    4. ratio 0.5  - deep, growly bass
 *
 *  pitchKnob     picks the note (A minor scale, one octave up/down)
 *  fmKnob        how much FM - from a clean sine to a buzzy, bright tone
 *  cutoffKnob    where the filter sits when the envelope is at rest
 *  envAmountKnob how far the envelope opens the filter on each note
 *  resonanceKnob makes the filter "ring" / whistle around the cutoff
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    playButton  - seed::D9     pitchKnob     - seed::A0
 *    ratioButton - seed::D8     fmKnob        - seed::A1
 *                               cutoffKnob    - seed::A2
 *                               envAmountKnob - seed::A3
 *                               resonanceKnob - seed::A4
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
Button ratioButton;

Potentiometer pitchKnob;
Potentiometer fmKnob;
Potentiometer cutoffKnob;
Potentiometer envAmountKnob;
Potentiometer resonanceKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    playButton.Init(seed::D9);
    ratioButton.Init(seed::D8);

    pitchKnob.Init(hw, seed::A0);
    fmKnob.Init(hw, seed::A1);
    cutoffKnob.Init(hw, seed::A2);
    envAmountKnob.Init(hw, seed::A3);
    resonanceKnob.Init(hw, seed::A4);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    PrintValue("play", playButton.IsPressed());
    PrintValue("ratio", ratioButton.Count());
    PrintValue("pitch", pitchKnob.Read());
    PrintValue("fm", fmKnob.Read());
    PrintValue("cutoff", cutoffKnob.Read());
    PrintValue("envAmount", envAmountKnob.Read());
    PrintValue("resonance", resonanceKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// The FM voice: makes the raw sound
dsp::FM fm;

// The filter: removes high frequencies above its cutoff
dsp::LadderFilter filter;

// A second envelope, just for the filter. It moves the cutoff up and
// down over the note, separately from the volume.
dsp::Adsr filterEnv;

// The four FM sounds ratioButton steps through (see the top of the
// file). The ratio is the modulator's pitch compared to the note.
constexpr int kNumRatios = 4;
float ratios[kNumRatios] = {1.f, 2.f, 3.5f, 0.5f};

// Set from the knobs in main(), used in AudioCallback
float baseCutoff = 500.f; // Hz, cutoff when the envelope is at 0
float envAmount  = 3.f;   // how many octaves the envelope opens it

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
        // Where the filter envelope is right now, 0 - 1
        float env = filterEnv.Process();

        // Each octave doubles the frequency, so "envAmount octaves
        // above baseCutoff" is baseCutoff * 2^(envAmount * env).
        // Clamp keeps the filter from ever opening all the way - the
        // top few kHz of FM are where the harshness lives.
        float cutoff = baseCutoff * powf(2.f, envAmount * env);
        filter.SetFreq(dsp::Clamp(cutoff, 20.f, 8000.f));

        // FM voice -> filter -> a bit quieter to leave some headroom
        float sample = filter.Process(fm.Process()) * 0.7f;

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

    float sampleRate = hw.AudioSampleRate();

    // The FM voice has a volume envelope and a modulation envelope
    // built in. A short modulation decay makes each note start
    // bright and then mellow out - try changing these times.
    fm.Init(sampleRate);
    fm.SetAmpAttack(0.01f);
    fm.SetAmpDecay(0.4f);
    fm.SetAmpSustain(0.7f);
    fm.SetAmpRelease(0.4f);
    fm.SetModDecay(0.4f);
    fm.SetModSustain(0.25f);

    // 24 dB/oct lowpass - the classic Moog-style filter
    filter.Init(sampleRate);
    filter.SetMode(dsp::LadderFilter::MODE_LOWPASS_24);

    // Fast attack, medium decay, low sustain: the cutoff snaps open
    // at the start of the note, then falls most of the way back.
    // Make the decay shorter (e.g. 0.1) for a snappier pluck.
    filterEnv.Init(sampleRate);
    filterEnv.SetAttack(0.01f);
    filterEnv.SetDecay(0.35f);
    filterEnv.SetSustain(0.2f);
    filterEnv.SetRelease(0.4f);

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // The note pitchKnob is centered on
    float rootNote = dsp::NoteToFreq("A2");

    // Runs forever, alongside the sound: reads the sensors and
    // passes their values on to the synth and the filter
    while (1)
    {
        // Read the buttons first, so JustPressed() and JustReleased()
        // below see the latest change
        playButton.Update();
        ratioButton.Update();

        // Start and stop the note - both envelopes together, so the
        // volume and the filter move at the same time
        if (playButton.JustPressed())
        {
            fm.NoteOn();
            filterEnv.NoteOn();
        }
        if (playButton.JustReleased())
        {
            fm.NoteOff();
            filterEnv.NoteOff();
        }

        // Count() goes up by one on every press. "% kNumRatios" wraps
        // it back to 0 after the last one: 0, 1, 2, 3, 0, 1, ...
        fm.SetModRatio(ratios[ratioButton.Count() % kNumRatios]);

        // Pitch: snaps to the notes of A minor, an octave either way
        fm.SetFreq(dsp::MapSemitones(pitchKnob.Read(), rootNote, 12.f, dsp::SCALE_MINOR));

        // FM depth: 0 = pure sine, 4 = bright and buzzy
        fm.SetModDepth(dsp::MapLinear(fmKnob.Read(), 0.f, 4.f));

        // Filter: MapExponential so the knob feels even across its
        // whole travel (see dsp/Utility.h)
        baseCutoff = dsp::MapExponential(cutoffKnob.Read(), 60.f, 2000.f);
        envAmount  = dsp::MapLinear(envAmountKnob.Read(), 0.f, 4.f);
        // Resonance stops just below 1 - from 1 upward the filter
        // whistles on its own, even after the note has ended
        filter.SetResonance(dsp::MapLinear(resonanceKnob.Read(), 0.f, 0.95f));

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
