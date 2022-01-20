/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2021  Nicola Cassetta
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
#include "matrix.h"


/// \addtogroup GLOBALS
///@{

/// \name Helper functions for viewing messages content
///@{

/// Sets the numbering of MIDI channels when messages are printed. If c == 0 channels will be numbered 0 ... 15, else
/// 1 ... 16. This will affect all the functions of this file. See \ref NUMBERING.
void SetChanFrom(unsigned char c = 0);
/// Prints a readable string describing the contents of the given MIDIMessage (or MIDITimedMessage).
void DumpMIDITimedMessage (MIDITimedMessage* const msg, std::ostream& ost = std::cout);
/// Prints a list of all the MIDI messages in the given MIDItrack.
void DumpMIDITrack (MIDITrack* const trk, std::ostream& ost = std::cout);
/// Prints the main properties of the given MIDItrack.
/// It prints the track name, its type (see \ref MIDITrack::GetType()), the number of events and the end time.
/// For printing the track messages use DumpMIDITrack().
/// \param trk a pointer to the track
/// \param num will be printed in the first line as track number.
/// \param ost the output stream
/// \return Actually this always returns 3, the number of written text lines
int DumpMIDITrackAttr (MIDITrack* const trk, int num, std::ostream& ost = std::cout);
/// Prints all the properties of the given MIDItrack.
/// It prints the track name, its type (see \ref MIDITrack::GetType()) with a detailed description, the in and
/// out ports, the time shift amount, the number of events and the end time.
/// For printing the track messages use DumpMIDITrack().
/// \param trk a pointer to the track
/// \param num will be printed in the first line as track number.
/// \param ost the output stream
/// \return The number of text lines written, which depends from the track attributes.
int DumpMIDITrackAttrVerbose (MIDITrack* const trk, int num, std::ostream& ost = std::cout);
/// Prints a list of all MIDI messages in the given MIDIMultiTrack, a track at once.
void DumpAllTracks (MIDIMultiTrack* const mlt, std::ostream& ost = std::cout);
/// Prints a list of all MIDI messages in the given MIDIMultiTrack in temporal order.
void DumpMIDIMultiTrack (MIDIMultiTrack* const mlt, std::ostream& ost = std::cout);
///@}
///@}

/// \addtogroup GLOBALS
///@{
/// \name Other helper functions
///@{

/// Helper function which shows the contents of a MIDIMatrix.
void CheckMIDIMatrix(const MIDIMatrix& matrix, std::ostream& ost = std::cout);
///@}
///@}


#endif // DUMP_TRACKS_H_INCLUDED
