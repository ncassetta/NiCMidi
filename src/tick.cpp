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


#include "../include/tick.h"
#include "../include/manager.h"


MIDITickComponent::~MIDITickComponent() {
    Stop();
    MIDIManager::RemoveMIDITick(this);
}


void MIDITickComponent::SetDevOffset(tMsecs dev_offs) {
    proc_lock.lock();
    dev_time_offset = dev_offs;
    proc_lock.unlock();

}

void MIDITickComponent::Start() {
    if (!running.load()) {
        MIDITimer::Start();
        sys_time_offset = MIDITimer::GetSysTimeMs();
        running.store(true);
    }
}


void MIDITickComponent::Stop() {
    if(running.load()) {
        running.store(false);
        MIDITimer::Stop();
    }
}
