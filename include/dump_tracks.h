/*
 *  libjdkmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  *** RELEASED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) April 27, 2004 ***
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/// \file
/// Contains the definition of some helper functions which can be useful for debugging
/// purposes.

#ifndef DUMP_TRACKS_H_INCLUDED
#define DUMP_TRACKS_H_INCLUDED

#include <iostream>
#include "multitrack.h"


/// \addtogroup GLOBALS
//@{
/// Prints a readable string describing the contents of the given MIDIMessage (or MIDITimedMessage).
void DumpMIDITimedMessage (MIDITimedMessage* const msg, std::ostream& ost = std::cout);
/// Prints a list of all the MIDI messages in the given MIDItrack.
void DumpMIDITrack (MIDITrack* const t, std::ostream& ost = std::cout);
/// Prints a list of all MIDI messages in the given MIDIMultiTrack, a track at once.
void DumpAllTracks (MIDIMultiTrack* const mlt, std::ostream& ost = std::cout);
/// Prints a list of all MIDI messages in the given MIDIMultiTrack in temporal order.
void DumpMIDIMultiTrack (MIDIMultiTrack*const mlt, std::ostream& ost = std::cout);
//@}


#endif // DUMP_TRACKS_H_INCLUDED
