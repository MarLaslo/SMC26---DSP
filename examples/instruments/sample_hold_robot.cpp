/** SMC26 DSP course example: random robot (sample and hold).
 *
 *  Sample and hold grabs a value at one moment and keeps it until the
 *  next grab - turning anything into a staircase of steps. Here it
 *  picks a new note on every step, and each step plays a short,
 *  plucky note: the bleepy "computer thinking" sound of old sci-fi
 *  films and analog synths.
 *
 *  Hold playButton to play. Press sourceButton to switch what the
 *  sample and hold grabs:
 *    1. random - a new random note every step (the classic sound)
 *    2. LFO    - grabs a slow wave as it moves, so the notes climb
 *                up and walk back down like a staircase
 *
 *  rateKnob     how many steps (notes) per second
 *  rangeKnob    how far the notes can jump: one note to two octaves
 *  cutoffKnob   the filter: low = dark and soft, high = bright
 *  lfoRateKnob  how fast the staircase climbs (LFO mode only)
 *  lengthKnob   how long each note rings: short blip to longer pluck
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    playButton   - seed::D9     rateKnob    - seed::A0
 *    sourceButton - seed::D8     rangeKnob   - seed::A1
 *                                cutoffKnob  - seed::A2
 *                                lfoRateKnob - seed::A3
 *                                lengthKnob  - seed::A4
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
Button sourceButton;

Potentiometer rateKnob;
Potentiometer rangeKnob;
Potentiometer cutoffKnob;
Potentiometer lfoRateKnob;
Potentiometer lengthKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    playButton.Init(seed::D9);
    sourceButton.Init(seed::D8);

    rateKnob.Init(hw, seed::A0);
    rangeKnob.Init(hw, seed::A1);
    cutoffKnob.Init(hw, seed::A2);
    lfoRateKnob.Init(hw, seed::A3);
    lengthKnob.Init(hw, seed::A4);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    PrintValue("play", playButton.IsPressed());
    PrintValue("lfoMode", sourceButton.Toggled());
    PrintValue("rate", rateKnob.Read());
    PrintValue("range", rangeKnob.Read());
    PrintValue("cutoff", cutoffKnob.Read());
    PrintValue("lfoRate", lfoRateKnob.Read());
    PrintValue("length", lengthKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// The synth that plays the notes: waveform -> filter -> envelope
dsp::Subtractive synth;

// The sample and hold. Its built-in clock decides when to grab a new
// value (SetRate), and Stepped() tells us when that just happened.
dsp::SampleHold steps;

// The slow wave the sample and hold grabs in LFO mode
dsp::Lfo lfo;

// Set from the knobs in main(), used in AudioCallback
float noteRange = 12.f; // how many semitones up/down a note can jump

// The note everything is centered on
float rootNote = 220.f; // A3

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
        // The LFO keeps moving all the time, even when nobody listens
        float lfoValue = lfo.Process();

        // Ask the sample and hold for its value, -1 to 1. It only
        // changes on a step - in between, it repeats the held value.
        //   Process()         grabs a random value
        //   Process(lfoValue) grabs wherever the LFO is right now
        float held;
        if (sourceButton.Toggled())
            held = steps.Process(lfoValue);
        else
            held = steps.Process();

        // On every step, while playButton is held: a new note.
        // MapSemitones wants 0 - 1, so (held + 1) / 2 moves -1..1
        // into that range, then snaps it to the A minor scale.
        if (steps.Stepped() && playButton.IsPressed())
        {
            float position = (held + 1.f) * 0.5f;
            synth.SetFreq(dsp::MapSemitones(position, rootNote, noteRange, dsp::SCALE_MINOR));
            synth.NoteOn();
        }

        // The next sample of the synth, turned down by 6 dB to leave
        // some headroom
        float sample = dsp::ApplyGain(synth.Process(), -6.f);

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

    // Each note is a pluck: instant attack, then it dies away over
    // the decay (set by lengthKnob). Sustain 0 means it never holds,
    // so every step sounds like a separate note. The filter envelope
    // adds a bright "blip" at the start of each one.
    synth.Init(sampleRate);
    synth.SetWaveform(dsp::Subtractive::WAVE_SQUARE);
    synth.SetAmpAttack(0.002f);
    synth.SetAmpSustain(0.f);
    synth.SetAmpRelease(0.05f);
    synth.SetFilterEnvAmount(2000.f);
    synth.SetFilterDecay(0.08f);
    synth.SetResonance(0.5f);

    steps.Init(sampleRate);

    lfo.Init(sampleRate);
    lfo.SetWaveform(dsp::Lfo::WAVE_TRIANGLE);

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // Runs forever, alongside the sound: reads the sensors and
    // passes their values on to the synth and the sample and hold
    while (1)
    {
        // Read the buttons first, so JustPressed() and JustReleased()
        // below see the latest change
        playButton.Update();
        sourceButton.Update();

        // Reset() restarts the step clock, so the first note plays
        // the moment you press - not whenever the next step was due
        if (playButton.JustPressed())
            steps.Reset();
        if (playButton.JustReleased())
            synth.NoteOff();

        // Steps per second: from 1 (slow, one note at a time) to
        // 16 (a fast, chattering stream of notes)
        steps.SetRate(dsp::MapExponential(rateKnob.Read(), 1.f, 16.f));

        // How far notes can jump: from 1 semitone to 2 octaves
        noteRange = dsp::MapLinear(rangeKnob.Read(), 1.f, 24.f);

        // Filter: MapExponential so the knob feels even across its
        // whole travel (see dsp/Utility.h)
        synth.SetCutoff(dsp::MapExponential(cutoffKnob.Read(), 150.f, 4000.f));

        // The staircase speed: from one climb every 20 seconds to
        // two a second
        lfo.SetFreq(dsp::MapExponential(lfoRateKnob.Read(), 0.05f, 2.f));

        // Note length: from a 30 ms blip to a 0.8 s pluck
        synth.SetAmpDecay(dsp::MapExponential(lengthKnob.Read(), 0.03f, 0.8f));

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
