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


/// \file
/// Contains the definition of some helper functions which can be useful for debugging
/// purposes.

#ifndef DUMP_TRACKS_H_INCLUDED
#define DUMP_TRACKS_H_INCLUDED

#include <iostream>
#include "multitrack.h"


/// \addtogroup GLOBALS
///@{

/// \name Helper functions for viewing track content
///@{
/// Prints a readable string describing the contents of the given MIDIMessage (or MIDITimedMessage).
void DumpMIDITimedMessage (MIDITimedMessage* const msg, std::ostream& ost = std::cout);
/// Prints a list of all the MIDI messages in the given MIDItrack.
void DumpMIDITrack (MIDITrack* const trk, std::ostream& ost = std::cout);
/// Prints the properties of the given MIDItrack.
void DumpMIDITrackAttr (MIDITrack* const trk, std::ostream& ost = std::cout);
/// Prints a list of all MIDI messages in the given MIDIMultiTrack, a track at once.
void DumpAllTracks (MIDIMultiTrack* const mlt, std::ostream& ost = std::cout);
/// Prints a list of all MIDI messages in the given MIDIMultiTrack in temporal order.
void DumpMIDIMultiTrack (MIDIMultiTrack*const mlt, std::ostream& ost = std::cout);
///@}
///@}


#endif // DUMP_TRACKS_H_INCLUDED
