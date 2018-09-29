#include "../include/timer.h"

const MIDITimer::timepoint MIDITimer::sys_clock_base = std::chrono::steady_clock::now();



unsigned int MIDITimer::resolution = MIDITimer::DEFAULT_RESOLUTION;

void* MIDITimer::tick_param = 0;
MIDITick* MIDITimer::tick = 0;
std::atomic<int> MIDITimer::num_open(0);
std::thread MIDITimer::bg_thread;

/*
MIDITimer::MIDITimer(unsigned int res = DEFAULT_RESOLUTION) {
    resolution(res);
}
*/

void MIDITimer::SetResolution(unsigned int res) {
    int was_open = num_open;
    HardStop();
    resolution = res;
    if (was_open > 0) {
        Start();
        num_open = was_open;
    }
}


void MIDITimer::SetMIDITick(MIDITick t, void* tp) {
    int was_open = num_open;
    HardStop();
    tick = t;
    tick_param = tp;
    if (was_open > 0) {
        Start();
        num_open = was_open;
    }
}


bool MIDITimer::Start () {
    if (tick == 0)
        return false;                           // Callback not set

    num_open++;
    if (num_open == 1) {                         // Must create thread
        bg_thread = std::thread(ThreadProc);
        std::cout << "Timer open with " << resolution << " msecs resolution" << std::endl;
    }
    return true;
}


void MIDITimer::Stop() {
    if (num_open > 0) {
        num_open--;
        if (num_open == 0) {
            bg_thread.join();

            std:: cout << "Timer stopped" << std::endl;
        }
    }
}


void MIDITimer::HardStop() {
    if (num_open > 0) {
        num_open = 0;
        bg_thread.join();
        std:: cout << "Timer stopped" << std::endl;
    }
}

    // This is the background thread procedure
void MIDITimer::ThreadProc() {
    timepoint current, next;
    duration tick(MIDITimer::resolution);

    while(MIDITimer::num_open) {
        current = std::chrono::steady_clock::now();
                // execute the supplied function
        MIDITimer::tick(MIDITimer::GetSysTimeMs(), MIDITimer::tick_param);
                // find the next timepoint and sleep until it
        next = current + tick;
        std::this_thread::sleep_until(next);
    }
}


