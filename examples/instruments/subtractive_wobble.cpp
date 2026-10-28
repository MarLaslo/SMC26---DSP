/** SMC26 DSP course example: subtractive synth with a wobble.
 *
 *  Subtractive synthesis starts with a raw, buzzy waveform full of
 *  harmonics, then "subtracts" some of them with a filter - like
 *  carving a shape out of a block of wood. It's the sound of most
 *  classic analog synths. dsp::Subtractive does it all in one voice:
 *  waveform -> lowpass filter -> volume envelope.
 *
 *  On top of that, an LFO (a slow, silent wave) moves the filter's
 *  cutoff up and down for you, giving the "wah-wah" / dubstep
 *  "wobble" sound.
 *
 *  Hold playButton to play a note. Press waveButton to step through
 *  the three raw waveforms - listen to how each one sounds through
 *  the filter:
 *    1. saw      - bright and buzzy, every harmonic (strings, brass)
 *    2. square   - hollow, odd harmonics only (clarinet, retro games)
 *    3. triangle - soft, very few harmonics (flute-like)
 *
 *  pitchKnob     picks the note (A minor scale, one octave up/down)
 *  cutoffKnob    the filter: low = dark and muffled, high = bright
 *  resonanceKnob makes the filter "sing" around the cutoff
 *  wobbleRateKnob  how fast the LFO moves the cutoff (slow to fast)
 *  wobbleDepthKnob how far it moves it. All the way down = no wobble
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    playButton - seed::D9     pitchKnob       - seed::A0
 *    waveButton - seed::D8     cutoffKnob      - seed::A1
 *                              resonanceKnob   - seed::A2
 *                              wobbleRateKnob  - seed::A3
 *                              wobbleDepthKnob - seed::A4
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
Button waveButton;

Potentiometer pitchKnob;
Potentiometer cutoffKnob;
Potentiometer resonanceKnob;
Potentiometer wobbleRateKnob;
Potentiometer wobbleDepthKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    playButton.Init(seed::D9);
    waveButton.Init(seed::D8);

    pitchKnob.Init(hw, seed::A0);
    cutoffKnob.Init(hw, seed::A1);
    resonanceKnob.Init(hw, seed::A2);
    wobbleRateKnob.Init(hw, seed::A3);
    wobbleDepthKnob.Init(hw, seed::A4);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    PrintValue("play", playButton.IsPressed());
    PrintValue("wave", waveButton.Count());
    PrintValue("pitch", pitchKnob.Read());
    PrintValue("cutoff", cutoffKnob.Read());
    PrintValue("resonance", resonanceKnob.Read());
    PrintValue("wobbleRate", wobbleRateKnob.Read());
    PrintValue("wobbleDepth", wobbleDepthKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// The subtractive voice: waveform -> filter -> volume envelope
dsp::Subtractive synth;

// The LFO that wobbles the filter cutoff. It makes no sound itself -
// it just moves between -1 and 1, slowly, over and over.
dsp::Lfo wobble;

// The three waveforms waveButton steps through (see the top of the
// file)
constexpr int kNumWaves = 3;
dsp::Subtractive::Waveform waves[kNumWaves] = {
    dsp::Subtractive::WAVE_SAW,
    dsp::Subtractive::WAVE_SQUARE,
    dsp::Subtractive::WAVE_TRIANGLE,
};

// Set from the knobs in main(), used in AudioCallback
float baseCutoff  = 800.f; // Hz, the cutoff with no wobble
float wobbleDepth = 0.f;   // how many octaves the LFO moves it

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
        // The LFO runs here, once per sample like the sound, so the
        // wobble is perfectly smooth. Its value goes from -1 to 1.
        float lfoValue = wobble.Process();

        // Move the cutoff up and down by "wobbleDepth" octaves. Each
        // octave doubles the frequency, so: base * 2^(depth * lfo).
        // Clamp keeps it inside the range the filter can handle.
        float cutoff = baseCutoff * powf(2.f, wobbleDepth * lfoValue);
        synth.SetCutoff(dsp::Clamp(cutoff, 30.f, 10000.f));

        // The next sample of the synth, turned down by 6 dB to leave
        // some headroom (see dsp::ApplyGain in dsp/Utility.h)
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

    // The synth has its own filter envelope built in: on every note
    // it pushes the cutoff up by SetFilterEnvAmount Hz, then lets it
    // fall back over SetFilterDecay seconds. That gives each note a
    // bright "pluck" at the start. Set the amount to 0 to turn it off.
    synth.Init(sampleRate);
    synth.SetFilterEnvAmount(1500.f);
    synth.SetFilterDecay(0.25f);
    synth.SetAmpRelease(0.2f);

    // A sine LFO glides smoothly up and down. Try Lfo::WAVE_SQUARE
    // for a choppy on/off wobble, or Lfo::WAVE_SAW for a "wub" that
    // jumps up and slides down.
    wobble.Init(sampleRate);
    wobble.SetWaveform(dsp::Lfo::WAVE_SINE);

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // The note pitchKnob is centered on
    float rootNote = dsp::NoteToFreq("A2");

    // Runs forever, alongside the sound: reads the sensors and
    // passes their values on to the synth and the LFO
    while (1)
    {
        // Read the buttons first, so JustPressed() and JustReleased()
        // below see the latest change
        playButton.Update();
        waveButton.Update();

        // Start and stop the note
        if (playButton.JustPressed())
            synth.NoteOn();
        if (playButton.JustReleased())
            synth.NoteOff();

        // Count() goes up by one on every press. "% kNumWaves" wraps
        // it back to 0 after the last one: 0, 1, 2, 0, 1, ...
        synth.SetWaveform(waves[waveButton.Count() % kNumWaves]);

        // Pitch: snaps to the notes of A minor, an octave either way
        synth.SetFreq(dsp::MapSemitones(pitchKnob.Read(), rootNote, 12.f, dsp::SCALE_MINOR));

        // Cutoff: MapExponential so the knob feels even across its
        // whole travel (see dsp/Utility.h)
        baseCutoff = dsp::MapExponential(cutoffKnob.Read(), 80.f, 5000.f);

        // Resonance stops at 0.9 - from about 1 upward the filter
        // whistles on its own, even when no note is playing
        synth.SetResonance(dsp::MapLinear(resonanceKnob.Read(), 0.f, 0.9f));

        // Wobble speed: from one wobble every 5 seconds (0.2 Hz) up
        // to 10 wobbles a second
        wobble.SetFreq(dsp::MapExponential(wobbleRateKnob.Read(), 0.2f, 10.f));

        // Wobble depth: from 0 (no wobble) to 3 octaves either way
        wobbleDepth = dsp::MapLinear(wobbleDepthKnob.Read(), 0.f, 3.f);

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
