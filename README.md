<h1 align="center">SMC26 DSP</h1>

<p align="center">
  Build sensor-driven music instruments on the <a href="https://electro-smith.com/products/daisy-seed">Daisy Seed</a>, no coding experience needed.<br>
  Everything you write goes in one file: <code>main.cpp</code>.
</p>

---

## What's in here

| Folder / file | What it is |
| --- | --- |
| [`main.cpp`](main.cpp) | **Your instrument.** The only file you edit. |
| [`examples/`](examples/) | Finished instruments to copy into `main.cpp` and learn from |
| [`dsp/`](dsp/) | Ready-made sound building blocks: synths, effects, filters, envelopes, sequencer |
| [`sensors/`](sensors/) | One small class per sensor: knobs, buttons, flex, touch, … |
| [`hardware.h`](hardware.h) | Starts the board, the sensors and the USB printing |
| `libs/libDaisy`, `libs/DaisySP` | The Daisy hardware and DSP libraries that everything above is built on (included, no submodules) |

You don't need to open `dsp/`, `sensors/`, `hardware.h` or `libs/` to build an instrument. They are there if you're curious.

## Getting started

### 1. Install the toolchain

Follow the Daisy guide for your OS: [daisy.audio/tutorials/cpp-dev-env](https://daisy.audio/tutorials/cpp-dev-env/).
You need `arm-none-eabi-gcc`, `make` and `dfu-util`. VS Code is recommended.

### 2. Get the code and build the libraries (once)

```sh
git clone https://github.com/MarLaslo/SMC26---DSP.git
cd SMC26---DSP
make -C libs/libDaisy
make -C libs/DaisySP
```

This takes a minute or two and only has to be done once. In VS Code you can run the task **build_all** instead.

### 3. Build and flash

Connect the Daisy over USB and put it in bootloader mode: hold **BOOT**, tap **RESET**, release **BOOT**. Then:

```sh
make
make program-dfu
```

In VS Code you can instead run the task **build_and_program_dfu**.
No terminal? Flash `build/main.bin` with the [Daisy Web Programmer](https://electro-smith.github.io/Programmer/).

### 4. See the sensor values (optional)

In `main()`, change `InitHardware(false)` to `InitHardware(true)`, build and flash again.
The board now prints every sensor's value over USB about 10 times a second.
Open the VS Code Serial Monitor, the Arduino Serial Monitor, or:

```sh
screen /dev/tty.usbmodem* 115200
```

## How main.cpp is structured

Every `main.cpp` (and every example) has the same four parts, top to bottom:

| Part | What goes there |
| --- | --- |
| **1. Sensors** | Declare your sensors, give each one a pin in `InitInputs()`, and list them in `PrintValues()` |
| **2. Sound** | Declare your `dsp::` objects: oscillators, synths, effects, … |
| **3. AudioCallback** | Makes the sound, one sample at a time. Keep it fast: no sensor reads, no printing, no delays |
| **4. main()** | Runs once at power-on: sets everything up, starts the audio, then loops forever reading the sensors |

The idea: **read sensors in the `while` loop in `main()`, pass their values to your `dsp::` objects, and let `AudioCallback` turn those into sound.**

To start from a blank file, copy [`examples/main_template.cpp`](examples/main_template.cpp) into `main.cpp`.

### Example: a knob that controls the pitch of a tone

This is [`examples/beginner/02_one_knob.cpp`](examples/beginner/02_one_knob.cpp), without the comments:

```cpp
#include "daisysp.h"
#include "dsp/Utility.h"
#include "dsp/Synthesis/Synthesis.h"
#include "dsp/Effects/Effects.h"
#include "dsp/Filters/Filters.h"
#include "dsp/Other/Other.h"
#include "hardware.h"

// 1. Sensors
Potentiometer pitchKnob;

void InitInputs()
{
    pitchKnob.Init(hw, seed::A0);
}

void PrintValues()
{
    PrintValue("pitchKnob", pitchKnob.Read());
    PrintLineEnd();
}

// 2. Sound
dsp::Oscillator osc;

// 3. AudioCallback
void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        float sample = osc.Process();
        OUT_L[i] = sample;
        OUT_R[i] = sample;
    }
}

// 4. main()
int main(void)
{
    InitHardware(false);

    osc.Init(hw.AudioSampleRate());
    osc.SetWaveform(dsp::Oscillator::WAVE_SINE);
    osc.NoteOn();

    hw.StartAudio(AudioCallback);

    while (1)
    {
        float pitch = dsp::MapExponential(pitchKnob.Read(), 110.f, 880.f);
        osc.SetFreq(pitch);

        if (printValues && TimeToPrint())
            PrintValues();

        System::Delay(1);
    }
}
```

> Set up your `dsp::` objects **before** `hw.StartAudio()`: from that line on, the board starts calling `AudioCallback`.

## Examples

To try one, copy the whole file into `main.cpp`, then build and flash. Each example lists its wiring at the top of the file.
Save your own `main.cpp` somewhere first if you want to keep it.

**Start here, in order:**

| Example | What you learn |
| --- | --- |
| [`beginner/01_hello_sound.cpp`](examples/beginner/01_hello_sound.cpp) | Make a tone |
| [`beginner/02_one_knob.cpp`](examples/beginner/02_one_knob.cpp) | Change the tone with a knob |
| [`beginner/03_one_button.cpp`](examples/beginner/03_one_button.cpp) | Play notes with buttons |

**Building blocks:**

| Example | What it shows |
| --- | --- |
| [`main_template.cpp`](examples/main_template.cpp) | The empty starting point |
| [`sensor_printing.cpp`](examples/sensor_printing.cpp) | Printing sensor values to the computer |
| [`effects_chain.cpp`](examples/effects_chain.cpp) | Running a sound through several effects |
| [`voice_manager.cpp`](examples/voice_manager.cpp) | Playing several notes at once |

**Instruments** ([`examples/instruments/`](examples/instruments/)):
`additive`, `drum_sequence`, `finger_drums`, `fm_filter_envelope`, `fm_simple`, `granular_drone`,
`modal_sequence`, `strummed_strings`, `subtractive_wobble`, `wind_ocean`.

## Sound building blocks (`dsp::`)

All of these are included at the top of `main.cpp` already. Each one is a small class with the same shape:

```cpp
dsp::Oscillator osc;              // declare it (part 2)
osc.Init(hw.AudioSampleRate());   // set it up once, in main()
osc.SetFreq(440.f);               // change settings, in the while loop
float sample = osc.Process();     // get the next sample, in AudioCallback
```

Effects and filters take the sound in: `sample = delay.Process(sample);`.
Open a class's header file in `dsp/` to see all its settings. Every header starts with a short explanation and a usage example.

### Synthesis: [`dsp/Synthesis/`](dsp/Synthesis/)

Start and stop notes with `NoteOn()` / `NoteOff()`.

| Class | Sound |
| --- | --- |
| `Oscillator` | A single waveform with a volume envelope. The simplest voice: start here |
| `Subtractive` | A bright oscillator carved down by a resonant filter |
| `Additive` | A stack of sine waves added together |
| `FM`, `FMSimple` | Frequency modulation: bells, electric pianos, metallic tones |
| `KarplusString`, `StringVoice` | Physical model of a plucked string |
| `ModalVoice` | Physical model of a struck object: bells, bars, bowls |
| `Granular` | Clouds of tiny sound grains, for drones and textures |
| `SynthBassDrum`, `SynthSnareDrum`, `HiHat` | Drum sounds |
| `Noise`, `ColoredNoise`, `SteppedNoise` | Noise: for wind, percussion, random modulation |

### Effects: [`dsp/Effects/`](dsp/Effects/)

`Overdrive`, `WaveFolder`, `SampleRateReducer` (lo-fi), `Tremolo`, `Delay`, `Chorus`, `Flanger`, `Phaser`, `PitchShifter`.

### Filters: [`dsp/Filters/`](dsp/Filters/)

| Class | Use |
| --- | --- |
| `StateVariableFilter` | All-round filter: lowpass, highpass, bandpass, notch, peak |
| `LadderFilter` | Classic Moog-style resonant filter |
| `OnePoleFilter` | Gentle lowpass/highpass, also good for smoothing a jumpy sensor |
| `AllPassFilter` | Bandpass or notch around one frequency |

### Envelopes, modulation and control: [`dsp/Other/`](dsp/Other/)

| Class | Use |
| --- | --- |
| `Adsr` | Attack-Decay-Sustain-Release envelope |
| `AdEnvelope` | Attack-Decay envelope, for hits and plucks |
| `Lfo` | A slow wobble for vibrato, tremolo or a moving filter |
| `Sequencer` | A loop of notes that plays itself at a set tempo |
| `VoiceManager` | Turns any synth into a polyphonic one (several notes at once) |
| `Limiter` | Keeps the output from getting too loud and distorting |

### Helpers: [`dsp/Utility.h`](dsp/Utility.h)

Sensors give a number from `0` to `1`. These turn it into something useful:

| Function | What it does |
| --- | --- |
| `dsp::MapLinear(value, min, max)` | `0`–`1` → `min`–`max`, evenly |
| `dsp::MapExponential(value, min, max)` | Same, but feels natural for pitch and time (like a piano keyboard) |
| `dsp::MapLog(value, min, max)` | The opposite curve |
| `dsp::MapSemitones(...)`, `dsp::SnapToScale(...)` | Snap a knob to musical notes or a scale |
| `dsp::MapPitchBend(value, base_freq, semitone_range)` | Bend a note up and down |
| `dsp::NoteToFreq("A4")` | A note name → its frequency in Hz |
| `dsp::ApplyGain(sample, db)`, `dsp::DbToLinear(db)`, `dsp::LinearToDb(x)` | Volume in decibels |
| `dsp::Clamp(value, min, max)` | Keep a value within a range |

## Sensors

Every sensor is already included through `hardware.h`. For each sensor:

1. declare it in part 1, e.g. `Potentiometer knob1;`
2. give it a pin in `InitInputs()`, e.g. `knob1.Init(hw, seed::A0);`
3. use it in the `while` loop in `main()`

**Free pins on the course board:** analog `seed::A0`–`seed::A4` (the same pins as D15–D19 on the Seed pinout), digital `seed::D9` and `seed::D8`.
The pins below are suggestions. Any pin of the right type works.

### Analog sensors

All return a value from `0.0` to `1.0` with `Read()`. Up to 12 analog sensors.

| Sensor | Class | Init | Extras |
| --- | --- | --- | --- |
| Potentiometer (knob) | `Potentiometer` | `knob.Init(hw, seed::A0)` | |
| Piezo disc | `Piezo` | `piezo.Init(hw, seed::A1)` | |
| Flex sensor | `FlexSensor` | `flex.Init(hw, seed::A2)` | |
| Stretch cord | `StretchSensor` | `stretch.Init(hw, seed::A3)` | |
| Force sensor (FSR) | `ForceSensor` | `force.Init(hw, seed::A4)` | `IsPressed(threshold)` |
| Joystick (KY-023) | `Joystick` | `joy.Init(hw, xPin, yPin, swPin)` | `X()`, `Y()` from `-1` to `1`; call `Update()` every loop |

Flex, stretch and force sensors need a voltage divider: `3V3 → sensor → pin → 10 kΩ → GND`.

### Digital

| Sensor | Class | Init | Use |
| --- | --- | --- | --- |
| Button | `Button` | `button.Init(seed::D9)` | Call `Update()` every loop |
| RGB LED (common cathode) | `ColorLed` | `led.Init(rPin, gPin, bPin)` | `Red()`, `Cyan()`, `Set(r, g, b)`, `Off()`, … |
| Ultrasonic HC-SR04 | `Ultrasonic` | `sonar.Init(seed::D0, seed::D1)` (Trig, Echo) | `Read()` → cm, or `-1` |

**Buttons need a pull-up resistor:** `3V3 → 10 kΩ → pin`, with the button between the pin and GND.
No resistor? Use the chip's built-in one: `button.Init(seed::D9, true)`.

`Button` gives you `IsPressed()`, `JustPressed()`, `JustReleased()`, `IsHeld(ms)`, `HeldTime()`, `LastPressTime()`, `Count()` and `Toggled()`. Debouncing is built in.
Call `Update()` on every button at the start of the `while` loop, before you check it.

The ultrasonic sensor measures in the background, so `Read()` never waits.
Up to 4 can be used; they take turns, so each updates about every 60 ms × number of sensors.
The HC-SR04 Echo pin outputs 5 V: use a voltage divider to bring it down to 3.3 V.

### I²C sensors

All share I²C1: **SCL = D11, SDA = D12**. `Init()` returns `false` if the sensor isn't found,
so check it. Call `Read()` in the loop, then use the values.

| Sensor | Class | Values after `Read()` |
| --- | --- | --- |
| Accelerometer (Adafruit MMA8451) | `Accelerometer` | `x`, `y`, `z` in m/s² (±2 g) |
| Barometer (Adafruit BMP585) | `Barometer` | `temperature` in °C, `pressure` in Pa |
| Capacitive touch (Adafruit MPR121) | `CapacitiveTouch` | `IsTouched(pad)`, `JustTouched(pad)`, `JustReleased(pad)`, `Raw(pad)` for pads 0–11 |

```cpp
Accelerometer accel;

void InitInputs()
{
    if (!accel.Init())
        DaisySeed::PrintLine("Accelerometer not found, check the wiring");
}
```

`CapacitiveTouch::Init(touch, release)` takes optional thresholds (default `12`, `6`). Lower is more sensitive.

## Printing values

Add one line per sensor to `PrintValues()`:

```cpp
void PrintValues()
{
    PrintValue("button1", button1.IsPressed());   // prints 1 or 0
    PrintValue("knob1", knob1.Read());            // prints e.g. 0.512
    PrintLineEnd();
}
```

Turn printing on with `InitHardware(true)`. Printing is slow: never print inside `AudioCallback`.

## Troubleshooting

| Problem | Try |
| --- | --- |
| `make: arm-none-eabi-g++: No such file` | Toolchain not installed or not on your `PATH` (step 1) |
| `cannot find -ldaisy` or `-ldaisysp` | Build the libraries first (step 2) |
| `make program-dfu` can't find the device | Enter bootloader mode again: hold BOOT, tap RESET, release BOOT |
| Flashed, but no sound | Did you call `NoteOn()`? Is `hw.StartAudio()` in `main()`? Is the volume up? |
| Nothing in the serial monitor | Is it `InitHardware(true)`? Is the serial monitor open on the right port? |
| An analog sensor always reads 0 | Is its `Init()` inside `InitInputs()`? Sensors set up anywhere else are not read |
| A button doesn't react | Is `Update()` called in the loop? Does it have a pull-up resistor (or `Init(pin, true)`)? |
| I²C sensor `Init()` returns `false` | Check SCL/SDA aren't swapped, and the sensor has power and GND |
| Clicks or dropouts in the audio | Something slow is in `AudioCallback`; move it to the `while` loop |

## Links

- [Daisy documentation](https://daisy.audio/)
- [libDaisy reference](https://daisy.audio/software/libdaisy/) · [DaisySP reference](https://daisy.audio/software/daisysp/)
- [Daisy Seed pinout](https://daisy.audio/hardware/Seed/)
- [Daisy forum](https://forum.electro-smith.com/)

## Credits

Sensor classes from [SMCWinterSchool_Daisy](https://github.com/leofogadic/SMCWinterSchool_Daisy).
Built on [libDaisy](https://github.com/electro-smith/libDaisy) (MIT) and [DaisySP](https://github.com/electro-smith/DaisySP) (MIT; the optional DaisySP-LGPL modules are LGPL) by Electro-Smith.
