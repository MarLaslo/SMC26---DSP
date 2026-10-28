/** SMC26 DSP course example: effects chain.
 *
 *  How to use effects. A simple plucked synth note goes through three
 *  effects, one after the other, like guitar pedals on a cable:
 *
 *    synth -> overdrive -> chorus -> delay -> output
 *
 *  Every effect works the same way: give it a sample with Process(),
 *  and it gives you back the changed sample. Pass that on to the next
 *  effect, and so on. Swap any of them for another effect from
 *  dsp/Effects/ (Tremolo, Flanger, Phaser, WaveFolder, ...) - the
 *  chain stays the same.
 *
 *  Press playButton to play the next note of a short melody (A, C, E,
 *  G, then round again). Press bypassButton to switch all effects off
 *  and on (each press flips it) to hear the difference.
 *
 *  driveKnob      overdrive: from nearly clean up to fuzzy and loud
 *  chorusKnob     how much chorus is mixed in. All the way down = off
 *  delayTimeKnob  time between echoes, 50 ms (slapback) to 800 ms
 *  feedbackKnob   how many echoes: one, up to a long trail
 *  delayMixKnob   how loud the echoes are. All the way down = off
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    playButton   - seed::D9     driveKnob     - seed::A0
 *    bypassButton - seed::D8     chorusKnob    - seed::A1
 *                                delayTimeKnob - seed::A2
 *                                feedbackKnob  - seed::A3
 *                                delayMixKnob  - seed::A4
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
Button bypassButton;

Potentiometer driveKnob;
Potentiometer chorusKnob;
Potentiometer delayTimeKnob;
Potentiometer feedbackKnob;
Potentiometer delayMixKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    playButton.Init(seed::D9);
    bypassButton.Init(seed::D8);

    driveKnob.Init(hw, seed::A0);
    chorusKnob.Init(hw, seed::A1);
    delayTimeKnob.Init(hw, seed::A2);
    feedbackKnob.Init(hw, seed::A3);
    delayMixKnob.Init(hw, seed::A4);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    PrintValue("play", playButton.IsPressed());
    PrintValue("bypass", bypassButton.Toggled());
    PrintValue("drive", driveKnob.Read());
    PrintValue("chorus", chorusKnob.Read());
    PrintValue("delayTime", delayTimeKnob.Read());
    PrintValue("feedback", feedbackKnob.Read());
    PrintValue("delayMix", delayMixKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// The sound going into the effects: a short, plucked synth note
dsp::Subtractive synth;

// The effects, in the order the sound goes through them
dsp::Overdrive drive;
dsp::Chorus    chorus;
dsp::Delay     delay;

// The melody playButton steps through - note names instead of Hz,
// see dsp::NoteToFreq in dsp/Utility.h
constexpr int kNumNotes = 4;
float notes[kNumNotes] = {
    dsp::NoteToFreq("A2"),
    dsp::NoteToFreq("C3"),
    dsp::NoteToFreq("E3"),
    dsp::NoteToFreq("G3"),
};

// Set in main(), used in AudioCallback
float chorusMix  = 0.f;  // 0 = no chorus, 1 = only chorus
bool  effectsOn  = true; // false = bypass every effect

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
        // The sound going into the effects. To put your own sound
        // through them instead - a guitar, a microphone - plug it
        // into the board's audio input and use:
        //   float sample = IN_L[i];
        float sample = synth.Process();

        if (effectsOn)
        {
            // 1. Overdrive: each effect takes the sample in and gives
            // the changed sample back, so we just overwrite it
            sample = drive.Process(sample);

            // 2. Chorus, mixed in by hand. "dry" is the sound without
            // the effect, "wet" is with it. Fading between them lets
            // a knob set how much effect you hear - the same trick
            // works for any effect.
            float dry = sample;
            float wet = chorus.Process(sample);
            sample    = (1.f - chorusMix) * dry + chorusMix * wet;

            // 3. Delay. It has its own dry/wet mix built in
            // (delay.SetMix in main() below), so no fading needed.
            sample = delay.Process(sample);
        }

        // Turn the end result down by 9 dB to leave some headroom -
        // overdrive makes the sound louder (see dsp::ApplyGain in
        // dsp/Utility.h)
        sample = dsp::ApplyGain(sample, -9.f);

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

    // A short pluck: fades out on its own (sustain 0), so you can
    // hear the echoes after it
    synth.Init(sampleRate);
    synth.SetCutoff(1200.f);
    synth.SetAmpDecay(0.3f);
    synth.SetAmpSustain(0.f);
    synth.SetAmpRelease(0.1f);

    // Every effect needs Init() before audio starts. Overdrive is
    // the only one here that doesn't need the sample rate.
    drive.Init();
    chorus.Init(sampleRate);
    delay.Init(sampleRate);

    // A slow, deep chorus - try your own values here
    chorus.SetLfoFreq(0.4f);
    chorus.SetLfoDepth(0.7f);

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // Which note of the melody plays next
    int noteIndex = 0;

    // Runs forever, alongside the sound: plays the notes and passes
    // the knob values on to the effects
    while (1)
    {
        // Read the buttons first, so JustPressed() and JustReleased()
        // below see the latest change
        playButton.Update();
        bypassButton.Update();

        // Each press plays the next note, then moves on. "% kNumNotes"
        // wraps back to the first note after the last: 0, 1, 2, 3, 0, ...
        if (playButton.JustPressed())
        {
            synth.SetFreq(notes[noteIndex]);
            synth.NoteOn();
            noteIndex = (noteIndex + 1) % kNumNotes;
        }
        if (playButton.JustReleased())
            synth.NoteOff();

        // Toggled() flips on every press: starts false, so the
        // effects start on
        effectsOn = !bypassButton.Toggled();

        // Drive: only 0.25 - 0.6 is useful. 0 would be silent, and
        // above 0.6 it's extreme (see dsp/Effects/Overdrive.h).
        drive.SetDrive(dsp::MapLinear(driveKnob.Read(), 0.25f, 0.6f));

        // Chorus mix is already 0 - 1, same as the knob
        chorusMix = chorusKnob.Read();

        // Delay time: MapExponential so short times get as much of
        // the knob as long ones (see dsp/Utility.h)
        delay.SetTimeMs(dsp::MapExponential(delayTimeKnob.Read(), 50.f, 800.f));

        // Feedback stops at 0.85 so the echoes always die away
        delay.SetFeedback(dsp::MapLinear(feedbackKnob.Read(), 0.f, 0.85f));

        // Delay mix stops at 0.7 so the note itself stays louder
        // than its echoes
        delay.SetMix(dsp::MapLinear(delayMixKnob.Read(), 0.f, 0.7f));

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
