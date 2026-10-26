/** SMC26 DSP course starter project.
 *
 *  Demo: two buttons, each playing its own note, sharing a small pool
 *  of voices via dsp::VoiceManager. Pressing both at once plays both
 *  notes together - that's the "polyphony" VoiceManager buys you over
 *  wiring a single voice straight to a single button.
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
#include "dsp/Filters/Filters.h"
#include "dsp/Other/Other.h"
#include "hardware.h"

using namespace daisy;

DigitalIn note1Button(seed::D9, "note 1 button");
DigitalIn note2Button(seed::D8, "note 2 button");

constexpr float kNote1Freq = 220.f; // A3
constexpr float kNote2Freq = 330.f; // E4

dsp::VoiceManager<dsp::Subtractive, 2> voices;

void AudioCallback(AudioHandle::InputBuffer  in,
                    AudioHandle::OutputBuffer out,
                    size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float sample = voices.Process();
        OUT_L[i]     = sample;
        OUT_R[i]     = sample;
    }
}

int main(void)
{
    InitHardware();
    voices.Init(hw.AudioSampleRate());

    hw.StartAudio(AudioCallback);

    bool note1Was = false;
    bool note2Was = false;

    while(1)
    {
        UpdateControls();

        bool note1Is = note1Button.Value();
        if(note1Is && !note1Was)
            voices.NoteOn(kNote1Freq);
        else if(!note1Is && note1Was)
            voices.NoteOff(kNote1Freq);
        note1Was = note1Is;

        bool note2Is = note2Button.Value();
        if(note2Is && !note2Was)
            voices.NoteOn(kNote2Freq);
        else if(!note2Is && note2Was)
            voices.NoteOff(kNote2Freq);
        note2Was = note2Is;

        System::Delay(1);
    }
}
