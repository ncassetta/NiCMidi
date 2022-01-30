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
  Definitions of input/output functions for the example files of
  NiCMidi library.
*/


#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

#include "../include/multitrack.h"
#include "../include/dump_tracks.h"

// gets from the user the string command_buf, then parses it splitting it into command, par1 and par2 substrings
void GetCommand();
// shows the content of a MIDIMultiTrack ordered by time, pausing every 80 lines
void DumpMIDIMultiTrackWithPauses(MIDIMultiTrack *mlt);
// shows the content of a MIDITrack ordered by time, pausing every 80 lines
void DumpMIDITrackWithPauses(MIDITrack* trk, int trk_num);
// shows the attributes of all the tracks of a multitrack, pausing every 80 lines
void DumpAllTracksAttr(MIDIMultiTrack* mlt, bool v);


static const int PAUSE_LINES = 80;

/*
// readable names of track statuses
static const char TRACK_TYPES[6][12] = {
    "MAIN TRACK",
    "TEXT ONLY ",
    "CHANNEL   ",
    "IRREG CHAN",
    "MIXED CHAN",
    "IRREGULAR "

};
*/


#endif // FUNCTIONS_H_INCLUDED
