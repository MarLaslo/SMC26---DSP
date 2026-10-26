/** Small helpers for mapping a 0..1 control value (e.g. AnalogIn::Value())
 *  onto a parameter's real range.
 */
#pragma once

#include <cmath>

namespace dsp
{
/** Maps `value` (expected 0..1) linearly onto [min, max]. */
inline float MapLinear(float value, float min, float max)
{
    return min + value * (max - min);
}

/** Maps `value` (expected 0..1) exponentially onto [min, max].
 *
 *  Use this instead of MapLinear for anything the ear hears
 *  logarithmically - pitch and time are the two big ones. A pot wired
 *  with MapLinear(value, 55.f, 880.f) spends most of its travel above
 *  440 Hz; MapExponential spends it evenly across octaves instead, so
 *  the knob "feels" linear to play.
 *
 *  `min` and `max` must both be > 0.
 */
inline float MapExponential(float value, float min, float max)
{
    return min * powf(max / min, value);
}

/** The inverse of MapExponential: given `value` already somewhere in
 *  [min, max] (e.g. a frequency in Hz you measured or calculated),
 *  returns the 0..1 position that would have produced it. Useful for
 *  driving an LED bar or display from a frequency/time value so it
 *  moves evenly across octaves/decades, same as the knob that set it.
 *
 *  `min`, `max` and `value` must all be > 0.
 */
inline float MapLog(float value, float min, float max)
{
    return logf(value / min) / logf(max / min);
}

/** A set of notes to snap to within each octave, for MapSemitones.
 *  SCALE_CHROMATIC uses every semitone (any note); SCALE_MAJOR and
 *  SCALE_MINOR restrict it to that scale's 7 notes, rooted at
 *  whatever `base_freq` MapSemitones is given, so turning the knob
 *  always lands on a note that sounds "in key". */
enum Scale
{
    SCALE_CHROMATIC,
    SCALE_MAJOR,
    SCALE_MINOR,
};

namespace utility_detail
{
/** Rounds `semitones` to the nearest semitone that's a member of
 *  `scale`, searching every octave independently so values well
 *  outside the first octave still snap correctly. */
inline float SnapToScale(float semitones, Scale scale)
{
    if(scale == SCALE_CHROMATIC)
        return roundf(semitones);

    static constexpr int kMajorDegrees[] = {0, 2, 4, 5, 7, 9, 11};
    static constexpr int kMinorDegrees[] = {0, 2, 3, 5, 7, 8, 10};
    const int*           degrees = scale == SCALE_MINOR ? kMinorDegrees : kMajorDegrees;

    float octave      = floorf(semitones / 12.f);
    float in_octave    = semitones - octave * 12.f;

    float best      = degrees[0];
    float best_diff = fabsf(in_octave - best);
    for(int i = 1; i < 7; i++)
    {
        float diff = fabsf(in_octave - static_cast<float>(degrees[i]));
        if(diff < best_diff)
        {
            best      = static_cast<float>(degrees[i]);
            best_diff = diff;
        }
    }
    // Also compare against the next octave's root, so values near
    // the top of this octave can snap forward instead of always
    // rounding down to the nearest degree below them.
    if(fabsf(in_octave - 12.f) < best_diff)
        best = 12.f;

    return octave * 12.f + best;
}
} // namespace utility_detail

/** Maps `value` (expected 0..1, centered at 0.5) onto a pitch offset
 *  around `base_freq`, stepping between discrete notes rather than
 *  gliding smoothly - like turning a knob across the notes of a
 *  keyboard. 0.5 returns base_freq unchanged; 0 returns it down
 *  `semitone_range` semitones; 1 returns it up `semitone_range`
 *  semitones, snapping to the nearest note in `scale` as it goes.
 *  `scale` defaults to every semitone (chromatic); pass SCALE_MAJOR
 *  or SCALE_MINOR to restrict it to that scale rooted at base_freq.
 *
 *  For a "select a note" knob, e.g.
 *  MapSemitones(noteKnob.Value(), 220.f, 12.f, SCALE_MAJOR) picks any
 *  note of the A major scale across an octave up or down from A3.
 */
inline float MapSemitones(float       value,
                           float       base_freq,
                           float       semitone_range,
                           Scale       scale = SCALE_CHROMATIC)
{
    float semitones = (value - 0.5f) * 2.f * semitone_range;
    semitones        = utility_detail::SnapToScale(semitones, scale);
    return base_freq * powf(2.f, semitones / 12.f);
}

/** Like MapSemitones, but glides smoothly instead of stepping between
 *  notes - for a detune/pitch-bend/vibrato-depth knob centered at
 *  noon, e.g. MapPitchBend(bendKnob.Value(), 440.f, 2.f) for a smooth
 *  whole-tone bend either way.
 */
inline float MapPitchBend(float value, float base_freq, float semitone_range)
{
    float semitones = (value - 0.5f) * 2.f * semitone_range;
    return base_freq * powf(2.f, semitones / 12.f);
}

/** Restricts `value` to [min, max]. Handy after a sensor reading
 *  drifts slightly outside 0..1, or after combining two mapped values.
 */
inline float Clamp(float value, float min, float max)
{
    if(value < min)
        return min;
    if(value > max)
        return max;
    return value;
}

/** Converts a gain in decibels to the plain multiplier that actually
 *  scales a sample (what you'd otherwise pass to ApplyGain or just
 *  multiply a sample by directly). 0 dB = 1 (unchanged), negative dB
 *  turns it down, positive dB turns it up - e.g. DbToLinear(-12.f)
 *  for "quieter by 12 dB" instead of guessing a multiplier like 0.2
 *  by trial and error. */
inline float DbToLinear(float db)
{
    return powf(10.f, db / 20.f);
}

/** The inverse of DbToLinear: given a plain multiplier, returns the
 *  gain in decibels it corresponds to. Useful for reporting/debugging
 *  a level rather than setting one. `linear` must be > 0. */
inline float LinearToDb(float linear)
{
    return 20.f * log10f(linear);
}

/** Scales `sample` by a gain given in decibels. Shorthand for
 *  `sample * DbToLinear(db)` when you don't need the multiplier
 *  itself - e.g. ApplyGain(wet, -12.f) to turn a signal down before
 *  it hits a limiter, so the limiter has headroom to work with
 *  instead of constantly clamping. */
inline float ApplyGain(float sample, float db)
{
    return sample * DbToLinear(db);
}

} // namespace dsp
