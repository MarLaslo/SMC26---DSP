#pragma once

/** Easy DSP library: include this one header to get every synthesis
 *  technique. Each technique is a small class with the same shape:
 *
 *      Init(sample_rate)    - call once, at startup
 *      SetFreq(freq_hz)     - set the pitch
 *      NoteOn() / NoteOff() - start and stop a note
 *      Process()            - call once per audio sample, returns it
 *
 *  The noise sources in Noise.h are the exception: they have no notes
 *  and just play all the time (and plain Noise needs no sample rate,
 *  just Init()).
 */

#include "Oscillator.h"
#include "Subtractive.h"
#include "FM.h"
#include "FMSimple.h"
#include "PhysicalModels/KarplusString.h"
#include "PhysicalModels/ModalVoice.h"
#include "PhysicalModels/StringVoice.h"
#include "Granular.h"
#include "Additive.h"
#include "Drums.h"
#include "Noise.h"
