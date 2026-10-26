/** SMC26 DSP course starter project.
 *
 *  Demo: an 8-step sequencer plays an example pattern on its own,
 *  through a small pool of voices shared via dsp::VoiceManager.
 *  accentKnob speeds/slows it; note1Button stops/resumes it;
 *  note2Button jumps it back to the first step.
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

AnalogIn structureKnob(seed::D15, "structure");
AnalogIn brightnessKnob(seed::D16, "brightness");
AnalogIn dampingKnob(seed::D17, "damping");
AnalogIn tempoKnob(seed::D18, "tempo");

constexpr int kNumVoices = 2;

dsp::VoiceManager<dsp::ModalVoice, kNumVoices> voices;

// A free-running LFO, here used for vibrato on each voice's pitch.
// Swap SetFreq/the depth below, or point a Process() call at any
// other parameter instead (amplitude for tremolo, cutoff for a filter
// wobble, ...) - see dsp/Other/Lfo.h.
dsp::Lfo        vibrato;
constexpr float kVibratoDepthHz = 8.f;

// Plays itself: an 8-step example pattern, looping at a tempo set by
// tempoKnob. Edit kPattern below (and sequencer.SetNumSteps if you
// change how many notes are in it) to make your own. See
// dsp/Other/Sequencer.h.
dsp::Sequencer sequencer;

void AudioCallback(AudioHandle::InputBuffer  in,
                    AudioHandle::OutputBuffer out,
                    size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        float vibratoOffset = kVibratoDepthHz * vibrato.Process();
        for(int v = 0; v < kNumVoices; v++)
            voices.Voice(v).SetFreq(voices.Freq(v) + vibratoOffset);

        if(sequencer.Process() && sequencer.CurrentGate())
            voices.NoteOn(sequencer.CurrentFreq());

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

    sequencer.Init(hw.AudioSampleRate());
    sequencer.SetTempo(120.f);
    // Note names instead of Hz - see dsp::NoteToFreq in dsp/Utility.h.
    static const char* kPattern[] = {"A3", "C4", "D4", "E4", "G4", "E4", "D4", "C4"};
    for(int i = 0; i < 8; i++)
        sequencer.SetStep(i, dsp::NoteToFreq(kPattern[i]));
    sequencer.SetNumSteps(8);

    hw.StartAudio(AudioCallback);

    bool playStopWas = false;
    bool resetWas    = false;

    while(1)
    {
        UpdateControls();

        float structure  = structureKnob.Value();
        float brightness = brightnessKnob.Value();
        float damping    = dampingKnob.Value();

        for(int v = 0; v < kNumVoices; v++)
        {
            voices.Voice(v).SetStructure(structure);
            voices.Voice(v).SetBrightness(brightness);
            voices.Voice(v).SetDamping(damping);
        }

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
