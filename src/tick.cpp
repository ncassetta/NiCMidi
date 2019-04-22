#include "../include/tick.h"


void MIDITickComponent::Start(tMsecs dev_offs)
{
    if (!running.load()) {
        MIDITimer::Start();
        sys_time_offset = MIDITimer::GetSysTimeMs();
        dev_time_offset = dev_offs;
        running.store(true);
    }
}


void MIDITickComponent::Stop()
{
    if(running.load()) {
        running.store(false);
        MIDITimer::Stop();
    }
}