/** SMC26 DSP course example: strummed strings (physical model).
 *
 *  Physical modeling makes sound by imitating how a real object
 *  vibrates, instead of building the sound from waves. A plucked
 *  string is the classic one (the Karplus-Strong method): a short
 *  burst of noise - the "pluck" - goes round and round a tiny loop.
 *  The length of the loop sets the pitch, and a little bit of the
 *  sound is lost on every trip round, so it fades out just like a
 *  real string. No recordings, just a loop and some noise.
 *
 *  Here four dsp::KarplusString strings are tuned to a chord, like a
 *  small guitar. Press downButton to strum from the lowest string to
 *  the highest, upButton to strum back up from the highest.
 *
 *  chordKnob      picks the chord (major chords, around A)
 *  decayKnob      short, muted "plunk" up to long ringing strings
 *  brightnessKnob dull, soft pluck up to bright, metallic pluck
 *  materialKnob   what the string is made of: middle = plain string,
 *                 down = buzzy, sitar-like, up = stiff, piano/bell-like
 *  strumKnob      how spread out the strum is: all down = all strings
 *                 at once, up = a slow, harp-like strum
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    downButton - seed::D9     chordKnob      - seed::A0
 *    upButton   - seed::D8     decayKnob      - seed::A1
 *                              brightnessKnob - seed::A2
 *                              materialKnob   - seed::A3
 *                              strumKnob      - seed::A4
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

Button downButton;
Button upButton;

Potentiometer chordKnob;
Potentiometer decayKnob;
Potentiometer brightnessKnob;
Potentiometer materialKnob;
Potentiometer strumKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    downButton.Init(seed::D9);
    upButton.Init(seed::D8);

    chordKnob.Init(hw, seed::A0);
    decayKnob.Init(hw, seed::A1);
    brightnessKnob.Init(hw, seed::A2);
    materialKnob.Init(hw, seed::A3);
    strumKnob.Init(hw, seed::A4);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    PrintValue("down", downButton.IsPressed());
    PrintValue("up", upButton.IsPressed());
    PrintValue("chord", chordKnob.Read());
    PrintValue("decay", decayKnob.Read());
    PrintValue("brightness", brightnessKnob.Read());
    PrintValue("material", materialKnob.Read());
    PrintValue("strum", strumKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// Four strings. An array holds several objects of the same kind:
// strings[0] is the lowest string, strings[3] the highest.
constexpr int kNumStrings = 4;
dsp::KarplusString strings[kNumStrings];

// How many semitones above the chord's root note each string is
// tuned: root, fifth, octave, and the third an octave up. Spread out
// like this it sounds like a real guitar chord.
float stringSemitones[kNumStrings] = {0.f, 7.f, 12.f, 16.f};

// Catches the loudest moments - all four strings at once - and turns
// them down smoothly, so the output never distorts
dsp::Limiter limiter;

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
        // Add all four strings together
        float sample = 0.f;
        for (int s = 0; s < kNumStrings; s++)
            sample += strings[s].Process();

        // Four strings added up get loud, so turn the mix down by
        // 12 dB, then the limiter catches anything left over
        sample = limiter.Process(dsp::ApplyGain(sample, -12.f));

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

    // Every string needs to know the sample rate
    for (int s = 0; s < kNumStrings; s++)
        strings[s].Init(hw.AudioSampleRate());
    limiter.Init();

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // The chord chordKnob is centered on
    float rootNote = dsp::NoteToFreq("A2");

    // For strumming: a strum plucks the strings one at a time, with a
    // short wait between them. strumStep counts how many strings this
    // strum has plucked so far - once it reaches kNumStrings, the
    // strum is finished. It starts there, so nothing plays at power-on.
    int      strumStep     = kNumStrings;
    bool     strumDown     = true; // true = low to high, false = high to low
    uint32_t lastPluckTime = 0;    // when the last string was plucked, in ms

    // Runs forever, alongside the sound: tunes the strings, passes
    // the knob values on to them, and plays the strums
    while (1)
    {
        // Read the buttons first, so JustPressed() below sees the
        // latest press
        downButton.Update();
        upButton.Update();

        // Chord: the root snaps to semitones, up to 7 either way of A2
        // (E2 to E3). Then each string is tuned stringSemitones above
        // it - every semitone up multiplies the pitch by 2^(1/12).
        float root = dsp::MapSemitones(chordKnob.Read(), rootNote, 7.f);
        for (int s = 0; s < kNumStrings; s++)
            strings[s].SetFreq(root * powf(2.f, stringSemitones[s] / 12.f));

        // Decay stops at 0.9 - from 0.95 up the string never stops
        // ringing. Brightness is already 0 - 1, same as the knob.
        // Material goes from -1 (buzzy) through 0 (plain) to 1 (stiff).
        float decay      = dsp::MapLinear(decayKnob.Read(), 0.2f, 0.9f);
        float brightness = brightnessKnob.Read();
        float material   = dsp::MapLinear(materialKnob.Read(), -1.f, 1.f);
        for (int s = 0; s < kNumStrings; s++)
        {
            strings[s].SetDamping(decay);
            strings[s].SetBrightness(brightness);
            strings[s].SetNonLinearity(material);
        }

        // A button press starts a new strum: back to the first
        // string, in that button's direction
        if (downButton.JustPressed())
        {
            strumStep = 0;
            strumDown = true;
        }
        if (upButton.JustPressed())
        {
            strumStep = 0;
            strumDown = false;
        }

        // Wait between strings: 0 ms (all at once) up to 80 ms
        uint32_t strumGapMs = dsp::MapLinear(strumKnob.Read(), 0.f, 80.f);

        // While a strum is going, pluck the next string once enough
        // time has passed since the last one. System::GetNow() is the
        // time in milliseconds since the board started.
        uint32_t now = System::GetNow();
        if (strumStep < kNumStrings && now - lastPluckTime >= strumGapMs)
        {
            // Down: strings 0, 1, 2, 3. Up: strings 3, 2, 1, 0.
            int s;
            if (strumDown)
                s = strumStep;
            else
                s = kNumStrings - 1 - strumStep;

            strings[s].Pluck();
            strumStep++;
            lastPluckTime = now;
        }

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
