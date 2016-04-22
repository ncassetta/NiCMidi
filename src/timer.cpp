#include "../include/timer.h"


void Wait(unsigned int msecs) {
    std::this_thread::sleep_for(milliseconds(msecs));
}


MIDITimer::MIDITimer(int res) : resolution(res), tick(0), tick_param(0),
                                timer_on(false), sys_clock_base(steady_clock::now()) {}


void MIDITimer::SetResolution(int res) {
    Stop();
    resolution = res;
}


void MIDITimer::SetMIDITick(MIDITick t, void* tp) {
    Stop();
    tick = t;
    tick_param = tp;
}


bool MIDITimer::Start () {
    if(bg_thread.joinable() || tick == 0)    // Already running or callback not set
        return false;

    timer_on = true;
    bg_thread = std::thread(ThreadProc, this);
    return true;
}


void MIDITimer::Stop() {
    if (timer_on) {
        timer_on = false;
        bg_thread.join();
    }
}


    // This is the background thread procedure
void MIDITimer::ThreadProc(MIDITimer* timer) {
    timepoint current, next;
    duration tick(timer->resolution);

    while(timer->timer_on) {
        current = steady_clock::now();
                // execute the supplied function
        timer->tick(timer->GetSysTimeMs(), timer->tick_param);
                // find the next timepoint and sleep until it
        next = current + tick;
        std::this_thread::sleep_until(next);
    }
}


