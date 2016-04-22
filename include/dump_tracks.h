#ifndef DUMP_TRACKS_H_INCLUDED
#define DUMP_TRACKS_H_INCLUDED

#include "multitrack.h"


void DumpMIDITimedBigMessage ( MIDITimedBigMessage *msg );
void DumpMIDITrack ( MIDITrack *t );
void DumpAllTracks ( MIDIMultiTrack *mlt );
void DumpMIDIMultiTrack ( MIDIMultiTrack *mlt );



#endif // DUMP_TRACKS_H_INCLUDED
