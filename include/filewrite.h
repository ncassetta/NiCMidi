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


#ifndef _JDKMIDI_FILEWRITE_H
#define _JDKMIDI_FILEWRITE_H

#include <fstream>

#include "midi.h"
#include "msg.h"
#include "sysex.h"
#include "file.h"


///
/// This class is used internally for writing MIDI files. It writes a stream of *char* to a C++ ostream object,
///

class MIDIFileWriteStream {
public:
    /// In this constructor you must specify the filename.\ The constructor tries to open the file, you
    /// should call IsValid() for checking if it was successful
    MIDIFileWriteStream( const char *fname );
    /// In this constructor you must specify and already open ostream object, so you can write to whatever
    /// output stream
    MIDIFileWriteStream(std::ostream* ofs);
    /// The destructor deletes the ostream if it was opened by the ctor
    virtual ~MIDIFileWriteStream();

    /// Implements pure virtual parent function
    long Seek( long pos, int whence=SEEK_SET );
    /// Implements pure virtual parent function
    int WriteChar( int c );
    /// Returns *true* if the ostream is open
    bool IsValid();

private:
    std::ostream* outfs;
    std::streampos begin;
    bool del;
};



///
/// This class is used internally for writing MIDI files.
///

class MIDIFileWriter {
    public:
                        MIDIFileWriter(MIDIFileWriteStream *out_stream_);
        virtual   	    ~MIDIFileWriter();


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
        //void            WriteTextEvent(unsigned long time, unsigned short text_type, const char *text);
        void            WriteMetaEvent( unsigned long time, unsigned char type, const unsigned char *data, long length );
        //void            WriteTempo(unsigned long time, long tempo);
        //void            WriteKeySignature(unsigned long time, char sharp_flat, char minor);
        //void            WriteTimeSignature(
        //                    unsigned long time,
        //                    char numerator=4,
        //                    char denominator_power=2,
        //                    char midi_clocks_per_metronome=24,
        //                    char num_32nd_per_midi_quarter_note=8
        //                );

        void            WriteEndOfTrack(unsigned long time);

        virtual void    RewriteTrackLength();

    protected:
        virtual	void    Error(char *s);

        void            WriteCharacter( uchar c ) { if(out_stream->WriteChar(c) < 0) error = true; }
        void            Seek(long pos)            { if(out_stream->Seek(pos) < 0) error = true; }
        void            IncrementCounters(int c)  { track_length += c; file_length += c; }
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
        uchar           running_status;

        MIDIFileWriteStream *out_stream;
};


#endif







