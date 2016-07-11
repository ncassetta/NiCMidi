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
/*
** Copyright 2016 By N. Cassetta
** myjdkmidi library
** see header for changes against jdksmidi
*/


#include "../include/world.h"
#include "../include/process.h"
#include "../include/dump_tracks.h"

#include <string>



 void MIDIMultiProcessor::SetProcessor(MIDIProcessor* proc, int pos) {
    if (pos >= 0 && pos < processors.size())
        processors[pos] = proc;
    else
        processors.push_back(proc);
}


void MIDIMultiProcessor::RemoveProcessor(int pos) {
    if (pos >= 0 && pos < processors.size())
        processors.erase(processors.begin() + pos);
}


void MIDIMultiProcessor::RemoveProcessor(const MIDIProcessor* proc) {
    unsigned int i = 0;
    for (; i < processors.size(); i++)
        if (processors[i] == proc) break;
    if (i < processors.size())
        processors.erase(processors.begin() + i);
}


bool MIDIMultiProcessor::Process(MIDITimedMessage *msg) {
    bool ret = true;
    for(unsigned int i = 0; i < processors.size(); ++i)
        if(processors[i]->Process(msg) == false) {
            if (process_mode != IGNORE)
                ret = false;
            if (process_mode == STOP)
                break;
        }
    return ret;
}



MIDIProcessorTransposer::MIDIProcessorTransposer() {
    for(int i = 0; i < 16; ++i)
        trans_amount[i] = 0;
}


void MIDIProcessorTransposer::SetAllTranspose(int val) {
    for (int chan = 0; chan < 16; ++chan)
        trans_amount[chan] = val;
}


bool MIDIProcessorTransposer::Process (MIDITimedMessage *msg) {
    if(msg->IsNoteOn() || msg->IsNoteOff() || msg->IsPolyPressure()) {
        int trans = trans_amount[msg->GetChannel()];
        int new_note = ((int)msg->GetNote()) + trans;
        if(new_note > 127 || new_note < 0)
            // delete event if out of range
            return false;
        else
            // set new note number
            msg->SetNote( (unsigned char)new_note );
    }
    return true;
}


MIDIProcessorRechannelizer::MIDIProcessorRechannelizer() {
    for(int i = 0; i < 16; ++i)
        rechan_map[i] = i;
}


void MIDIProcessorRechannelizer::Reset() {
    for(int i = 0; i < 16; ++i)
        rechan_map[i] = i;
}


void MIDIProcessorRechannelizer::SetAllRechan(int dest_chan) {
    for(int i = 0; i < 16; ++i)
        rechan_map[i] = dest_chan;
}


bool MIDIProcessorRechannelizer::Process(MIDITimedMessage *msg) {
    if(msg->IsChannelMsg()) {
        int new_chan = rechan_map[msg->GetChannel()];
        if(new_chan == -1)
            // this channel is to be deleted! return false
            return false;
        msg->SetChannel( (unsigned char)new_chan );
    }
    return true;
}


bool MIDIProcessorPrinter::Process(MIDITimedMessage *msg) {
    if (print_on)
        std::cout << msg->MsgToText() << std::endl;
    return true;
}



// TODO: implement a MIDIProcessorFilter
