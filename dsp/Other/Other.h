#pragma once

/** Easy DSP library: include this one header to get every "other"
 *  utility that doesn't fit under synthesis or effects - envelopes and
 *  dynamics processing. Each one is a small class with the same shape:
 *
 *      Init(sample_rate)      - call once, at startup (some don't need
 *                                a sample rate)
 *      SetXxx(...)            - set its parameters
 *      Process(...)           - call once per audio sample, returns it
 */

#include "Adsr.h"
#include "AdEnvelope.h"
#include "Limiter.h"
#include "VoiceManager.h"
