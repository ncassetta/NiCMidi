#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

#include "../include/multitrack.h"
#include "../include/dump_tracks.h"
#include <iostream>
#include <string>

void GetCommand();
void DumpMIDIMultiTrackWithPauses(MIDIMultiTrack *mlt);
void DumpMIDITrackWithPauses(MIDITrack* trk, int trk_num);

static const char TRACK_TYPES[6][12] = {
    "MAIN TRACK",
    "TEXT ONLY ",
    "CHANNEL   ",
    "IRREG CHAN",
    "MIXED CHAN",
    "IRREGULAR "

};


#endif // FUNCTIONS_H_INCLUDED
