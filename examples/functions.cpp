/*
  Input/output functions for the example files of myjdkmidi
  library.

  Copyright (C) 2018 - 2019 N.Cassetta
  ncassetta@tiscali.it

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program;
  if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "functions.h"

using namespace std;

std::string command, par1, par2;


// gets from the user the string command_buf, then parses it dividing it into command, par1 and par2 substrings
void GetCommand()
{
    string command_buf;
    size_t pos1, pos2;

    cout << "\n=> ";
    getline(cin, command_buf);

    command = "";
    par1 = "";
    par2 = "";

    for (size_t i = 0; i < command_buf.size(); ++i)
        command_buf[i] = tolower( command_buf[i]);

    pos1 = command_buf.find_first_not_of ( " ");
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if (pos1 == string::npos)
        return;

    command = command_buf.substr (pos1, pos2 - pos1);

    pos1 = command_buf.find_first_not_of (" ", pos2);
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if (pos1 == string::npos)
        return;

    par1 = command_buf.substr (pos1, pos2 - pos1);
    pos1 = command_buf.find_first_not_of (" ", pos2);
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if (pos1 == string::npos)
        return;

    par2 = command_buf.substr (pos1, pos2 - pos1);
}


// shows the content of a MIDIMultiTrack ordered by time
void DumpMIDIMultiTrackWithPauses (MIDIMultiTrack *mlt) {
    MIDIMultiTrackIterator iter (mlt);
    MIDITimedMessage *msg;
    int trk_num;
    int num_lines = 0;

    printf ("DUMP OF MIDI MULTITRACK\n");
    printf ("Clocks per beat: %d\n\n", mlt->GetClksPerBeat() );

    while (iter.GetNextEvent (&trk_num, &msg)) {
        printf ("Tr %2d - ", trk_num);
        DumpMIDITimedMessage (msg);
        num_lines++;
        if (num_lines == 80) {
            printf ("Press <ENTER> to continue or q + <ENTER> to exit ...\n");
            char ch = std::cin.get();
            if (tolower(ch) == 'q')
                return;
            num_lines = 0;
        }
    }
}


// shows the content of a MIDITrack ordered by time
void DumpMIDITrackWithPauses (MIDITrack *trk, int trk_num) {
    int num_lines = 0;

    printf ("DUMP OF MIDI TRACK %d\n", trk_num);

    for (unsigned int ev_num = 0; ev_num < trk->GetNumEvents(); ev_num++) {
        DumpMIDITimedMessage (trk->GetEventAddress(ev_num));
        num_lines++;
        if (num_lines == 80) {
            printf ("Press <ENTER> to continue or q + <ENTER> to exit ...\n");
            char ch = std::cin.get();
            if (tolower(ch) == 'q')
                return;
            num_lines = 0;
        }
    }
}
