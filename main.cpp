/** SMC26 DSP course starter project.
 *
 *  Pick a synthesis technique below, play a note with it, and build
 *  from there. Every technique in dsp/Synthesis.h shares the same
 *  three functions: Init(), SetFreq(), and Process().
 */
#include "daisy_seed.h"
#include "dsp/Synthesis.h"

using namespace daisy;

DaisySeed hw;

/** Change this to dsp::FM, dsp::Wavetable, or dsp::PhysicalModel to
 *  try a different technique. */
dsp::Subtractive voice;

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

    voice.Init(hw.AudioSampleRate());
    voice.SetFreq(220.f);

    hw.StartAudio(AudioCallback);

    while(1) {}
}
