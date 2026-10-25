/** Small helpers for mapping a 0..1 control value (e.g. AnalogIn::Value())
 *  onto a parameter's real range.
 */
#pragma once

namespace dsp
{
/** Maps `value` (expected 0..1) linearly onto [min, max]. */
inline float MapLinear(float value, float min, float max)
{
    return min + value * (max - min);
}

} // namespace dsp
