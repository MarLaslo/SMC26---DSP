/** SMC26 DSP course starter project.
 *
 *  dsp::StringVoice, played live from 4 pots and a button on a bare
 *  Daisy Seed:
 *
 *      Pot 1 (D15 / A0) -> Frequency
 *      Pot 2 (D16 / A1) -> Structure (curved bridge / dispersion)
 *      Pot 3 (D17 / A2) -> Brightness
 *      Pot 4 (D18 / A3) -> Damping (decay time)
 *      Button           -> Strike (press to re-strike)
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
#include "dsp/Synthesis.h"
#include "hardware.h"

using namespace daisy;

AnalogIn  potFreq(seed::D15, "frequency", 0.05f); // longer slew: feeds base freq
AnalogIn  potStructure(seed::D16, "structure");
AnalogIn  potBrightness(seed::D17, "brightness");
AnalogIn  potDamping(seed::D18, "damping");
DigitalIn button(seed::D9, "strike button");

dsp::StringVoice voice;
bool             button_was_pressed = false;

void AudioCallback(AudioHandle::InputBuffer  in,
                    AudioHandle::OutputBuffer out,
                    size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float sample = voice.Process();
        sample *= 0.5f;
        OUT_L[i]      = sample;
        OUT_R[i]      = sample;
    }
}

int main(void)
{
    InitHardware();
    voice.Init(hw.AudioSampleRate());

    hw.StartAudio(AudioCallback);

    while(1)
    {
        UpdateControls();

        float freq       = dsp::MapLinear(potFreq.Value(), 80.f, 1000.f);
        float structure  = potStructure.Value();
        float brightness = potBrightness.Value();
        float damping    = potDamping.Value();

        voice.SetFreq(freq);
        voice.SetStructure(structure);
        voice.SetBrightness(brightness);
        voice.SetDamping(damping);

        bool button_pressed = button.Value();
        if(button_pressed && !button_was_pressed)
            voice.Strike();
        button_was_pressed = button_pressed;

        System::Delay(1);
    }
}
