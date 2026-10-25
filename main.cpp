/** SMC26 DSP course starter project.
 *
 *  dsp::Subtractive (oscillator -> filter -> amp envelope), played
 *  live from 4 pots and a button, with dsp::Overdrive and dsp::Chorus
 *  on the output:
 *
 *      Pot 1 (D15 / A0) -> Note pitch
 *      Pot 2 (D16 / A1) -> Filter cutoff
 *      Pot 3 (D17 / A2) -> Overdrive amount
 *      Pot 4 (D18 / A3) -> Chorus amount
 *      Button           -> Note on/off (hold to sustain)
 *
 *  Filter resonance and filter envelope amount aren't wired to a pot;
 *  they're fixed at a reasonable default below.
 *
 *  Wiring:
 *      Each pot: outer legs to 3V3 and GND, middle wiper to its D pin.
 *      Button:   one leg to D9, other leg to GND.
 *                (D9 is free, and the Switch defaults to using the
 *                Seed's internal pull-up, so no extra resistor is
 *                needed.)
 */
#include "daisysp.h"
#include "dsp/Utility.h"
#include "dsp/Synthesis/Subtractive.h"
#include "dsp/Effects/Overdrive.h"
#include "dsp/Effects/Chorus.h"
#include "hardware.h"

using namespace daisy;

AnalogIn  potFreq(seed::D15, "note pitch");
AnalogIn  potCutoff(seed::D16, "filter cutoff");
AnalogIn  potDrive(seed::D17, "overdrive amount");
AnalogIn  potChorus(seed::D18, "chorus amount");
DigitalIn noteButton(seed::D9, "note button");

dsp::Subtractive voice;
dsp::Overdrive   overdrive;
dsp::Chorus      chorus;

void AudioCallback(AudioHandle::InputBuffer  in,
                    AudioHandle::OutputBuffer out,
                    size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float sample = voice.Process();
        sample       = overdrive.Process(sample);
        sample       = chorus.Process(sample);
        OUT_L[i]     = sample;
        OUT_R[i]     = sample;
    }
}

int main(void)
{
    InitHardware();
    voice.Init(hw.AudioSampleRate());
    voice.SetResonance(0.3f);
    voice.SetFilterEnvAmount(2000.f);
    overdrive.Init();
    chorus.Init(hw.AudioSampleRate());

    hw.StartAudio(AudioCallback);

    bool was_pressed = false;

    while(1)
    {
        UpdateControls();

        voice.SetFreq(dsp::MapLinear(potFreq.Value(), 80.f, 880.f));
        voice.SetCutoff(dsp::MapLinear(potCutoff.Value(), 200.f, 5000.f));

        overdrive.SetDrive(potDrive.Value());
        chorus.SetLfoDepth(potChorus.Value());

        bool pressed = noteButton.Value();
        if(pressed && !was_pressed)
            voice.NoteOn();
        else if(!pressed && was_pressed)
            voice.NoteOff();
        was_pressed = pressed;

        System::Delay(1);
    }
}
