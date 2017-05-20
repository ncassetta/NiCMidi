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
//
// Copyright (C) 2010 V.R.Madgazin
// www.vmgames.com vrm@vmgames.com
//
/*
** Copyright 2016 By N. Cassetta
** myjdkmidi library
**
** CHECKED with jdksmidi. CHANGES:
**  - used fstreams for reading - writing. Dropped old c FILE objects
**  - revised doxygen comments
*/

#ifndef _JDKMIDI_FILEREAD_H
#define _JDKMIDI_FILEREAD_H

#include <fstream>

#include "midi.h"
#include "msg.h"
#include "sysex.h"

/*
///
/// This class is used internally for reading MIDI files. It reads a stream of *char* from a C++ istream object.
///
class MIDIFileReadStream {
    public:
        /// In this constructor you must specify the filename.\ The constructor tries to open the file, you
        /// should call IsValid() for checking if it was successful.
        explicit                        MIDIFileReadStream(const char *fname);
        /// In this constructor you must specify and already open istream object, so you can read from whatever
        /// input stream.
        explicit                        MIDIFileReadStream(std::istream* ifs);
        /// The destructor deletes the istream if it was opened by the ctor.
        virtual                         ~MIDIFileReadStream();

        /// Rewind the stream to the beginning.
        virtual void                    Rewind();
        /// Reads a char from the stream.
        virtual int                     ReadChar();
        /// Returns *true* if the istream is open.
        virtual bool                    IsValid();

    private:
        std::istream*                   infs;
        bool                            del;
};
*/



///
/// An abstract class for objects that can manipulate MIDI events sent by a MIDIFile Reader.
/// Actually it's only implemented in the MIDIFileReadMultiTrack, which creates a MIDIultitrack
/// from these events. This has no interest for the user.
///
class MIDIFileEventHandler {
    public:
                                        MIDIFileEventHandler() {}
        virtual                         ~MIDIFileEventHandler() {}

    //
    // The possible events in a MIDI Files
    //

        virtual void                    mf_system_mode(const MIDITimedMessage &msg) = 0;
        virtual void                    mf_note_on(const MIDITimedMessage &msg) = 0;
        virtual void                    mf_note_off(const  MIDITimedMessage &msg) = 0;
        virtual void                    mf_poly_after(const MIDITimedMessage &msg) = 0;
        virtual void                    mf_bender(const MIDITimedMessage &msg) = 0;
        virtual void                    mf_program(const MIDITimedMessage &msg) = 0;
        virtual void                    mf_chan_after(const MIDITimedMessage &msg) = 0;
        virtual void                    mf_control(const MIDITimedMessage &msg) = 0;

        virtual void                    mf_sysex(MIDIClockTime time, const MIDISystemExclusive &ex) = 0;
        virtual void                    mf_arbitrary(MIDIClockTime time, int len, unsigned char *data) = 0;
        virtual void                    mf_metamisc(MIDIClockTime time, int b1, int b2, unsigned char *ch) = 0;
        virtual void                    mf_meta16 (MIDIClockTime time, int type, int b1, int b2) = 0;
        virtual void                    mf_smpte(MIDIClockTime time, int h, int m, int s, int f, int sf) = 0;
        virtual void                    mf_timesig(MIDIClockTime time, int m1, int m2, int m3, int m4) = 0;
        virtual void                    mf_tempo(MIDIClockTime time, int b1, int b2, int b3) = 0;
        virtual void                    mf_keysig(MIDIClockTime time, int sf, int majmin) = 0;
        virtual void                    mf_sqspecific(MIDIClockTime time, int len, unsigned char *data) = 0;
        virtual void                    mf_text(MIDIClockTime time, int type, int len, unsigned char *data) = 0;
        virtual void                    mf_eot(MIDIClockTime time) = 0;

    //
    // the following methods are to be overridden for your specific purpose
    //

        virtual void                    mf_error(const char* err)   = 0;
        virtual void                    mf_starttrack(int trk)      = 0;
        virtual void                    mf_endtrack(int trk)        = 0;
        virtual void                    mf_header(int, int, int)    = 0;

    //
    // Higher level dispatch functions
    //
        virtual	void	                UpdateTime(MIDIClockTime delta_time) = 0;
        virtual	void                    ChanMessage(const MIDITimedMessage &msg);
        virtual	void                    MetaEvent(MIDIClockTime time, int type, int len, unsigned char *buf);
};




///
/// A structure holding data which represent the header of a MIDI file.
///
struct MIDIFileHeader {
    MIDIFileHeader() : format(0), ntrks(0), division(0) {}
    short format;               ///< the file format (currently only 0 and 1 are allowed).
    short ntrks;                ///< the number of tracks.
    short division;             ///< the number of MIDI ticks for a quarter note.
};



///
/// Converts a stream of *char* read from a std::istream into MIDI data and sends them to a
/// MIDIFileEventHandler.
/// Used in conjunction with the MIDIFileReadMultiTrack class for reading MIDI files, and you don't need
/// to deal with this (unless you want to implement your custom routines for reading MIDI files).
///
class MIDIFileReader {
    public:

        /// In the constructor you must specify the MIDIFileReadStream.\ The stream must be already open.
                                        MIDIFileReader (std::istream* ist,
                                                        MIDIFileEventHandler *ev_h,
                                                        unsigned long max_msg_len = 8192);

        virtual                         ~MIDIFileReader();
        virtual	int                     ReadHeader();
        virtual bool                    Parse();

        int		                        GetFormat()			    { return header.format; }
        int		                        GetNumberTracks()       { return header.ntrks; }
        int		                        GetDivision()		    { return header.division; }

    protected:

        virtual void                    mf_error(const char*);

        int                             no_merge;
        MIDIClockTime                   cur_time;
        int                             skip_init;
        unsigned long                   to_be_read;
        int  	                        cur_track;
        int  	                        abort_parse;

        unsigned char*                  the_msg;
        int		                        max_msg_len;
        int   	                        msg_index;

        static const unsigned long      _MThd = ('M')*0x1000000 + ('T')*0x10000 + ('h')*0x100 + ('d');
        static const unsigned long      _MTrk = ('M')*0x1000000 + ('T')*0x10000 + ('r')*0x100 + ('k');


    private:
        unsigned long                   ReadVariableNum();
        unsigned long                   Read32Bit();
        int  	                        Read16Bit();
        void                            ReadTrack();
        void                            MsgAdd(int);
        void                            MsgInit();
        int  	                        EGetC();
        int  	                        ReadMT(unsigned long, int);
        void                            BadByte(int);
        void	                        FormChanMessage(unsigned char st, unsigned char b1, unsigned char b2);

        MIDIFileHeader                  header;

        std::istream*                   in_stream;
        MIDIFileEventHandler*           event_handler;
};


#endif
