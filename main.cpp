/** SMC26 DSP course starter project.
 *
 *  Test patch: an additive synth voice (a stack of sine harmonics)
 *  through the delay effect, played with a button and shaped with
 *  pots.
 *
 *  Add your own sensors in hardware.h-style declarations above
 *  AudioCallback, create your dsp:: objects, Init() them in main(),
 *  and call their Process() methods inside AudioCallback to hear
 *  something.
 */
#include "daisysp.h"
#include "dsp/Utility.h"
#include "dsp/Synthesis/Synthesis.h"
#include "dsp/Effects/Effects.h"
#include "dsp/Other/Other.h"
#include "hardware.h"

using namespace daisy;

AnalogIn  pitchKnob(seed::D15, "pitch");
AnalogIn  delayTimeKnob(seed::D16, "delay time");
AnalogIn  delayFeedbackKnob(seed::D17, "delay feedback");
AnalogIn  delayMixKnob(seed::D18, "delay mix");
DigitalIn gateButton(seed::D9, "gate");

dsp::Additive voice;
dsp::Delay    delay;
dsp::Limiter  limiter;

void AudioCallback(AudioHandle::InputBuffer  in,
                    AudioHandle::OutputBuffer out,
                    size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float dry  = voice.Process();
        float wet  = delay.Process(dry);
        float out_sample = limiter.Process(wet);
        OUT_L[i]   = out_sample;
        OUT_R[i]   = out_sample;
    }
}

int main(void)
{
    InitHardware();

    voice.Init(hw.AudioSampleRate());
    delay.Init(hw.AudioSampleRate());
    limiter.Init();

    hw.StartAudio(AudioCallback);

    while(1)
    {
        UpdateControls();

        // Pitch knob: 55 Hz (A1) - 880 Hz (A5), exponential feel.
        float pitch_norm = pitchKnob.Value();
        voice.SetFreq(55.f * powf(16.f, pitch_norm));

        if(gateButton.Value())
        {
            voice.NoteOn();
        }
        else
        {
            voice.NoteOff();
        }

        delay.SetTimeMs(10.f + delayTimeKnob.Value() * 990.f);
        delay.SetFeedback(delayFeedbackKnob.Value());
        delay.SetMix(delayMixKnob.Value());

        System::Delay(1);
    }
}
