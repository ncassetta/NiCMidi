/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2021  Nicola Cassetta
 *   https://github.com/ncassetta/NiCMidi
 *
 *   This file is part of NiCMidi.
 *
 *   NiCMidi is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   NiCMidi is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with NiCMidi.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "../include/timer.h"

#include <iostream>         // for debugging

const MIDITimer::timepoint MIDITimer::sys_clock_base = std::chrono::steady_clock::now();



unsigned int MIDITimer::resolution = MIDITimer::DEFAULT_RESOLUTION;

void* MIDITimer::tick_param = 0;
MIDITick* MIDITimer::tick_proc = 0;
std::atomic<int> MIDITimer::num_open(0);
MIDITimer::timepoint MIDITimer::current;
std::thread MIDITimer::bg_thread;


// We need this for assuring the destructor is called at exit, stopping the timer and joining the bg_thread
// Without this we have errors if exiting while the timer is open
//static MIDITimer dummy;

//MIDITimer::MIDITimer() {
    //std::cout << "MIDITimer constructor" << std::endl;
//}

//MIDITimer::~MIDITimer() {
    //std::cout << "MIDITimer destructor" << std::endl;
    //HardStop();             // this joins the bg_thread
// };



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
    tick_proc = t;
    tick_param = tp;
    if (was_open > 0) {
        Start();
        num_open = was_open;
    }
}


bool MIDITimer::Start () {
    if (tick_proc == 0)
        return false;                           // Callback not set

    num_open++;
    if (num_open == 1) {                         // Must create thread
        current = std::chrono::steady_clock::now();
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

            std:: cout << "Timer stopped by MIDITimer::Stop()" << std::endl;
        }
    }
}


void MIDITimer::HardStop() {
    if (num_open > 0) {
        num_open = 0;
        bg_thread.join();
        std:: cout << "Timer stopped by MIDITimer::HardStop()" << std::endl;
    }
}

    // This is the background thread procedure
void MIDITimer::ThreadProc() {
    duration tick(resolution);

    while(MIDITimer::num_open) {
        // execute the supplied function
        MIDITimer::tick_proc(MIDITimer::GetSysTimeMs(), MIDITimer::tick_param);
        // find the next timepoint and sleep until it
        current += tick;
        std::this_thread::sleep_until(current);
    }
}


