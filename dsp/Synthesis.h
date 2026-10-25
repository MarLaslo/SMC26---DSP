#pragma once

/** Easy DSP library: include this one header to get every synthesis
 *  technique. Each technique is a small class with the same shape:
 *
 *      Init(sample_rate)   - call once, at startup
 *      SetFreq(freq_hz)    - set the pitch
 *      Process()           - call once per audio sample, returns it
 *
 *  Techniques are currently placeholders (they output silence) and
 *  will be filled in as the course progresses.
 */

#include "Subtractive.h"
#include "FM.h"
#include "FMSimple.h"
#include "Wavetable.h"
#include "PhysicalModels/KarplusString.h"
#include "PhysicalModels/ModalVoice.h"
#include "PhysicalModels/StringVoice.h"
#include "Granular.h"
#include "Sampler.h"
#include "Additive.h"
