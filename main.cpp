/** SMC26 DSP course starter project.
 *
 *  dsp::Granular, played live from 3 pots and a button on a bare
 *  Daisy Seed:
 *
 *      Pot 1 (D15 / A0) -> Frequency
 *      Pot 2 (D16 / A1) -> Grain size
 *      Pot 3 (D17 / A2) -> Scan speed (how fast the grains drift
 *                          through the buffer; centered = frozen)
 *      Button           -> Gate (hold to trigger the amp envelope)
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
#include "dsp/Granular.h"
#include "hardware.h"

using namespace daisy;

AnalogIn  potFreq(seed::D15, "frequency", 0.05f); // longer slew: feeds base freq
AnalogIn  potGrainSize(seed::D16, "grain size");
AnalogIn  potSpeed(seed::D17, "scan speed");
DigitalIn gateButton(seed::D9, "gate button");

dsp::Granular voice;

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
        float grain_size  = dsp::MapLinear(potGrainSize.Value(), 5.f, 200.f);
        float speed       = dsp::MapLinear(potSpeed.Value(), -2.f, 2.f);

        voice.SetFreq(freq);
        voice.SetGrainSize(grain_size);
        voice.SetSpeed(speed);

        if(gateButton.Value())
            voice.NoteOn();
        else
            voice.NoteOff();

        System::Delay(1);
    }
}
