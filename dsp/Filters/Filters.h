#pragma once

/** Easy DSP library: include this one header to get every filter.
 *  Each filter is a small class with the same shape:
 *
 *      Init(sample_rate)      - call once, at startup
 *      SetXxx(...)            - set its parameters
 *      Process(sample)        - call once per audio sample, returns it
 */

#include "LadderFilter.h"
#include "OnePoleFilter.h"
#include "StateVariableFilter.h"
#include "AllPassFilter.h"
