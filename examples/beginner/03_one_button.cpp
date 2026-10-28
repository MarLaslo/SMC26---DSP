/** SMC26 DSP course - beginner step 3 of 3: buttons.
 *
 *  Now it's an instrument: the tone only plays while you hold a
 *  button, and a second button changes the sound's color. The knob
 *  from step 2 still sets the pitch.
 *
 *  What you learn here:
 *    - buttons: Update() them every loop, then ask JustPressed() /
 *      JustReleased()
 *    - NoteOn() / NoteOff() and the envelope: how a note fades in
 *      when it starts and fades out when it stops
 *    - "if": only do something when a condition is true
 *
 *  Try changing:
 *    - the envelope times in main() - e.g. SetAmpAttack(1.f) for a
 *      slow fade in, SetAmpRelease(2.f) for a long fade out
 *    - the order of the waveforms in "waves", or remove one
 *
 *  Where next: examples/instruments/ - full instruments built from
 *  exactly these pieces.
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    playButton - seed::D9     pitchKnob - seed::A0
 *    waveButton - seed::D8
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

// NEW: two buttons. Each one connects its pin to GND when pressed.
Button playButton;
Button waveButton;

// The knob from step 2
Potentiometer pitchKnob;

// Tells the board which pin each sensor is plugged into
void InitInputs()
{
    playButton.Init(seed::D9);
    waveButton.Init(seed::D8);
    pitchKnob.Init(hw, seed::A0);
}

// Prints every sensor's value - only when InitHardware(true) in main()
void PrintValues()
{
    PrintValue("playButton", playButton.IsPressed());
    PrintValue("waveButton", waveButton.IsPressed());
    PrintValue("pitchKnob", pitchKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// The oscillator from steps 1 and 2
dsp::Oscillator osc;

// NEW: a list of the four waveforms, for waveButton to step through.
// waves[0] is the first one (sine), waves[3] the last (triangle).
dsp::Oscillator::Waveform waves[4] = {
    dsp::Oscillator::WAVE_SINE,     // smooth and pure, like a whistle
    dsp::Oscillator::WAVE_TRIANGLE, // soft, a bit like a flute
    dsp::Oscillator::WAVE_SQUARE,   // hollow, like an old video game
    dsp::Oscillator::WAVE_SAW,      // bright and buzzy
};

// ---------------------------------------------------------------------
// 3. AudioCallback
// ---------------------------------------------------------------------

// The same as steps 1 and 2
void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        float sample = osc.Process();

        OUT_L[i] = sample;
        OUT_R[i] = sample;
    }
}

// ---------------------------------------------------------------------
// 4. main()
// ---------------------------------------------------------------------

int main(void)
{
    // Sets up the board and the sensors (InitInputs() above)
    InitHardware(false);

    osc.Init(hw.AudioSampleRate());

    // NEW: the envelope - how the volume changes over a note, in
    // seconds. Attack = fade in after NoteOn(). Release = fade out
    // after NoteOff(). (There's also decay and sustain - see the
    // instruments examples.)
    osc.SetAmpAttack(0.02f);
    osc.SetAmpRelease(0.3f);

    // No NoteOn() here any more - the button does it now

    hw.StartAudio(AudioCallback);

    // Which waveform from "waves" is playing: starts at 0 (sine)
    int waveNumber = 0;

    while (1)
    {
        // NEW: buttons need Update() every time round the loop, so
        // they notice when they're pressed or let go
        playButton.Update();
        waveButton.Update();

        // JustPressed() is true only at the moment the button goes
        // down - so the note starts once, not 1000 times a second
        if (playButton.JustPressed())
            osc.NoteOn();

        // JustReleased() is true only at the moment it comes up
        if (playButton.JustReleased())
            osc.NoteOff();

        // Each press of waveButton moves to the next waveform. After
        // the last one (3), go back to the first (0).
        if (waveButton.JustPressed())
        {
            waveNumber = waveNumber + 1;
            if (waveNumber > 3)
                waveNumber = 0;

            osc.SetWaveform(waves[waveNumber]);
        }

        // The pitch knob, the same as step 2
        osc.SetFreq(dsp::MapExponential(pitchKnob.Read(), 110.f, 880.f));

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
