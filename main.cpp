/** SMC26 DSP course starter project.
 *
 *  Declare your sensors (see sensors/sensors.h) and dsp:: objects
 *  above AudioCallback, Init() them in main(), and call their
 *  Process() methods inside AudioCallback to hear something.
 *
 *  For finished instruments to copy from, see examples/.
 */
#include "daisy_seed.h"
#include "daisysp.h"
#include "dsp/Utility.h"
#include "dsp/Synthesis/Synthesis.h"
#include "dsp/Effects/Effects.h"
#include "dsp/Filters/Filters.h"
#include "dsp/Other/Other.h"
#include "sensors/sensors.h"

using namespace daisy;

DaisySeed hw;

Button button1;
Button button2;

Potentiometer knob1;
Potentiometer knob2;
Potentiometer knob3;
Potentiometer knob4;
Potentiometer knob5;

// Set by InitHardware() - when true, main() prints the sensor values.
bool printValues = false;

// Sets up the board and the sensors. Pass true to print the sensor
// values over USB serial (open a serial monitor to see them).
void InitHardware(bool print)
{
    // Flush denormal floats to zero. Without this, a value decaying
    // toward (but never quite reaching) zero - a delay repeat trailing
    // off, an envelope's release tail - becomes a "denormal" number
    // that this FPU takes dramatically longer to do arithmetic on,
    // long enough to blow the audio callback's time budget and cause
    // an audible click/pop with no knob involved at all.
    __set_FPSCR(__get_FPSCR() | (1 << 24) | (1 << 25));

    hw.Init();

    printValues = print;
    if (printValues)
        hw.StartLog();

    button1.Init(seed::D9);
    button2.Init(seed::D8);

    knob1.Init(hw, seed::D15);
    knob2.Init(hw, seed::D16);
    knob3.Init(hw, seed::D17);
    knob4.Init(hw, seed::D18);
    knob5.Init(hw, seed::D19);
    StartAnalogInputs(hw); // once, after every analog sensor's Init()
}

// Prints every sensor's value on one line. Add a line here for each
// sensor you add.
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

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        float sample = 0.f;
        OUT_L[i] = sample;
        OUT_R[i] = sample;
    }
}

int main(void)
{
    InitHardware(true);

    // Init() your dsp:: objects here, before audio starts.

    hw.StartAudio(AudioCallback);

    while (1)
    {
        button1.Update();
        button2.Update();

        if (printValues && TimeToPrint())
            PrintValues();

        System::Delay(1);
    }
}
