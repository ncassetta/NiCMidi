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
/// Contains the definition of the class MIDIFileWriteMultiTrack, used for saving MIDI files, plus some related function.


#ifndef _JDKMIDI_FILEWRITEMULTITRACK_H
#define _JDKMIDI_FILEWRITEMULTITRACK_H

#include "filewrite.h"
#include "multitrack.h"


///
/// Writes the contents of a MIDIMultiTrack to a std::ostream in the MIDI file format.
/// Used by other classes and functions for reading MIDI files; the file will be in MIDI format 0 if the multitrack
/// has only one track, otherwise  the format will be 1.
///
/// If you want to save a MIDIMultiTrack into a  MIDI file you probably will use the simple and fast WriteMIDIFile()
/// global function (that creates and uses this class), so this is not documented.
///
class MIDIFileWriteMultiTrack {
/// \cond EXCLUDED
    public:
        /// The constructor creates an object which can write the multitrack events to the given C++ stream.
        /// The pointed objects are not owned by the class.
                                MIDIFileWriteMultiTrack(const MIDIMultiTrack *mlt, std::ostream *ost);
        /// The destructor.
        virtual                 ~MIDIFileWriteMultiTrack()      {}
        /// Writes the multitrack events to the std::ostream in the standard MIDI file format
        bool                    Write(int num_tracks, int division);

    private:

        virtual bool            PreWrite();
        virtual bool            PostWrite();

        const MIDIMultiTrack    *multitrack;
        MIDIFileWriter          writer;
/// \endcond
};


/// \addtogroup GLOBALS
///@{

/// \name Functions for saving MIDI files
///@{
/// Writes the given MIDIMultiTrack object into a MIDI file.
/// \param filename the file name
/// \param format the MIDI file format (only 0 and 1 are supported)
/// \param tracks the MIDIMultiTrack to be written
/// \param strip if the format is 1 (many tracks) and this is *true*, empty tracks are skipped
/// \return **true** if the writing was successful.
bool WriteMIDIFile(const char* filename, int format, const MIDIMultiTrack* tracks, bool strip = false);
/// Writes the given MIDIMultiTrack object into a MIDI file.
/// \see WriteMIDIFile(const char* filename, format, tracks, strip)
bool WriteMIDIFile(const std::string& filename, int format, const MIDIMultiTrack* tracks, bool strip = false);
///@}
///@}

#endif
