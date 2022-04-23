/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2021, 2022  Nicola Cassetta
 *   https://github.com/ncassetta/NiCMidi
 *
 *   This file is part of NiCMidi.
 *
 *   NiCMidi is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation, either version 3 of
 *   the License, or (at your option) any later version.
 *
 *   NiCMidi is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with NiCMidi. If not, see <http://www.gnu.org/licenses/>.
 */


/// \file
/// Contains the definition of the class MIDIFileWiter; this is a low level object used by the MIDIFileWriteMultiTrack for
/// saving MIDI files, and is not documented.


#ifndef _NICMIDI_FILEWRITE_H
#define _NICMIDI_FILEWRITE_H

#include <fstream>

#include "msg.h"        // includes "midi.h" and "sysex.h"



// EXCLUDED FROM DOCUMENTATION BECAUSE UNDOCUMENTED
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

