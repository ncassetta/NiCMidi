/*
 *  libjdkmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  *** RELEASED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) April 27, 2004 ***
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// updated to reflect changes in jdksmidi

#include "../include/manager.h"


std::vector<MIDIOutDriver*> MIDIManager::MIDI_outs;
std::vector<std::string> MIDIManager::MIDI_out_names;
std::vector<MIDIInDriver*> MIDIManager::MIDI_ins;
std::vector<std::string> MIDIManager::MIDI_in_names;
std::vector<MIDITickComponent*> MIDIManager::MIDITicks;

//MIDISequencer* MIDIManager::sequencer = 0;
std::mutex MIDIManager::proc_lock;


MIDIManager main_manager;



MIDIManager::MIDIManager() {
#ifdef WIN32    //TODO: this is temporary, needed by WINDOWS10
     CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif // WIN32
    try {
        RtMidiOut temp_MIDI_out;
        for (unsigned int i = 0; i < temp_MIDI_out.getPortCount(); i++) {
            MIDI_outs.push_back(new MIDIOutDriver(i));
            MIDI_out_names.push_back(temp_MIDI_out.getPortName(i));
        }
        RtMidiIn temp_MIDI_in;
        for (unsigned int i = 0; i < temp_MIDI_in.getPortCount(); i++) {
            MIDI_ins.push_back(new MIDIInDriver(i));
            MIDI_in_names.push_back(temp_MIDI_in.getPortName(i));
        }
    }
    catch (RtMidiError &error) {
        error.printMessage();
        exit(EXIT_FAILURE);
    }
    MIDITimer::SetMIDITick(TickProc, this);
}


MIDIManager::~MIDIManager() {
    MIDITimer::HardStop();
    for (unsigned int i = 0; i < MIDI_outs.size(); i++)
        delete MIDI_outs[i];
    for (unsigned int i = 0; i < MIDI_ins.size(); i++)
        delete MIDI_ins[i];

#ifdef WIN32
    CoUninitialize();
#endif // WIN32
}


void MIDIManager::Reset() {
    MIDITimer::HardStop();

    MIDITicks.clear();
    for(unsigned int i = 0; i < MIDI_outs.size(); i++)
        MIDI_outs[i]->Reset();
    for(unsigned int i = 0; i < MIDI_ins.size(); i++)
        MIDI_ins[i]->Reset();
}


MIDISequencer* MIDIManager::GetSequencer() {
    for (unsigned int i = 0; i < MIDITicks.size(); i++)
        if (MIDITicks[i]->GetPriority() == PR_SEQ)
            return (MIDISequencer *)MIDITicks[i];
    return 0;
}


void MIDIManager::OpenInPorts() {
    for (unsigned int i = 0; i < MIDI_ins.size(); i++)
        MIDI_ins[i]->OpenPort();
}


void MIDIManager::CloseInPorts() {
    for (unsigned int i = 0; i < MIDI_ins.size(); i++)
        MIDI_ins[i]->ClosePort();
}


void MIDIManager::OpenOutPorts() {
    for (unsigned int i = 0; i < MIDI_outs.size(); i++)
        MIDI_outs[i]->OpenPort();
}


void MIDIManager::CloseOutPorts() {
    for (unsigned int i = 0; i < MIDI_outs.size(); i++)
        MIDI_outs[i]->ClosePort();
}


void MIDIManager::AllNotesOff() {
    for (unsigned int i = 0; i < MIDI_outs.size(); i++)
        MIDI_outs[i]->AllNotesOff();
}


void MIDIManager::AddMIDITick(MIDITickComponent* tick) {
    unsigned int i;
    // if tick has PR_FIRST priority it goes at first place in the vector
    if (tick->GetPriority() == PR_FIRST)
        i = 0;
    // if has PR_LAST goes to the last place
    else if (tick->GetPriority() == PR_LAST)
        i = MIDITicks.size();
    // finds the correct position for tick
    else {
        i = 0;
        while (i < MIDITicks.size() && MIDITicks[i]->GetPriority() <= tick->GetPriority())
            i++;
    }
    // prevents the TickProc from reading from the MIDITicks vector while we are messing about it
    proc_lock.lock();
    // we can have only one sequencer! If found a previous substitute it
    if (i > 0 && tick->GetPriority() == PR_SEQ && MIDITicks[i - 1]->GetPriority() == PR_SEQ)
        MIDITicks[i - 1] = tick;
    else
    // add the MIDITickComponent
        MIDITicks.insert(MIDITicks.begin() + i, tick);
    proc_lock.unlock();
    std::cout << "Inserted new MIDITickComponent into the queue" << std::endl;
    for (unsigned int i = 0; i < MIDITicks.size(); i ++)
        std::cout << i + 1 << "\tAddress: " << MIDITicks[i] << "\tPriority: " << MIDITicks[i]->GetPriority() << std::endl;
}


bool MIDIManager::RemoveMIDITick(MIDITickComponent* tick) {
    unsigned int i = 0;
    for ( ; i < MIDITicks.size(); i++)
        if (MIDITicks[i] == tick)
            break;
    // item not found
    if (i == MIDITicks.size())
        return false;
    // prevents the TickProc from reading from the MIDITicks vector while we are messing about it
    proc_lock.lock();
    if (MIDITicks[i]->IsPlaying())
        MIDITicks[i]->Stop();
    MIDITicks.erase(MIDITicks.begin() + i);
    proc_lock.unlock();
    return true;
}


void MIDIManager::TickProc(tMsecs sys_time, void* p) {
    proc_lock.lock();
    for (unsigned int i = 0; i < MIDITicks.size(); i++) {
        MIDITickComponent* tp = MIDITicks[i];
        if (tp->IsPlaying())
            tp->GetFunc()(sys_time, tp);
    }

    for (unsigned int i = 0; i < MIDI_ins.size(); i++)
        if (MIDI_ins[i]->IsPortOpen())
            MIDI_ins[i]->FlushQueue();
    proc_lock.unlock();

    //std::cout << "MIDIManager::TickProc" << std::endl;
}
