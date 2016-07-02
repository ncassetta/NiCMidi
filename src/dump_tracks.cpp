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


#include "../include/midi.h"
#include "../include/track.h"
#include "../include/multitrack.h"

#include <iostream>


void DumpMIDITimedMessage (MIDITimedMessage* const msg) {
    if (msg)
        std::cout << msg->MsgToText() << std::endl;
}


void DumpMIDITrack (MIDITrack* const t) {
    MIDITimedMessage *msg;

    std::cout << "Track dump" << std::endl;
    for (unsigned int i = 0; i < t->GetNumEvents(); ++i)
        DumpMIDITimedMessage (t->GetEventAddress (i));
}


void DumpAllTracks (MIDIMultiTrack* const mlt) {
    std::cout << "DUMP OF MIDI MULTITRACK\n";
    std::cout << "Clocks per beat: " << mlt->GetClksPerBeat() << "\n" << std::endl;

    for (int i = 0; i < mlt->GetNumTracks(); ++i) {
        std::cout << "Dump of track" << i << std::endl;
        for (int j = 0; j < mlt->GetTrack(i)->GetNumEvents(); j++)
            DumpMIDITimedMessage(mlt->GetTrack (i)->GetEventAddress(j));
        std::cout << std::endl;
    }
}


void DumpMIDIMultiTrack (MIDIMultiTrack* const mlt) {
    MIDIMultiTrackIterator i (mlt);
    MIDITimedMessage *msg;
    char s[10];
    int trk_num;

    std::cout << "DUMP OF MIDI MULTITRACK\n";
    std::cout << "Clocks per beat: " << mlt->GetClksPerBeat() << "\n" << std::endl;

    i.GoToTime (0);
    do {
        if (i.GetCurEvent (&trk_num, &msg)) {
            sprintf (s, "#%2d - ", trk_num);
            std::cout << s;
            DumpMIDITimedMessage (msg);
        }
    } while (i.GoToNextEvent());
}
