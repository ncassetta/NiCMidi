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


#include "../include/dump_tracks.h"


void DumpMIDITimedMessage (MIDITimedMessage* const msg, std::ostream& ost) {
    if (msg)
        ost << msg->MsgToText() << std::endl;
}


void DumpMIDITrack (MIDITrack* const t, std::ostream& ost) {
    for (unsigned int i = 0; i < t->GetNumEvents(); ++i)
        DumpMIDITimedMessage (t->GetEventAddress (i), ost);
}


void DumpAllTracks (MIDIMultiTrack* const mlt, std::ostream& ost) {
    ost << "DUMP OF MIDI MULTITRACK BY TRACKS" << std::endl;
    ost << "Clocks per beat: " << mlt->GetClksPerBeat() << std::endl << std::endl;

    for (unsigned int i = 0; i < mlt->GetNumTracks(); ++i) {
        ost << "Dump of track" << i << std::endl;
        for (unsigned int j = 0; j < mlt->GetTrack(i)->GetNumEvents(); j++)
            DumpMIDITimedMessage(mlt->GetTrack (i)->GetEventAddress(j), ost);
        ost << std::endl;
    }
}


void DumpMIDIMultiTrack (MIDIMultiTrack* const mlt, std::ostream& ost) {
    MIDIMultiTrackIterator iter (mlt);
    MIDITimedMessage *msg;
    char s[10];
    int trk_num;

    ost << "DUMP OF MIDI MULTITRACK\n";
    ost << "Clocks per beat: " << mlt->GetClksPerBeat() << std::endl << std::endl;

    iter.GoToTime (0);
    while (iter.GetNextEvent (&trk_num, &msg)) {
            sprintf (s, "#%2d - ", trk_num);
            ost << s;
            DumpMIDITimedMessage (msg, ost);
    }
}
