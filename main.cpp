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

AnalogIn structureKnob(seed::D15, "structure");
AnalogIn brightnessKnob(seed::D16, "brightness");
AnalogIn dampingKnob(seed::D17, "damping");
AnalogIn accentKnob(seed::D18, "accent");

constexpr float kNote1Freq  = 220.f; // A3
constexpr float kNote2Freq  = 330.f; // E4
constexpr int   kNumVoices  = 2;

dsp::VoiceManager<dsp::ModalVoice, kNumVoices> voices;

// A free-running LFO, here used for vibrato on each voice's pitch.
// Swap SetFreq/the depth below, or point a Process() call at any
// other parameter instead (amplitude for tremolo, cutoff for a filter
// wobble, ...) - see dsp/Other/Lfo.h.
dsp::Lfo         vibrato;
constexpr float  kVibratoDepthHz = 8.f;

void AudioCallback(AudioHandle::InputBuffer  in,
                    AudioHandle::OutputBuffer out,
                    size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float vibratoOffset = kVibratoDepthHz * vibrato.Process();
        for(int v = 0; v < kNumVoices; v++)
            voices.Voice(v).SetFreq(voices.Freq(v) + vibratoOffset);

        float sample = voices.Process();
        OUT_L[i]     = sample;
        OUT_R[i]     = sample;
    }
}

int main(void)
{
    InitHardware();
    voices.Init(hw.AudioSampleRate());
    vibrato.Init(hw.AudioSampleRate());
    vibrato.SetFreq(5.f);

    hw.StartAudio(AudioCallback);

    bool note1Was = false;
    bool note2Was = false;

    while(1)
    {
        UpdateControls();

        float structure  = structureKnob.Value();
        float brightness = brightnessKnob.Value();
        float damping    = dampingKnob.Value();
        float accent     = accentKnob.Value();

        for(int v = 0; v < kNumVoices; v++)
        {
            voices.Voice(v).SetStructure(structure);
            voices.Voice(v).SetBrightness(brightness);
            voices.Voice(v).SetDamping(damping);
            voices.Voice(v).SetAccent(accent);
        }

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
