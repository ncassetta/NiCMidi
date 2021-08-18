/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2010 V.R.Madgazin
 *   www.vmgames.com vrm@vmgames.com
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
/// Contains the definition of the classes MIDIFileEventHandler and MIDIFileReader and of the struct MIDIFileHeader;
/// the first two are low level objects used by the MIDIFileReadMultiTrack for loading MIDI files, and are not documented.


#ifndef _JDKMIDI_FILEREAD_H
#define _JDKMIDI_FILEREAD_H

#include <fstream>
#include <string>

#include "msg.h"            // includes "midi.h" and "sysex.h"



// EXCLUDED FROM DOCUMENTATION BECAUSE UNDOCUMENTED
// An abstract class for objects that can manipulate MIDI events sent by a MIDIFileReader.
// Actually it's only implemented in the MIDIFileReadMultiTrack, which creates a MIDIMultiTrack
// from these events. This has no interest for the user and it is not documented.
// \see LoadMidiFile() for a fast way to load a MIDI file into a MIDIMultiTrack.
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



/// A structure holding data which represent the header of a MIDI file. This is useful if you want to load a file, edit it
/// and then save it with the same format and name. You can get the header of a MIDI file with the GetMIDIFileHeader()
/// global function; see also LoadMIDIFile().
struct MIDIFileHeader {
    MIDIFileHeader() : format(0), ntrks(0), division(0), filename("") {}
    short format;               ///< The file format (currently only 0 and 1 are allowed by the library).
    short ntrks;                ///< The number of tracks.
    short division;             ///< The number of MIDI ticks for a quarter note.
    std::string filename;       ///< The file name
};



// EXCLUDED FROM DOCUMENTATION BECAUSE UNDOCUMENTED
// Converts a stream of *char* read from a std::istream into MIDI data and sends them to a
// MIDIFileEventHandler.
// Used in conjunction with the MIDIFileReadMultiTrack class for reading MIDI files, and you don't need
// to deal with this (unless you want to implement your custom routines for reading MIDI files).
class MIDIFileReader {
    public:

        // In the constructor you must specify the MIDIFileReadStream.\ The stream must be already open.
                                        MIDIFileReader (std::istream* ist,
                                                        MIDIFileEventHandler *ev_h,
                                                        unsigned long max_msg_len = 8192);

        virtual                         ~MIDIFileReader();
        void                            Reset();
        virtual	int                     ReadHeader();
        virtual bool                    Parse();

        int		                        GetFormat()	const		{ return header.format; }
        int		                        GetNumberTracks() const { return header.ntrks; }
        int		                        GetDivision() const		{ return header.division; }

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
