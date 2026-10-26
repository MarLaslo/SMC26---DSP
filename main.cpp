/** SMC26 DSP course starter project.
 *
 *  A blank canvas: no synth or effects wired up yet.
 *
 *  Add your own sensors in hardware.h-style declarations above
 *  AudioCallback, create your dsp:: objects, Init() them in main(),
 *  and call their Process() methods inside AudioCallback to hear
 *  something.
 */
#include "daisysp.h"
#include "dsp/Utility.h"
#include "hardware.h"

using namespace daisy;

void AudioCallback(AudioHandle::InputBuffer  in,
                    AudioHandle::OutputBuffer out,
                    size_t                    size)
{
    for(size_t i = 0; i < size; i++)
    {
        OUT_L[i] = 0.f;
        OUT_R[i] = 0.f;
    }
}

int main(void)
{
    InitHardware();

    hw.StartAudio(AudioCallback);

    while(1)
    {
        UpdateControls();
        System::Delay(1);
    }
}
