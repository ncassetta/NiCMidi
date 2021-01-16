/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2020  Nicola Cassetta
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


#include "../include/thru.h"
#include "../include/manager.h"


MIDIThru::MIDIThru() : MIDITickComponent(PR_PRE_SEQ, StaticTickProc), in_port(0), out_port(0), in_channel(-1),
                                         out_channel(-1), processor(0)
{
    std::cout << "MIDIThru constructor" << std::endl;
    //check if inand out ports exist
    if (!MIDIManager::IsValidInPortNumber(0) || !MIDIManager::IsValidOutPortNumber(0))
        throw RtMidiError("MIDIThru needs almost a MIDI in and out port in the system\n", RtMidiError::INVALID_DEVICE);
}


MIDIThru::~MIDIThru() {
    std::cout << "MIDIThru destructor" << std::endl;
    Stop();
}



void MIDIThru::Reset() {
    Stop();
    SilentOut();
    in_port = 0;
    out_port = 0;
    processor = 0;
    in_channel = -1;
    out_channel = -1;
}


void MIDIThru::SetInPort(unsigned int port) {
    if (!MIDIManager::IsValidInPortNumber(port))
        return;                                     // avoids out of range errors
    if (port == in_port)
        return;                                     // trying to assign same ports: nothing to do

    if (IsPlaying()) {
        proc_lock.lock();
        MIDIManager::GetInDriver(in_port)->ClosePort();
        SilentOut();
        MIDIManager::GetInDriver(port)->OpenPort();
        in_port = port;
        proc_lock.unlock();
    }
    else
        in_port = port;
}


void MIDIThru::SetOutPort(unsigned int port) {
    if (!MIDIManager::IsValidOutPortNumber(port))
        return;                                     // avoids out of range errors
    if (port == out_port)
        return;                                     // trying to assign same ports: nothing to do

    if (IsPlaying()) {
        proc_lock.lock();
        SilentOut();
        MIDIManager::GetOutDriver(out_port)->ClosePort();
        MIDIManager::GetOutDriver(port)->OpenPort();
        out_port = port;
        proc_lock.unlock();
    }
    else
        out_port = port;
}


void MIDIThru::SetProcessor(MIDIProcessor* proc) {
    if (IsPlaying()) {
        proc_lock.lock();
        SilentOut();
        processor = proc;
        proc_lock.unlock();
    }
    else
        processor = proc;
}


void MIDIThru::SetInChannel(char chan) {
    if (IsPlaying()) {
        proc_lock.lock();
        SilentOut();
        in_channel = chan;
        proc_lock.unlock();
    }
    else
        in_channel = chan;
}


void MIDIThru::SetOutChannel(char chan) {
    if (IsPlaying()) {
        proc_lock.lock();
        SilentOut();
        out_channel = chan;
        proc_lock.unlock();
    }
    else
        out_channel = chan;
}


void MIDIThru::Start() {
    if (!IsPlaying()) {
        MIDIManager::GetInDriver(in_port)->OpenPort();
        MIDIManager::GetOutDriver(out_port)->OpenPort();
        MIDITickComponent::Start();
    }
}


void MIDIThru::Stop() {
    if (IsPlaying()) {
        MIDITickComponent::Stop();
        MIDIManager::GetInDriver(in_port)->ClosePort();
        SilentOut();
        MIDIManager::GetOutDriver(out_port)->ClosePort();
    }
}


void MIDIThru::SilentOut() {
    if (out_channel != -1)
        MIDIManager::GetOutDriver(out_port)->AllNotesOff(out_channel);
    else
        MIDIManager::GetOutDriver(out_port)->AllNotesOff();
}


void MIDIThru::StaticTickProc(tMsecs sys_time, void* pt) {
    MIDIThru* thru_pt = static_cast<MIDIThru *>(pt);
    thru_pt->TickProc(sys_time);
}


void MIDIThru::TickProc(tMsecs sys_time_)
{
    proc_lock.lock();
/*
    static unsigned int times = 0;

    if (times % 1000 == 0)
        std::cout << "MIDIThru::TickProc() called " << times * 1000 << " times\n";
    times++;
*/
    MIDIRawMessage rmsg;
    MIDITimedMessage msg;
    MIDIInDriver* in_driver = MIDIManager::GetInDriver(in_port);
    MIDIOutDriver* out_driver = MIDIManager::GetOutDriver(out_port);
    in_driver->LockQueue();
    for (unsigned int i = 0; i < in_driver->GetQueueSize(); i++) {
        std::cout << "Message found\n";
        in_driver->ReadMessage(rmsg, i);
        msg = rmsg.msg;
        if (msg.IsChannelMsg()) {
            if (in_channel == msg.GetChannel() || in_channel == -1) {
                if (out_channel != -1)
                    msg.SetChannel(out_channel);
                if (processor)
                    processor->Process(&msg);
                out_driver->OutputMessage(msg);
            }
        }
    }
    in_driver->UnlockQueue();
    proc_lock.unlock();
}
