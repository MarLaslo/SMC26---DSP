/** SMC26 DSP course example: sensor printing.
 *
 *  Reads both buttons and all five knobs and prints their values
 *  over USB serial about 10 times a second - open a serial monitor
 *  to see them. Makes no sound; use it to check your wiring.
 *
 *  To try it, copy this whole file into main.cpp.
 *
 *  Wiring:
 *    button1 - seed::D9     knob1 - seed::A0     knob4 - seed::A3
 *    button2 - seed::D8     knob2 - seed::A1     knob5 - seed::A4
 *                           knob3 - seed::A2
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

Button button1;
Button button2;

Potentiometer knob1;
Potentiometer knob2;
Potentiometer knob3;
Potentiometer knob4;
Potentiometer knob5;

// Sets up every input on its pin. Called for you by InitHardware().
void InitInputs()
{
    button1.Init(seed::D9);
    button2.Init(seed::D8);

    knob1.Init(hw, seed::A0);
    knob2.Init(hw, seed::A1);
    knob3.Init(hw, seed::A2);
    knob4.Init(hw, seed::A3);
    knob5.Init(hw, seed::A4);
}

// Prints every sensor's value on one line, e.g.
//   button1: 0   button2: 1   knob1: 0.512   ...
// Buttons print 1 (pressed) or 0; knobs print 0.000 to 1.000.
void PrintValues()
{
    PrintValue("button1", button1.IsPressed());
    PrintValue("button2", button2.IsPressed());
    PrintValue("knob1", knob1.Read());
    PrintValue("knob2", knob2.Read());
    PrintValue("knob3", knob3.Read());
    PrintValue("knob4", knob4.Read());
    PrintValue("knob5", knob5.Read());
    PrintLineEnd();
}

// ---------------------------------------------------------------------
// 2. Sound
// ---------------------------------------------------------------------

// None in this example.

// ---------------------------------------------------------------------
// 3. AudioCallback
// ---------------------------------------------------------------------

// Called by the board over and over, very fast, to get the next few
// samples of sound. Here it just sends silence.
void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    // Runs once per sample - "size" samples each time
    for (size_t i = 0; i < size; i++)
    {
        // Silence (a sample is a number from -1 to 1)
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
    // Sets up the board and the sensors (InitInputs() above).
    // true = print the sensor values over USB.
    InitHardware(true);

    // From here on, the board keeps calling AudioCallback
    hw.StartAudio(AudioCallback);

    // Runs forever, alongside the sound
    while (1)
    {
        // Read the buttons, so IsPressed() sees the latest press
        button1.Update();
        button2.Update();

        // About 10 times a second, print the sensor values
        if (printValues && TimeToPrint())
            PrintValues();

        // Wait 1 millisecond before going round again
        System::Delay(1);
    }
}
