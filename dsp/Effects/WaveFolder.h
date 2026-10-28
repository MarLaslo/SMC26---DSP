#pragma once

#include "../Utility.h"
#include "Effects/wavefolder.h"

namespace dsp
{
/** Wavefolder: folds the signal back on itself instead of clipping it,
 *  adding jagged, buzzy harmonics that get harsher as the amount
 *  increases. Built on top of DaisySP's Wavefolder: the signal is
 *  turned up, and every part of it that goes past -1 or 1 is folded
 *  back inside instead of being cut off. So the output never goes past
 *  -1 to 1, but a quiet input comes out louder, and needs more amount
 *  before it starts to fold.
 *  Usage:
 *      WaveFolder folder;
 *      folder.Init();
 *      folder.SetAmount(0.5f);
 *      float sample = folder.Process(input);
 */
class WaveFolder
{
  public:
    void Init()
    {
        folder_.Init();
        SetAmount(0.f);
    }

    /** Amount of folding. Range: 0 (clean) - 1 (heavily folded/buzzy).
     *  Turns the signal up 1x - 20x before folding it (exponentially,
     *  so the knob feels even for loud and quiet sounds alike). */
    void SetAmount(float amount)
    {
        folder_.SetGain(MapExponential(amount, 1.f, 20.f));
    }

    /** Returns the next audio sample. */
    float Process(float in) { return folder_.Process(in); }

  private:
    daisysp::Wavefolder folder_;
};

} // namespace dsp
