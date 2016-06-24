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
/*
** Copyright 2016 By N. Cassetta
** myjdkmidi library
**
** CHECKED with jdksmidi. NO CHANGES
*/

#ifndef _JDKMIDI_FILEWRITEMULTITRACK_H
#define _JDKMIDI_FILEWRITEMULTITRACK_H

#include "filewrite.h"
#include "multitrack.h"


class MIDIFileWriteMultiTrack {
    public:

                                MIDIFileWriteMultiTrack(const MIDIMultiTrack *mlt_, MIDIFileWriteStream *strm_);
        virtual                 ~MIDIFileWriteMultiTrack();

        bool                    Write(int num_tracks, int division);

    private:

        virtual bool            PreWrite();
        virtual bool            PostWrite();

        const MIDIMultiTrack    *multitrack;
        MIDIFileWriter          writer;
};

/// Writes the given MIDIMultiTrack object into the MIDI file denoted by *filename*; *format*
/// is the MIDI file format (only 0 and 1 are supported).
/// Returns *true* if the writing was successful.
bool WriteMIDIFile(const char* filename, int format, const MIDIMultiTrack* tracks);

#endif
