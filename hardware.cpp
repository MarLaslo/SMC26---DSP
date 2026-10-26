#include "hardware.h"

using namespace daisy;

DaisySeed hw;

AnalogIn* AnalogIn::instances_[kMaxAnalogInputs];
size_t    AnalogIn::count_ = 0;

AnalogIn::AnalogIn(Pin pin, const char* name, float slew_seconds) :
    pin_(pin), name_(name), index_(count_), slew_seconds_(slew_seconds)
{
    instances_[count_++] = this;
}

size_t AnalogIn::Count()
{
    return count_;
}

AnalogIn* AnalogIn::At(size_t index)
{
    return instances_[index];
}

DigitalIn* DigitalIn::instances_[kMaxDigitalInputs];
size_t     DigitalIn::count_ = 0;

DigitalIn::DigitalIn(Pin pin, const char* name) : pin_(pin), name_(name)
{
    instances_[count_++] = this;
}

bool DigitalIn::Value() const
{
    return switch_.Pressed();
}

size_t DigitalIn::Count()
{
    return count_;
}

DigitalIn* DigitalIn::At(size_t index)
{
    return instances_[index];
}

void InitHardware()
{
    // Flush denormal floats to zero. Without this, a value decaying
    // toward (but never quite reaching) zero - a delay repeat trailing
    // off, an envelope's release tail - becomes a "denormal" number
    // that this FPU takes dramatically longer to do arithmetic on,
    // long enough to blow the audio callback's time budget and cause
    // an audible click/pop with no knob involved at all.
    __set_FPSCR(__get_FPSCR() | (1 << 24) | (1 << 25));

    hw.Init();
    // hw.StartLog();

    AdcChannelConfig adc_cfg[kMaxAnalogInputs];
    for(size_t i = 0; i < AnalogIn::Count(); i++)
        adc_cfg[i].InitSingle(AnalogIn::At(i)->pin_);
    hw.adc.Init(adc_cfg, AnalogIn::Count());
    hw.adc.Start();

    // UpdateControls() is called once per main-loop iteration, which
    // runs at roughly 1kHz (see System::Delay(1) in main()).
    for(size_t i = 0; i < AnalogIn::Count(); i++)
        AnalogIn::At(i)->AnalogControl::Init(
            hw.adc.GetPtr(i), 1000.f, false, false, AnalogIn::At(i)->slew_seconds_);

    for(size_t i = 0; i < DigitalIn::Count(); i++)
        DigitalIn::At(i)->switch_.Init(DigitalIn::At(i)->pin_);
}

void UpdateControls()
{
    for(size_t i = 0; i < AnalogIn::Count(); i++)
        AnalogIn::At(i)->Process();

    for(size_t i = 0; i < DigitalIn::Count(); i++)
        DigitalIn::At(i)->switch_.Debounce();
}

void PrintInputs()
{
    static int counter = 0;
    if(++counter < 50) // print roughly every 50 ms
        return;
    counter = 0;

    for(size_t i = 0; i < AnalogIn::Count(); i++)
        hw.PrintLine("%s: %d",
                     AnalogIn::At(i)->Name(),
                     (int)(AnalogIn::At(i)->Value() * 1000));
    for(size_t i = 0; i < DigitalIn::Count(); i++)
        hw.PrintLine("%s: %d", DigitalIn::At(i)->Name(), DigitalIn::At(i)->Value());
}
