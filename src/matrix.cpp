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
**	Copyright 1986 to 1998 By J.D. Koftinoff Software, Ltd.
**
**	All rights reserved.
**
**	No one may duplicate this source code in any form for any reason
**	without the written permission given by J.D. Koftinoff Software, Ltd.
**
*/
/*
** Copyright 2016 By N. Cassetta
** myjdkmidi library
** see header for changes against jdksmidi
*/

#include "../include/matrix.h"


MIDIMatrix::MIDIMatrix() {
    for(int channel = 0; channel < 16; channel++) {
        channel_count[channel] = 0;
        hold_pedal[channel] = false;
        for(unsigned char note = 0; note < 128; note++)
            note_on_count[channel][note] = 0;
    }
    total_count = 0;
}


bool MIDIMatrix::Process(const MIDIMessage& msg) {
    bool ret = false;

    if(msg.IsChannelMsg()) {
        int channel =msg.GetChannel();
        int note = msg.GetNote();

        if(msg.IsAllNotesOff()) {
            ClearChannel(channel);
            ret = true;
        }
        else if(msg.IsNoteOn()) {
            IncNoteCount(channel, note);
            ret = true;
        }
        else if(msg.IsNoteOff()) {
            DecNoteCount(channel, note);
            ret = true;
        }
        else if(msg.IsPedalOn()) {
            hold_pedal[channel] = true;
            ret = true;
        }
        else if(msg.IsPedalOff()) {
            hold_pedal[channel] = false;
            ret = true;
        }
        else
            OtherMessage(msg);
    }
    return ret;
}


void MIDIMatrix::Clear() {
    for(int channel = 0; channel < 16; ++channel)
        ClearChannel(channel);
    total_count = 0;
}


void MIDIMatrix::DecNoteCount(int channel, int note) {
    if(note_on_count[channel][note] > 0) {
      --note_on_count[channel][note];
      --channel_count[channel];
      --total_count;
    }
}


void MIDIMatrix::IncNoteCount(int channel, int note) {
    ++note_on_count[channel][note];
    ++channel_count[channel];
    ++total_count;
}


void MIDIMatrix::ClearChannel(int channel) {
    for(int note = 0; note < 128; ++note) {
      total_count -= note_on_count[channel][note];
      note_on_count[channel][note] = 0;
    }
    channel_count[channel] = 0;
    hold_pedal[channel] = 0;
}
