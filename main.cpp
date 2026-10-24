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
#include "daisy_seed.h"
#include "dsp/Synthesis.h"

using namespace daisy;

DaisySeed    hw;
Switch       button;
dsp::FMSimple voice;

enum
{
    POT_FREQ = 0,
    POT_RATIO,
    POT_DEPTH,
    POT_BRIGHTNESS,
    POT_LAST
};

void AudioCallback(AudioHandle::InputBuffer  in,
                    AudioHandle::OutputBuffer out,
                    size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float sample = voice.Process();
        OUT_L[i]      = sample;
        OUT_R[i]      = sample;
    }
}

int main(void)
{
    hw.Init();

    AdcChannelConfig adc_cfg[POT_LAST];
    adc_cfg[POT_FREQ].InitSingle(seed::D15);
    adc_cfg[POT_RATIO].InitSingle(seed::D16);
    adc_cfg[POT_DEPTH].InitSingle(seed::D17);
    adc_cfg[POT_BRIGHTNESS].InitSingle(seed::D18);
    hw.adc.Init(adc_cfg, POT_LAST);
    hw.adc.Start();

    button.Init(seed::D9);

    voice.Init(hw.AudioSampleRate());

    hw.StartAudio(AudioCallback);

    while(1)
    {
        button.Debounce();

        float freq       = 80.f + hw.adc.GetFloat(POT_FREQ) * 920.f;    // 80 Hz .. 1 kHz
        float mod_ratio   = 0.5f + hw.adc.GetFloat(POT_RATIO) * 7.5f;    // 0.5 .. 8
        float mod_depth   = hw.adc.GetFloat(POT_DEPTH) * 5.f;           // 0 .. 5
        float brightness  = hw.adc.GetFloat(POT_BRIGHTNESS);            // 0 .. 1

        voice.SetFreq(freq);
        voice.SetModRatio(mod_ratio);
        voice.SetModDepth(mod_depth);
        voice.SetBrightness(brightness);

        if(button.Pressed())
            voice.NoteOn();
        else
            voice.NoteOff();

        System::Delay(1);
    }
}
