/** SMC26 DSP course starter project.
 *
 *  dsp::FMSimple, played live from 4 pots and a button on a bare
 *  Daisy Seed:
 *
 *      Pot 1 (D15 / A0) -> Frequency
 *      Pot 2 (D16 / A1) -> Modulator ratio
 *      Pot 3 (D17 / A2) -> Modulator depth
 *      Pot 4 (D18 / A3) -> Brightness
 *      Button           -> Note on/off (hold to sustain)
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

AnalogIn  potFreq(seed::D15, "frequency", 0.05f); // longer slew: feeds FM carrier freq
AnalogIn  potRatio(seed::D16, "mod ratio");
AnalogIn  potDepth(seed::D17, "mod depth");
AnalogIn  potBrightness(seed::D18, "brightness");
DigitalIn button(seed::D9, "note button");

dsp::FMSimple voice;

void AudioCallback(AudioHandle::InputBuffer  in,
                    AudioHandle::OutputBuffer out,
                    size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float sample = voice.Process();
        sample *= 0.2f;
        OUT_L[i]      = sample;
        OUT_R[i]      = sample;
    }
}

int main(void)
{
    InitHardware();
    voice.Init(hw.AudioSampleRate());
    voice.SetAttack(4.0f);
    voice.SetRelease(3.0f);

    hw.StartAudio(AudioCallback);

    while(1)
    {
        UpdateControls();

        float freq       = dsp::MapLinear(potFreq.Value(), 80.f, 1000.f);
        float mod_ratio  = dsp::MapLinear(potRatio.Value(), 0.5f, 8.f);
        float mod_depth  = dsp::MapLinear(potDepth.Value(), 0.f, 5.f);
        float brightness = potBrightness.Value();

        voice.SetFreq(freq);
        voice.SetModRatio(mod_ratio);
        voice.SetModDepth(mod_depth);
        voice.SetBrightness(brightness);

        if(button.Value())
            voice.NoteOn();
        else
            voice.NoteOff();

        System::Delay(1);
    }
}
