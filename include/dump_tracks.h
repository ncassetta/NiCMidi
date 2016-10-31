#ifndef DUMP_TRACKS_H_INCLUDED
#define DUMP_TRACKS_H_INCLUDED

#include <iostream>
#include "multitrack.h"


/// Prints a readable string describing the contents of the MIDITimedMessage.
void DumpMIDITimedMessage (MIDITimedMessage* const msg, std::ostream& ost = std::cout);
/// Prints a list of all the MIDI messages in the given track
void DumpMIDITrack (MIDITrack* const t, std::ostream& ost = std::cout);
/// Prints a list of all MIDI messages in the MIDIMultiTrack, a track at once.
void DumpAllTracks (MIDIMultiTrack* const mlt, std::ostream& ost = std::cout);
/// Prints a list of all MIDI messages in the MIDIMultiTrack in temporal order.
void DumpMIDIMultiTrack (MIDIMultiTrack*const mlt, std::ostream& ost = std::cout);



#endif // DUMP_TRACKS_H_INCLUDED
