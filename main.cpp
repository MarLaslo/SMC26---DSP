/** SMC26 DSP course starter project.
 *
 *  dsp::Additive, played live from 4 pots and a button on a bare
 *  Daisy Seed:
 *
 *      Pot 1 (D15 / A0) -> Frequency
 *      Pot 2 (D16 / A1) -> Number of harmonics
 *      Pot 3 (D17 / A2) -> Rolloff (brightness)
 *      Pot 4 (D18 / A3) -> 2nd partial's ratio (inharmonic detune)
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

AnalogIn  potFreq(seed::D15, "frequency", 0.05f); // longer slew: feeds base freq
AnalogIn  potNumHarmonics(seed::D16, "num harmonics");
AnalogIn  potRolloff(seed::D17, "rolloff");
AnalogIn  potHarmonic2Ratio(seed::D18, "2nd partial ratio");
DigitalIn button(seed::D9, "note button");

dsp::Additive voice;

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
    voice.SetAmpAttack(0.01f);
    voice.SetAmpRelease(0.3f);

    hw.StartAudio(AudioCallback);

    while(1)
    {
        UpdateControls();

        float freq          = dsp::MapLinear(potFreq.Value(), 80.f, 1000.f);
        int   num_harmonics = static_cast<int>(
            dsp::MapLinear(potNumHarmonics.Value(), 1.f, dsp::Additive::kMaxHarmonics));
        float rolloff          = potRolloff.Value();
        float harmonic_2_ratio = dsp::MapLinear(potHarmonic2Ratio.Value(), 1.5f, 3.5f);

        voice.SetFreq(freq);
        voice.SetNumHarmonics(num_harmonics);
        voice.SetRolloff(rolloff);
        voice.SetHarmonicRatio(1, harmonic_2_ratio);

        if(button.Value())
            voice.NoteOn();
        else
            voice.NoteOff();

        System::Delay(1);
    }
}
