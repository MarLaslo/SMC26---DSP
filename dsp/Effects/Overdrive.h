#pragma once

#include "Effects/overdrive.h"

namespace dsp
{
/** Overdrive / soft distortion.
 *  Usage:
 *      Overdrive drive;
 *      drive.Init();
 *      drive.SetDrive(0.5f);
 *      float sample = drive.Process(input);
 */
class Overdrive
{
  public:
    void Init() { drive_.Init(); }

    /** Amount of drive/distortion. Range: 0 (clean) - 1 (heavily driven). */
    void SetDrive(float drive) { drive_.SetDrive(drive); }

    /** Returns the next audio sample. */
    float Process(float in) { return drive_.Process(in); }

  private:
    daisysp::Overdrive drive_;
};

} // namespace dsp
