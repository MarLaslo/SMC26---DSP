/** SMC26 DSP course starter project.
 *
 *  This is the only file you need to edit. It has four parts, top to
 *  bottom:
 *    1. Sensors     - declare your buttons/knobs/... and set their pins
 *    2. Sound       - declare your dsp:: objects (oscillators, effects, ...)
 *    3. AudioCallback - makes the sound, one sample at a time
 *    4. main()      - runs once at power-on, then loops forever
 *
 *  Free pins on the board:
 *    Analog  (knobs, flex, piezo, ...): seed::A0 A1 A2 A3 A4
 *    Digital (buttons):                 seed::D9 D8
 *  (A0-A4 are the same pins as D15-D19 on the Daisy Seed pinout.)
 *
 *  For finished instruments to copy from, see examples/.
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

// Declare your sensors here (see sensors/sensors.h for every kind),
// e.g.
//   Button button1;
//   Potentiometer knob1;

// Sets up every input on its pin. Called for you by InitHardware(),
// so you only fill it in - add one Init() line per sensor, e.g.
void InitInputs()
{
    // button1.Init(seed::D9);
    // knob1.Init(hw, seed::A0);
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add. Only runs when printing is on (see main() below).
void PrintValues()
{
    // PrintValue("button1", button1.IsPressed());
    // PrintValue("knob1", knob1.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// Declare your dsp:: objects here, e.g.
//   dsp::Oscillator osc;

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
        // Work out this sample (a number from -1 to 1) here,
        // e.g. float sample = osc.Process();
        float sample = 0.f;

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
    // Sets up the board and your sensors (InitInputs() above).
    // Change false to true to print the sensor values over USB -
    // open a serial monitor to see them.
    InitHardware(false);

    // Init() your dsp:: objects here, before audio starts,
    // e.g. osc.Init(hw.AudioSampleRate());
    //      osc.NoteOn(); // start the note so you can hear it

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // Runs forever, alongside the sound. Read your sensors here and
    // pass their values on to your dsp:: objects,
    // e.g. osc.SetFreq(dsp::MapLinear(knob1.Read(), 100.f, 1000.f));
    while (1)
    {
        // Call Update() on each button first, so JustPressed() and
        // IsPressed() see the latest press, e.g. button1.Update();

        // About 10 times a second, print the sensor values - only if
        // InitHardware() was given true
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
