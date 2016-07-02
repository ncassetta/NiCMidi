#ifndef DUMP_TRACKS_H_INCLUDED
#define DUMP_TRACKS_H_INCLUDED


#include "multitrack.h"

void DumpMIDITimedMessage (MIDITimedMessage* const msg);
void DumpMIDITrack (MIDITrack* const t);
void DumpAllTracks (MIDIMultiTrack* const mlt);
void DumpMIDIMultiTrack (MIDIMultiTrack*const mlt);



#endif // DUMP_TRACKS_H_INCLUDED
