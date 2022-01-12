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
/// Contains the definition of the class MIDIFileReadMultiTrack, used for loading MIDI files, plus some related function.


#ifndef _JDKMIDI_FILEREADMULTITRACK_H
#define _JDKMIDI_FILEREADMULTITRACK_H

//#include "midi.h"
//#include "msg.h"
//#include "sysex.h"
#include "fileread.h"
#include "multitrack.h"

#include <string>


///
/// Receives MIDI data from a MIDIFileReader (not documented) class and writes them to a MIDIMultiTrack.
/// Used by other classes and functions for reading MIDI files; actually it ignores these kind of messages:
/// - META Sequencer Specific
/// - other not identified meta data
///
/// If you want to load MIDI files into a MIDIMultiTrack you probably will use the simple and fast \ref LoadMIDIFile()
/// global function (that creates and uses this class), so this is not documented.
///
class MIDIFileReadMultiTrack : public MIDIFileEventHandler {
/// \cond EXCLUDED
    public:
                                        MIDIFileReadMultiTrack (MIDIMultiTrack *tracks);
        virtual                         ~MIDIFileReadMultiTrack()       {}

//
// The possible events in a MIDI Files
//
            // these are all unused because globally managed by ChanMessage()
        virtual void                    mf_system_mode(const MIDITimedMessage &msg)     {}
        virtual void                    mf_note_on(const MIDITimedMessage &msg)         {}
        virtual void                    mf_note_off(const  MIDITimedMessage &msg)       {}
        virtual void                    mf_poly_after(const MIDITimedMessage &msg)      {}
        virtual void                    mf_bender(const MIDITimedMessage &msg)          {}
        virtual void                    mf_program(const MIDITimedMessage &msg)         {}
        virtual void                    mf_chan_after(const MIDITimedMessage &msg)      {}
        virtual void                    mf_control(const MIDITimedMessage &msg)         {}

            // these implement pure virtual methods in MIDIFile EventHandler
        virtual void                    mf_sysex(MIDIClockTime time, const MIDISystemExclusive &ex);
        virtual void                    mf_arbitrary(MIDIClockTime time, int len, unsigned char *data);
        virtual void                    mf_metamisc(MIDIClockTime time, int b1, int b2, unsigned char *ch);
        virtual void                    mf_meta16(MIDIClockTime time, int type, int b1, int b2);
        virtual void                    mf_smpte(MIDIClockTime time, int h, int m, int s, int f, int sf);
        virtual void                    mf_timesig(MIDIClockTime time, int a, int b, int c, int d);
        virtual void                    mf_tempo(MIDIClockTime time, int b1, int b2, int b3);
        virtual void                    mf_keysig(MIDIClockTime time, int sf, int majmin);
        virtual void                    mf_sqspecific(MIDIClockTime time, int len, unsigned char *data);
        virtual void                    mf_text(MIDIClockTime time, int type, int len, unsigned char *data);
        virtual void                    mf_eot(MIDIClockTime time);

//
// the following methods are to be overridden for your specific purpose
//

        virtual void                    mf_error(const char* err);
        virtual void                    mf_starttrack(int trk);
        virtual void                    mf_endtrack(int trk);
        virtual void                    mf_header(int format_, int ntrks_, int division_);

//
// Higher level dispatch functions
//

        virtual void                    UpdateTime(MIDIClockTime delta_time)    {}
        virtual	void                    ChanMessage(const MIDITimedMessage &msg);

    protected:

        MIDIMultiTrack*                 multitrack;
        int                             cur_track;
        MIDIFileHeader                  header;
/// \endcond
};


/// \addtogroup GLOBALS
///@{

/// \name Functions for Loading MIDI files
///@{

/// Returns the header of the MIDI file specified by _filename_.
/// You can then inspect the format, (0, 1 or 2), the number of tracks and the division (MIDI ticks per
/// quarter note) of the file. If the header cannot be read these are all 0.
MIDIFileHeader&                         GetMIDIFileHeader(const char* filename);
/// Returns the header of the MIDI file specified by _filename_.
/// You can then inspect the format, (0, 1 or 2), the number of tracks and the division (MIDI ticks per
/// quarter note) of the file. If the header cannot be read these are all 0.
MIDIFileHeader&                         GetMIDIFileHeader(const std::string& filename);
/// Loads a MIDI file into a MIDIMultiTrack object. If the file is in SMF 0 (with only a track) splits it into
/// a master track with only system messages and 16 channel tracks (if you want to remember the original file format
/// you can set the default parameter _head_). This is the fastest way to put a file into a multitrack, without worrying
/// with intermediate reader objects.
/// \param filename the name of the file
/// \param tracks the MIDIMultiTrack to be loaded
/// \param head if you give the address of a MIDIFileHeader object this will be filled with the original file parameters
/// (format, number of tracks, division, name) which you could reuse if you want to save the file.
/// \return **true** if the loading is successful, otherwise **false**.
bool                                    LoadMIDIFile(const char* filename, MIDIMultiTrack* tracks,
                                                     MIDIFileHeader* const head = 0);
/// Loads a MIDI file into a MIDIMultiTrack object. \see LoadMIDIFile(const char*, MIDIMultiTrack, MIDIFileHeader* const).
bool                                    LoadMIDIFile(const std::string& filename, MIDIMultiTrack* tracks,
                                                     MIDIFileHeader* const head = 0);
///@}
///@}


#endif
