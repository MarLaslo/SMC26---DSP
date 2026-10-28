/** SMC26 DSP course example: granular drone.
 *
 *  Granular synthesis chops a sound into tiny pieces called grains -
 *  a few thousandths of a second each - and plays lots of them, one
 *  after another and overlapping. How long the grains are, and how
 *  they move through the sound, changes its texture from smooth to
 *  buzzy to fluttering - without changing its pitch.
 *
 *  Here three dsp::Granular voices - a root note, a fifth above and an
 *  octave above - make a slow, evolving drone, with a long echo
 *  behind it for space. The drone starts by itself when the board
 *  turns on.
 *
 *  Press droneButton to fade the drone out, and again to fade it back
 *  in. Press freezeButton to freeze the grains in place (they stop
 *  moving), and again to let them move.
 *
 *  pitchKnob  picks the root note (A minor scale, one octave up/down)
 *  grainKnob  grain length: short = buzzy and grainy, long = smooth
 *  motionKnob how fast the grains move: still, up to a shimmering flutter
 *  detuneKnob pulls the three voices slightly out of tune with each
 *             other, for a wider, wavier sound
 *  spaceKnob  how much echo. All the way down = dry
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    droneButton  - seed::D9     pitchKnob  - seed::A0
 *    freezeButton - seed::D8     grainKnob  - seed::A1
 *                                motionKnob - seed::A2
 *                                detuneKnob - seed::A3
 *                                spaceKnob  - seed::A4
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

Button droneButton;
Button freezeButton;

Potentiometer pitchKnob;
Potentiometer grainKnob;
Potentiometer motionKnob;
Potentiometer detuneKnob;
Potentiometer spaceKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    droneButton.Init(seed::D9);
    freezeButton.Init(seed::D8);

    pitchKnob.Init(hw, seed::A0);
    grainKnob.Init(hw, seed::A1);
    motionKnob.Init(hw, seed::A2);
    detuneKnob.Init(hw, seed::A3);
    spaceKnob.Init(hw, seed::A4);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    PrintValue("droneOff", droneButton.Toggled());
    PrintValue("frozen", freezeButton.Toggled());
    PrintValue("pitch", pitchKnob.Read());
    PrintValue("grain", grainKnob.Read());
    PrintValue("motion", motionKnob.Read());
    PrintValue("detune", detuneKnob.Read());
    PrintValue("space", spaceKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// Three granular voices. voices[0] plays the root note, voices[1] and
// voices[2] the notes above it.
constexpr int kNumVoices = 3;
dsp::Granular voices[kNumVoices];

// How many semitones above the root each voice plays: the root
// itself, a fifth (7) and an octave (12) - a calm, open chord
float voiceSemitones[kNumVoices] = {0.f, 7.f, 12.f};

// Each voice's grains move at a different speed - one even goes
// backwards (negative) - so the three never move in step, and the
// drone keeps slowly changing
float voiceMotion[kNumVoices] = {1.f, -0.7f, 0.45f};

// Which way each voice is pulled out of tune by detuneKnob: the
// root stays put, one goes sharp (+1), one goes flat (-1)
float voiceDetune[kNumVoices] = {0.f, 1.f, -1.f};

// A long echo behind the drone, for a sense of space
dsp::Delay echo;

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
        // Add the three voices together
        float sample = 0.f;
        for (int v = 0; v < kNumVoices; v++)
            sample += voices[v].Process();

        // Three voices added up get loud, so turn the mix down by
        // 12 dB (see dsp::ApplyGain in dsp/Utility.h), then add the echo
        sample = echo.Process(dsp::ApplyGain(sample, -12.f));

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

    // A drone should fade in and out slowly: 3 seconds in, 4 seconds
    // out, and stay at full volume in between (sustain 1)
    for (int v = 0; v < kNumVoices; v++)
    {
        voices[v].Init(sampleRate);
        voices[v].SetAmpAttack(3.f);
        voices[v].SetAmpSustain(1.f);
        voices[v].SetAmpRelease(4.f);
    }

    // A slow echo that repeats for a long time
    echo.Init(sampleRate);
    echo.SetTimeMs(700.f);
    echo.SetFeedback(0.7f);

    // Start the drone straight away
    for (int v = 0; v < kNumVoices; v++)
        voices[v].NoteOn();

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // The note pitchKnob is centered on
    float rootNote = dsp::NoteToFreq("A2");

    // Runs forever, alongside the sound: reads the sensors and
    // passes their values on to the voices
    while (1)
    {
        // Read the buttons first, so JustPressed() below sees the
        // latest press
        droneButton.Update();
        freezeButton.Update();

        // Toggled() flips on every press. It starts false while the
        // drone is already playing - so true means "fade it out".
        if (droneButton.JustPressed())
        {
            for (int v = 0; v < kNumVoices; v++)
            {
                if (droneButton.Toggled())
                    voices[v].NoteOff();
                else
                    voices[v].NoteOn();
            }
        }

        // Root note: snaps to the notes of A minor, an octave either way
        float root = dsp::MapSemitones(pitchKnob.Read(), rootNote, 12.f, dsp::SCALE_MINOR);

        // Detune: up to 25 cents (a quarter of a semitone) - enough
        // to make the voices "beat" against each other slowly
        float detuneCents = dsp::MapLinear(detuneKnob.Read(), 0.f, 25.f);

        // Grain length from 5 ms (buzzy) to 80 ms (smooth).
        // MapExponential so the short end gets as much of the knob as
        // the long end (see dsp/Utility.h).
        float grainMs = dsp::MapExponential(grainKnob.Read(), 5.f, 80.f);

        // Motion: how fast the grains move through the sound. 0 when
        // frozen, otherwise from 0 (still) up to 0.3.
        float motion = dsp::MapLinear(motionKnob.Read(), 0.f, 0.3f);
        if (freezeButton.Toggled())
            motion = 0.f;

        for (int v = 0; v < kNumVoices; v++)
        {
            // Each voice: its interval above the root, plus its share
            // of the detune. 100 cents = 1 semitone, and every
            // semitone up multiplies the pitch by 2^(1/12).
            float semitones = voiceSemitones[v] + voiceDetune[v] * detuneCents / 100.f;
            voices[v].SetFreq(root * powf(2.f, semitones / 12.f));

            voices[v].SetGrainSize(grainMs);
            voices[v].SetSpeed(motion * voiceMotion[v]);
        }

        // Echo: from dry up to half echo, half drone
        echo.SetMix(dsp::MapLinear(spaceKnob.Read(), 0.f, 0.5f));

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
