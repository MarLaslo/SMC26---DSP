/** SMC26 DSP course starter project.
 *
 *  dsp::SteppedNoise (sample-and-held noise), played live from 2
 *  pots and a button on a bare Daisy Seed:
 *
 *      Pot 1 (D15 / A0) -> Step rate (how often a new random value is held)
 *      Pot 2 (D16 / A1) -> Output level
 *      Button           -> Sync (forces a fresh random value on each press)
 *
 *  Wiring:
 *      Each pot: outer legs to 3V3 and GND, middle wiper to its D pin.
 *      Button:   one leg to D9, other leg to GND.
 *                (D9 is free, and the Switch defaults to using the
 *                Seed's internal pull-up, so no extra resistor is
 *                needed.)
 */
#include "daisysp.h"
#include "dsp/Mapping.h"
#include "dsp/Noise.h"
#include "hardware.h"

using namespace daisy;

AnalogIn  potRate(seed::D15, "step rate");
AnalogIn  potLevel(seed::D16, "level");
DigitalIn syncButton(seed::D9, "sync button");

dsp::SteppedNoise noise;
float             level = 0.5f;

void AudioCallback(AudioHandle::InputBuffer  in,
                    AudioHandle::OutputBuffer out,
                    size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float sample = noise.Process();
        sample *= level;
        OUT_L[i] = sample;
        OUT_R[i] = sample;
    }
}

int main(void)
{
    InitHardware();
    noise.Init(hw.AudioSampleRate());

    hw.StartAudio(AudioCallback);

    bool was_pressed = false;

    while(1)
    {
        UpdateControls();

        noise.SetFreq(dsp::MapLinear(potRate.Value(), 20.f, 8000.f));
        level = dsp::MapLinear(potLevel.Value(), 0.f, 1.f);

        bool pressed = syncButton.Value();
        if(pressed && !was_pressed)
            noise.Sync();
        was_pressed = pressed;

        System::Delay(1);
    }
}
