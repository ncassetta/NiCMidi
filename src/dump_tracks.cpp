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


#include "../include/dump_tracks.h"

static unsigned char chan_from_1 = 0;


static const char* trk_types[10] = {
        "EMPTY",
        "CONDUCTOR TRACK",
        "TEXT TRACK",
        "CHANNEL TRACK",
        "CHANNEL AND OTHER EVENTS",
        "MORE CHANNELS EVENTS",
        "NOT CLASSIFIED",
        "COMMON SYSEX ONLY",
        "RESET SYSEX ONLY",
        "COMMON AND RESET SYSEX"
    };


void SetChanFrom(unsigned char c) {
    chan_from_1 = c;
}


void DumpMIDITimedMessage (MIDITimedMessage* const msg, std::ostream& ost) {
    if (msg)
        ost << msg->MsgToText(chan_from_1) << std::endl;
}


void DumpMIDITrack (MIDITrack* const trk, std::ostream& ost) {
    for (unsigned int i = 0; i < trk->GetNumEvents(); ++i)
        DumpMIDITimedMessage (trk->GetEventAddress (i), ost);
}


int DumpMIDITrackAttr(MIDITrack* const trk, int num, std::ostream& ost) {
    char s[10];
    int type = trk->GetType();
    MIDITimedMessage* msg;

    sprintf(s, "%2d", num);
    ost << "Track " << s << "   Name: ";
    for (unsigned int i = 0; i < trk->GetNumEvents(); i++) {
        msg = trk->GetEventAddress(i);
        if (msg->IsTextEvent()) {
            ost << msg->GetText();
            break;
        }
        if (msg->IsChannelMsg())
            break;
    }

    ost << std::endl << "    Type: " << trk_types[type];
    if (type == MIDITrack::TYPE_CHAN || type == MIDITrack::TYPE_IRREG_CHAN)
        ost << " (" << trk->GetChannel() + chan_from_1 << ")";
    ost << std::endl;
    sprintf(s, "%6d", trk->GetNumEvents()),
    ost << "Events in track: " << s << "\t   End of track time: " << trk->GetEndTime()
        << std::endl;
    return 3;           // it always writes 3 lines
}


int DumpMIDITrackAttrVerbose(MIDITrack* const trk, int num, std::ostream& ost) {
    char s[10];
    int type = trk->GetType();
    int status = trk->GetStatus();
    MIDITimedMessage* msg;
    int lines = 7;          // this is the minimum number of text lines always written

    sprintf(s, "%2d", num);
    ost << "Track " << s << "   Name: ";
    for (unsigned int i = 0; i < trk->GetNumEvents(); i++) {
        msg = trk->GetEventAddress(i);
        if (msg->IsTextEvent()) {
            ost << msg->GetText();
            break;
        }
        if (msg->IsChannelMsg())
            break;
    }

    ost << std::endl << "    Type: " << trk_types[type] << std::endl;
    if (status & MIDITrack::HAS_MAIN_META) {
        ost << "    Has conductor meta events" << std::endl;
        lines++;
    }
    if (status & MIDITrack::HAS_TEXT_META) {
        ost << "    Has text meta events" << std::endl;
        lines++;
    }
    if (status & MIDITrack::HAS_ONE_CHAN) {
        ost << "    Has channel events (channel " << trk->GetChannel() + chan_from_1 << ")" << std::endl;
        lines++;
    }
    if (status & MIDITrack::HAS_MANY_CHAN) {
        ost << "    Has channel events (more than one channel)" << std::endl;
        lines++;
    }
    if (status & MIDITrack::HAS_SYSEX) {
        ost << "    Has sysex events" << std::endl;
        lines++;
    }
    if (status & MIDITrack::HAS_RESET_SYSEX) {
        ost << "    Has sysex reset events" << std::endl;
        lines++;
    }

    if (MIDIManager::IsValidInPortNumber(trk->GetInPort()))
        ost << "In port: " << MIDIManager::GetMIDIInName(trk->GetInPort()) << std::endl;
    else
        ost << "In port: Not Valid" << std::endl;
    if (trk->GetRecChannel() != -1)
        ost << "Recording channel: " << trk->GetRecChannel() << std::endl;
    else
        ost << "Recording channel: any" << std::endl;
    if (MIDIManager::IsValidOutPortNumber(trk->GetOutPort()))
        ost << "Out port: " << MIDIManager::GetMIDIOutName(trk->GetOutPort()) << std::endl;
    else
        ost << "Out port: Not Valid" << std::endl;
    ost << "Time shift: " << trk->GetTimeShift() << std::endl;

    sprintf(s, "%6d", trk->GetNumEvents()),
    ost << "Events in track: " << s << "\t   End of track time: " << trk->GetEndTime()
        << std::endl;
    return lines;
}





void DumpAllTracks (MIDIMultiTrack* const mlt, std::ostream& ost) {
    ost << "DUMP OF MIDI MULTITRACK BY TRACKS" << std::endl;
    ost << "Clocks per beat: " << mlt->GetClksPerBeat() << std::endl << std::endl;

    for (unsigned int i = 0; i < mlt->GetNumTracks(); ++i) {
        ost << "Dump of track " << i << std::endl;
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


void CheckMIDIMatrix(const MIDIMatrix& m, std::ostream& ost) {
    char s[120];
    ost << "STATE OF THE MIDI MATRIX AT " << &m << std::endl;
    for (unsigned int i = 0; i < 16; i++) {
        char* st = s;
        st += sprintf (st, "Chan %2d      Notes on: %2d ", i, m.GetChannelCount(i));
        st += (m.GetChannelCount(i) ? sprintf(st, "(min %3d, max %3d)    ", m.GetMinNoteOn(i), m.GetMaxNoteOn(i)) :
                                      sprintf(st, "                      "));
        st += sprintf(st, "Pedal %s", (m.GetHoldPedal(i) ? "ON\n" : "OFF\n"));
        ost << s;
    }
    ost << "Total note count: " << m.GetTotalCount() << std::endl << std::endl;
}
