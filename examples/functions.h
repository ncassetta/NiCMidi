/*
  Definitions of input/output functions for the example files of
  myjdkmidi library.

  Copyright (C) 2018 - 2020 N.Cassetta
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

// readable names of track statuses
static const char TRACK_TYPES[6][12] = {
    "MAIN TRACK",
    "TEXT ONLY ",
    "CHANNEL   ",
    "IRREG CHAN",
    "MIXED CHAN",
    "IRREGULAR "

};


#endif // FUNCTIONS_H_INCLUDED
