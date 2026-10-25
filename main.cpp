/** SMC26 DSP course starter project.
 *
 *  dsp::Subtractive, played live from 3 pots and a button on a bare
 *  Daisy Seed:
 *
 *      Pot 1 (D15 / A0) -> Frequency
 *      Pot 2 (D16 / A1) -> Filter cutoff
 *      Pot 3 (D17 / A2) -> Filter resonance
 *      Pot 4 (D18 / A3) -> Filter envelope amount (how far the cutoff
 *                          sweeps open on each note's attack)
 *      Button           -> Gate (hold to trigger the amp + filter envelopes)
 *
 *  Wiring:
 *      Each pot (including D18): outer legs to 3V3 and GND, middle
 *                wiper to its D pin.
 *      Button:   one leg to D9, other leg to GND.
 *                (D9 is free, and the Switch defaults to using the
 *                Seed's internal pull-up, so no extra resistor is
 *                needed.)
 */
#include "daisysp.h"
#include "dsp/Mapping.h"
#include "dsp/Subtractive.h"
#include "hardware.h"

using namespace daisy;

AnalogIn  potFreq(seed::D15, "frequency", 0.05f); // longer slew: feeds base freq
AnalogIn  potCutoff(seed::D16, "filter cutoff");
AnalogIn  potResonance(seed::D17, "filter resonance");
AnalogIn  potFilterEnvAmount(seed::D18, "filter envelope amount");
DigitalIn gateButton(seed::D9, "gate button");

dsp::Subtractive voice;

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

        float freq            = dsp::MapLinear(potFreq.Value(), 80.f, 1000.f);
        float cutoff          = dsp::MapLinear(potCutoff.Value(), 100.f, 5000.f);
        float resonance       = dsp::MapLinear(potResonance.Value(), 0.f, 1.f);
        float filter_env_amt = dsp::MapLinear(potFilterEnvAmount.Value(), 0.f, 4000.f);

        voice.SetFreq(freq);
        voice.SetCutoff(cutoff);
        voice.SetResonance(resonance);
        voice.SetFilterEnvAmount(filter_env_amt);

        if(gateButton.Value())
            voice.NoteOn();
        else
            voice.NoteOff();

        System::Delay(1);
    }
}
