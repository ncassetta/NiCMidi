/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2021, 2022  Nicola Cassetta
 *   https://github.com/ncassetta/NiCMidi
 *
 *   This file is part of NiCMidi.
 *
 *   NiCMidi is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation, either version 3 of
 *   the License, or (at your option) any later version.
 *
 *   NiCMidi is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with NiCMidi. If not, see <http://www.gnu.org/licenses/>.
 */


#include "../include/thru.h"
#include "../include/manager.h"


MIDIThru::MIDIThru() : MIDITickComponent(PR_PRE_SEQ, StaticTickProc), in_port(0), out_port(0), in_channel(-1),
                                         out_channel(-1), processor(0)
{
    //std::cout << "MIDIThru constructor" << std::endl;
    //check if in and out ports exist
    if (!MIDIManager::IsValidInPortNumber(0) || !MIDIManager::IsValidOutPortNumber(0))
        throw RtMidiError("MIDIThru needs almost a MIDI in and out port in the system\n", RtMidiError::INVALID_DEVICE);
}


MIDIThru::~MIDIThru() {
    //std::cout << "MIDIThru destructor" << std::endl;
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


bool MIDIThru::SetInPort(unsigned int port) {
    if (!MIDIManager::IsValidInPortNumber(port))
        return false;                               // avoids out of range errors
    if (port == in_port)
        return true;                                // trying to assign same ports: nothing to do

    if (IsPlaying()) {
        std::lock_guard<std::recursive_mutex> lock(proc_lock);
        MIDIManager::GetInDriver(in_port)->ClosePort();
        SilentOut();
        MIDIManager::GetInDriver(port)->OpenPort();
        in_port = port;
    }
    else
        in_port = port;
    return true;
}


bool MIDIThru::SetOutPort(unsigned int port) {
    if (!MIDIManager::IsValidOutPortNumber(port))
        return false;                               // avoids out of range errors
    if (port == out_port)
        return true;                                // trying to assign same ports: nothing to do

    if (IsPlaying()) {
        std::lock_guard<std::recursive_mutex> lock(proc_lock);
        SilentOut();
        MIDIManager::GetOutDriver(out_port)->ClosePort();
        MIDIManager::GetOutDriver(port)->OpenPort();
        out_port = port;
    }
    else
        out_port = port;
    return true;
}


void MIDIThru::SetProcessor(MIDIProcessor* proc) {
    if (IsPlaying()) {
        std::lock_guard<std::recursive_mutex> lock(proc_lock);
        SilentOut();
        processor = proc;
    }
    else
        processor = proc;
}


bool MIDIThru::SetInChannel(int chan) {
    if (chan < -1 || chan > 15)                     // avoids out of range errors
        return false;
    if (IsPlaying()) {
        std::lock_guard<std::recursive_mutex> lock(proc_lock);
        SilentOut();
        in_channel = chan;
    }
    else
        in_channel = chan;
    return true;
}


bool MIDIThru::SetOutChannel(int chan) {
    if (chan < -1 || chan > 15)                     // avoids out of range errors
        return false;

    if (IsPlaying()) {
        std::lock_guard<std::recursive_mutex> lock(proc_lock);
        SilentOut();
        out_channel = chan;
    }
    else
        out_channel = chan;
    return true;
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
        std::lock_guard<std::recursive_mutex> lock(proc_lock);
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
    std::lock_guard<std::recursive_mutex> lock(proc_lock);

    //static unsigned int times = 0;
    //times++;
    //if (!(times % 100))
    //    std::cout << "MIDIThru::TickProc() called " << times << " times\n";

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
}
