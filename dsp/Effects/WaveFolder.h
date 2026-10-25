#pragma once

#include "../Utility.h"
#include "Effects/fold.h"

namespace dsp
{
/** Wavefolder: folds the signal back on itself instead of clipping it,
 *  adding jagged, buzzy harmonics that get harsher as the amount
 *  increases.
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
        fold_.Init();
        SetAmount(0.f);
    }

    /** Amount of folding. Range: 0 (clean) - 1 (heavily folded/buzzy). */
    void SetAmount(float amount)
    {
        fold_.SetIncrement(MapLinear(amount, 1.f, 4000.f));
    }

    /** Returns the next audio sample. */
    float Process(float in) { return fold_.Process(in); }

  private:
    daisysp::Fold fold_;
};

} // namespace dsp
