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
**	Copyright 1986 to 1998 By J.D. Koftinoff Software, Ltd.
**
**	All rights reserved.
**
**	No one may duplicate this source code in any form for any reason
**	without the written permission given by J.D. Koftinoff Software, Ltd.
**
*/
/*
** Copyright 2016 By N. Cassetta
** myjdkmidi library
**
** CHECKED with jdksmidi. CHANGES:
**  - used fstreams for reading - writing. Dropped old c FILE objects
**  - merged class MIDIFileWriteStreamFileName into MIDIFileWriteStreamFile (symmetrical to MIDIFileReadReadFile)
**  - revised doxygen comments
*/


/// \file
/// Contains the definition of the class MIDIFileWiter; this is a low level object used by the MIDIFileWriteMultiTrack for
/// saving MIDI files, and is not documented.


#ifndef _JDKMIDI_FILEWRITE_H
#define _JDKMIDI_FILEWRITE_H

#include <fstream>

#include "msg.h"        // includes "midi.h" and "sysex.h"



// EXCLUDED FROM DOCUMENTATION
// Implements a low level set of methods for writing MIDI events to a std::ostream in MIDI file format.
// Used by the MIDIFileWriteMultiTrack class and higher level functions, and you don't need to deal with
// this (unless you want to implement your custom routines for writing MIDI files), so it is not
// documented.
class MIDIFileWriter {
    public:
                        MIDIFileWriter(std::ostream *out_stream_);
        virtual   	    ~MIDIFileWriter()       {}

        bool            ErrorOccurred()         { return error; }
        unsigned long   GetFileLength()         { return file_length; }
        unsigned long   GetTrackLength()        { return track_length; }
        void            ResetTrackLength()      { track_length = 0; }
        void            ResetTrackTime()        { track_time = 0; }

        void            WriteFileHeader(int format, int ntrks, int division);
        void            WriteTrackHeader(unsigned long length);
        void            WriteEvent(const MIDITimedMessage &msg);
        void            WriteChannelEvent(const MIDITimedMessage &msg);
        void            WriteSysExEvent(const MIDITimedMessage &msg);
        void            WriteMetaEvent( unsigned long time, unsigned char type, const unsigned char *data, long length );
        void            WriteEndOfTrack(unsigned long time);

        virtual void    RewriteTrackLength();

    protected:
        virtual	void    Error(char *s);

        void            WriteCharacter(unsigned char c) { out_stream->put(c); if(!out_stream->good()) error = true; }
        void            Seek(unsigned long pos)         { out_stream->seekp(pos); if (!out_stream->good()) error = true; }
        void            IncrementCounters(int c)        { track_length += c; file_length += c; }
        void            WriteShort(unsigned short c);
        void            Write3Char(long c);
        void            WriteLong(unsigned long c);
        int             WriteVariableNum(unsigned long n);
        void            WriteDeltaTime(unsigned long time);

    private:
        bool            error;
        bool            within_track;
        unsigned long   file_length;
        unsigned long   track_length;
        unsigned long   track_time;
        unsigned long   track_position;
        unsigned char   running_status;

        std::ostream*   out_stream;
};


#endif

