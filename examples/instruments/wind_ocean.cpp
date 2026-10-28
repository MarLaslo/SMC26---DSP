/** SMC26 DSP course example: wind and ocean.
 *
 *  Nature sounds made from noise. Noise is every pitch at once - a
 *  "shhhh" - and a filter picks out the part of it you want to hear.
 *  Move that filter slowly and the noise comes alive:
 *    - wind:  a narrow bandpass filter lets through one "howling"
 *             band of noise, and its pitch and volume drift around
 *             in uneven gusts
 *    - ocean: a lowpass filter, rumbly when a wave is far away and
 *             brighter and louder as it rolls in, again and again
 *  Both are the same few parts - noise, a filter and slow LFOs - just
 *  set up differently.
 *
 *  Press modeButton to switch between wind and ocean (each press
 *  flips it). Hold gustButton for a strong gust / a big wave.
 *
 *  speedKnob     how fast the gusts / waves come
 *  stormKnob     calm (gentle, steady) up to stormy (big swells)
 *  toneKnob      the pitch of the noise, low and rumbly to high and hissy
 *  howlKnob      filter resonance - turn up for a whistling, howling wind
 *  volumeKnob    overall volume
 *
 *  Sounds best in stereo headphones - each ear gets its own noise.
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    gustButton - seed::D9     speedKnob  - seed::A0
 *    modeButton - seed::D8     stormKnob  - seed::A1
 *                              toneKnob   - seed::A2
 *                              howlKnob   - seed::A3
 *                              volumeKnob - seed::A4
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

Button gustButton;
Button modeButton;

Potentiometer speedKnob;
Potentiometer stormKnob;
Potentiometer toneKnob;
Potentiometer howlKnob;
Potentiometer volumeKnob;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    gustButton.Init(seed::D9);
    modeButton.Init(seed::D8);

    speedKnob.Init(hw, seed::A0);
    stormKnob.Init(hw, seed::A1);
    toneKnob.Init(hw, seed::A2);
    howlKnob.Init(hw, seed::A3);
    volumeKnob.Init(hw, seed::A4);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    PrintValue("gust", gustButton.IsPressed());
    PrintValue("ocean", modeButton.Toggled());
    PrintValue("speed", speedKnob.Read());
    PrintValue("storm", stormKnob.Read());
    PrintValue("tone", toneKnob.Read());
    PrintValue("howl", howlKnob.Read());
    PrintValue("volume", volumeKnob.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// The raw sound: white noise
dsp::Noise noise;

// One filter for each ear. They get the same settings, but different
// noise, which makes the sound wide instead of stuck in the middle.
dsp::StateVariableFilter filterL;
dsp::StateVariableFilter filterR;

// Two slow LFOs that move the filter and volume. In wind mode they
// are mixed together: because their speeds don't match, their ups
// and downs never line up the same way twice - so the gusts feel
// random. In ocean mode only the first one is used, for steady,
// regular waves.
dsp::Lfo lfo1;
dsp::Lfo lfo2;

// Turns the gust button's sudden on/off into a smooth swell. A
// lowpass filter on a control value makes it glide instead of jump.
dsp::OnePoleFilter gustSmoother;

// Safety net on the final sound, so a big gust never distorts
dsp::Limiter limiterL;
dsp::Limiter limiterR;

// Set from the sensors in main(), used in AudioCallback
bool  oceanMode  = false; // false = wind, true = ocean
float storm      = 0.5f;  // 0 = calm, 1 = stormy
float baseTone   = 600.f; // Hz, the filter's center when calm
float gustTarget = 0.f;   // 1 while gustButton is held, else 0
float volumeDb   = -6.f;  // overall volume, in dB

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
        // Where the slow movement is right now, from -1 to 1
        float movement;
        if (oceanMode)
            movement = lfo1.Process();
        else
            movement = 0.6f * lfo1.Process() + 0.4f * lfo2.Process();

        // The gust, gliding smoothly between 0 and 1
        float gust = gustSmoother.Process(gustTarget);

        // Filter: moves up to 2 octaves either way with the movement
        // (more when stormy), plus up to 1.5 octaves higher in a gust.
        // Each octave doubles the frequency, hence powf(2, ...).
        float octaves = storm * 2.f * movement + gust * 1.5f;
        float cutoff  = dsp::Clamp(baseTone * powf(2.f, octaves), 40.f, 12000.f);
        filterL.SetFreq(cutoff);
        filterR.SetFreq(cutoff);

        // Volume swells with the movement too. "swell" is the
        // movement moved from -1..1 to 0..1. When calm the volume
        // barely moves; when stormy it drops right down between gusts.
        float swell = (movement + 1.f) / 2.f;
        float level = dsp::MapLinear(swell, 1.f - storm, 1.f) + gust;

        // Ask the noise for two different random samples, one for
        // each ear, and filter each one
        float left  = filterL.Process(noise.Process()) * level;
        float right = filterR.Process(noise.Process()) * level;

        // Overall volume, then the safety net
        left  = limiterL.Process(dsp::ApplyGain(left, volumeDb));
        right = limiterR.Process(dsp::ApplyGain(right, volumeDb));

        OUT_L[i] = left;
        OUT_R[i] = right;
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

    noise.Init();
    filterL.Init(sampleRate);
    filterR.Init(sampleRate);
    lfo1.Init(sampleRate);
    lfo2.Init(sampleRate);
    limiterL.Init();
    limiterR.Init();

    // 0.5 Hz: the gust swells in, and dies away, over about a second.
    // Lower it for slower swells.
    gustSmoother.Init(sampleRate);
    gustSmoother.SetFreq(0.5f);

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // Runs forever, alongside the sound: reads the sensors and
    // passes their values on to the sound
    while (1)
    {
        // Read the buttons first, so they see the latest press
        gustButton.Update();
        modeButton.Update();

        // Toggled() flips on every press: starts false, so wind first
        oceanMode = modeButton.Toggled();

        // The only difference between the two sounds: which part of
        // the noise the filter lets through. Bandpass = just a band
        // around the cutoff (howling wind). Lowpass = everything below
        // it (rumbling sea).
        if (oceanMode)
        {
            filterL.SetMode(dsp::StateVariableFilter::MODE_LOWPASS);
            filterR.SetMode(dsp::StateVariableFilter::MODE_LOWPASS);
        }
        else
        {
            filterL.SetMode(dsp::StateVariableFilter::MODE_BANDPASS);
            filterR.SetMode(dsp::StateVariableFilter::MODE_BANDPASS);
        }

        // 1 while the button is held, 0 when it's not - the smoother
        // in AudioCallback turns that into a gentle swell
        if (gustButton.IsPressed())
            gustTarget = 1.f;
        else
            gustTarget = 0.f;

        // Speed: from one wave every 20 seconds (0.05 Hz) up to two a
        // second. lfo2 runs at a bit over a third of lfo1's speed - an
        // uneven ratio, so the two never fall into step.
        float speed = dsp::MapExponential(speedKnob.Read(), 0.05f, 2.f);
        lfo1.SetFreq(speed);
        lfo2.SetFreq(speed * 0.37f);

        // Storm is already 0 - 1, same as the knob
        storm = stormKnob.Read();

        // Tone: MapExponential so the knob feels even across its
        // whole travel (see dsp/Utility.h)
        baseTone = dsp::MapExponential(toneKnob.Read(), 150.f, 3000.f);

        // Howl stops at 0.9 - above that the filter starts to ring
        // on its own
        float howl = dsp::MapLinear(howlKnob.Read(), 0.f, 0.9f);
        filterL.SetResonance(howl);
        filterR.SetResonance(howl);

        // Volume from -30 dB (quiet) to +6 dB (loud)
        volumeDb = dsp::MapLinear(volumeKnob.Read(), -30.f, 6.f);

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
