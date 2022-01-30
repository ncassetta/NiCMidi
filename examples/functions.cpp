/*
 *   Example file for NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2021, 2022  Nicola Cassetta
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


/*
  Input/output functions for the example files of NiCMidi library.
*/


#include "functions.h"
#include <iostream>
#include <string>

using namespace std;

std::string command, par1, par2, par3;


// gets from the user the string command_buf, then parses it splitting it
// into command, par1, par2 and par3 substrings
void GetCommand() {
    string command_buf;
    size_t pos1, pos2;

    cout << "\n=> ";
    getline(cin, command_buf);

    command = par1 = par2 = par3 = "";

    for (size_t i = 0; i < command_buf.size(); ++i)
        command_buf[i] = tolower(command_buf[i]);

    pos1 = command_buf.find_first_not_of(" ");
    pos2 = command_buf.find_first_of(" ", pos1 + 1);
    if (pos1 == string::npos)
        return;

    command = command_buf.substr (pos1, pos2 - pos1);
    pos1 = command_buf.find_first_not_of (" ", pos2);
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if ( pos1 == string::npos )
        return;

    par1 = command_buf.substr (pos1, pos2 - pos1);
    pos1 = command_buf.find_first_not_of (" ", pos2);
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if (pos1 == string::npos)
        return;

    par2 = command_buf.substr (pos1, pos2 - pos1);
    pos1 = command_buf.find_first_not_of ( " ", pos2);
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if (pos1 == string::npos)
        return;
    par3 = command_buf.substr (pos1, pos2 - pos1);
}



// shows the content of a MIDIMultiTrack ordered by time, pausing every 40 lines
void DumpMIDIMultiTrackWithPauses (MIDIMultiTrack *mlt) {
    MIDIMultiTrackIterator iter(mlt);
    MIDITimedMessage *msg;
    int trk_num;
    int num_lines = 0;

    printf ("DUMP OF MIDI MULTITRACK\n");
    printf ("Clocks per beat: %d\n\n", mlt->GetClksPerBeat() );
    while (iter.GetNextEvent(&trk_num, &msg)) {
        printf ("Tr %2d - ", trk_num);
        DumpMIDITimedMessage (msg);
        num_lines++;
        if (num_lines == PAUSE_LINES / 2) {
            printf ("Press <ENTER> to continue or q + <ENTER> to exit ...\n");
            char ch = std::cin.get();
            if (tolower(ch) == 'q')
                return;
            num_lines = 0;
        }
    }
}


// shows the content of a MIDITrack ordered by time, pausing every 80 lines
void DumpMIDITrackWithPauses (MIDITrack *trk, int trk_num) {
    int num_lines = 0;

    printf ("DUMP OF MIDI TRACK %d\n", trk_num);
    for (unsigned int ev_num = 0; ev_num < trk->GetNumEvents(); ev_num++) {
        DumpMIDITimedMessage (trk->GetEventAddress(ev_num));
        num_lines++;
        if (num_lines == PAUSE_LINES) {
            printf ("Press <ENTER> to continue or q + <ENTER> to exit ...\n");
            char ch = std::cin.get();
            if (tolower(ch) == 'q')
                return;
            num_lines = 0;
        }
    }
}

// shows the attributes of all the tracks of a multitrack, pausing every 80 lines
void DumpAllTracksAttr(MIDIMultiTrack* mlt, bool v) {
    int num_lines = 0;
    for (unsigned int i = 0; i < mlt->GetNumTracks(); i++) {
        num_lines += (v ? DumpMIDITrackAttrVerbose(mlt->GetTrack(i), i) : DumpMIDITrackAttr(mlt->GetTrack(i), i));
        if (num_lines == PAUSE_LINES) {
            printf ("Press <ENTER> to continue or q + <ENTER> to exit ...\n");
            char ch = std::cin.get();
            if (tolower(ch) == 'q')
                return;
            num_lines = 0;
        }
    }
}
