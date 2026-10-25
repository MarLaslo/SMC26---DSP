/** SMC26 DSP course starter project.
 *
 *  dsp::HiHat (808-style metallic noise), played live from 3 pots and
 *  a button on a bare Daisy Seed:
 *
 *      Pot 1 (D15 / A0) -> Frequency
 *      Pot 2 (D16 / A1) -> Decay
 *      Pot 3 (D17 / A2) -> Noisiness (tone vs. noise mix)
 *      Button           -> Hit (triggers the hi-hat on each press)
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
#include "dsp/Drums.h"
#include "hardware.h"

using namespace daisy;

AnalogIn  potFreq(seed::D15, "frequency");
AnalogIn  potDecay(seed::D16, "decay");
AnalogIn  potNoisiness(seed::D17, "noisiness");
DigitalIn hitButton(seed::D9, "hit button");

dsp::HiHat hat;

void AudioCallback(AudioHandle::InputBuffer  in,
                    AudioHandle::OutputBuffer out,
                    size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float sample = hat.Process();
        sample *= 0.8f;
        OUT_L[i] = sample;
        OUT_R[i] = sample;
    }
}

int main(void)
{
    InitHardware();
    hat.Init(hw.AudioSampleRate());

    hw.StartAudio(AudioCallback);

    bool was_pressed = false;

    while(1)
    {
        UpdateControls();

        hat.SetFreq(dsp::MapLinear(potFreq.Value(), 2000.f, 8000.f));
        hat.SetDecay(dsp::MapLinear(potDecay.Value(), 0.f, 1.f));
        hat.SetNoisiness(dsp::MapLinear(potNoisiness.Value(), 0.f, 1.f));

        bool pressed = hitButton.Value();
        if(pressed && !was_pressed)
            hat.Hit();
        was_pressed = pressed;

        System::Delay(1);
    }
}
