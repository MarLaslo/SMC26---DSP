#pragma once

/** Easy DSP library: include this one header to get every effect.
 *  Each effect is a small class with the same shape:
 *
 *      Init(sample_rate)      - call once, at startup (some don't need
 *                                a sample rate)
 *      SetXxx(...)            - set its parameters
 *      Process(sample)        - call once per audio sample, returns it
 */

#include "Overdrive.h"
#include "WaveFolder.h"
#include "SampleRateReducer.h"
#include "Tremolo.h"
#include "Chorus.h"
#include "Flanger.h"
#include "Phaser.h"
#include "PitchShifter.h"
