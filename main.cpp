/** SMC26 DSP course starter project.
 *
 *  Demo: an 8-step drum sequencer, playing kick/snare/hat on its own.
 *  tempoKnob speeds/slows it; playStopButton stops/resumes it;
 *  resetButton jumps it back to the first step.
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

DigitalIn playStopButton(seed::D9, "play/stop button");
DigitalIn resetButton(seed::D8, "reset button");

AnalogIn structureKnob(seed::D15, "kick decay");
AnalogIn brightnessKnob(seed::D16, "hat brightness");
AnalogIn dampingKnob(seed::D17, "snare snappy");
AnalogIn tempoKnob(seed::D18, "tempo");

dsp::SynthBassDrum  kick;
dsp::SynthSnareDrum snare;
dsp::HiHat          hat;

// Which drum each step hits - Sequencer just hands back whatever
// number you gave it via SetStep, so here we use it as a drum ID
// instead of a frequency. See dsp/Other/Sequencer.h.
constexpr float kKick  = 0.f;
constexpr float kSnare = 1.f;
constexpr float kHat   = 2.f;

// Plays itself: an 8-step pattern - Kick, Hat, Hat, Hat, Snare, Hat,
// Hat, Hat - looping at a tempo set by tempoKnob. Edit kPattern below
// (and sequencer.SetNumSteps if you change how many hits are in it)
// to make your own.
dsp::Sequencer sequencer;

void AudioCallback(AudioHandle::InputBuffer  in,
                    AudioHandle::OutputBuffer out,
                    size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        if(sequencer.Process() && sequencer.CurrentGate())
        {
            float drum = sequencer.CurrentFreq();
            if(drum == kKick)
                kick.Hit();
            else if(drum == kSnare)
                snare.Hit();
            else
                hat.Hit();
        }

        float sample = kick.Process() + snare.Process() + hat.Process();
        OUT_L[i]      = sample;
        OUT_R[i]      = sample;
    }
}

int main(void)
{
    InitHardware();
    kick.Init(hw.AudioSampleRate());
    snare.Init(hw.AudioSampleRate());
    hat.Init(hw.AudioSampleRate());

    sequencer.Init(hw.AudioSampleRate());
    sequencer.SetTempo(120.f);
    static const float kPattern[] = {kKick, kHat, kHat, kHat, kSnare, kHat, kHat, kHat};
    for(int i = 0; i < 8; i++)
        sequencer.SetStep(i, kPattern[i]);
    sequencer.SetNumSteps(8);

    hw.StartAudio(AudioCallback);

    bool playStopWas = false;
    bool resetWas    = false;

    while(1)
    {
        UpdateControls();

        kick.SetDecay(structureKnob.Value());
        hat.SetTone(brightnessKnob.Value());
        snare.SetSnappy(dampingKnob.Value());

        // 60 - 240 BPM; see dsp/Utility.h's MapExponential for why time
        // knobs want the exponential mapping rather than MapLinear.
        sequencer.SetTempo(dsp::MapExponential(tempoKnob.Value(), 60.f, 240.f));

        bool playStopIs = playStopButton.Value();
        if(playStopIs && !playStopWas)
        {
            if(sequencer.IsRunning())
                sequencer.Stop();
            else
                sequencer.Start();
        }
        playStopWas = playStopIs;

        bool resetIs = resetButton.Value();
        if(resetIs && !resetWas)
            sequencer.Reset();
        resetWas = resetIs;

        System::Delay(1);
    }
}
