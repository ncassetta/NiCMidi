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
** CHECKED with jdksmidi. NO CHANGES
*/

#ifndef _JDKMIDI_FILEREADMULTITRACK_H
#define _JDKMIDI_FILEREADMULTITRACK_H

#include "midi.h"
#include "msg.h"
#include "sysex.h"
#include "fileread.h"
#include "multitrack.h"

#include <string>


class MIDIFileReadMultiTrack : public MIDIFileEventHandler {
    public:
                                        MIDIFileReadMultiTrack (MIDIMultiTrack *tracks);
        virtual                         ~MIDIFileReadMultiTrack();


//
// The possible events in a MIDI Files
//

        virtual void                    mf_sysex( MIDIClockTime time, const MIDISystemExclusive &ex );
        virtual void                    mf_arbitrary( MIDIClockTime time, int len, unsigned char *data );
        virtual void                    mf_metamisc( MIDIClockTime time, int, int, unsigned char * );
        virtual void                    mf_meta16( MIDIClockTime time, int type, int b1, int b2 );
        virtual void                    mf_smpte( MIDIClockTime time, int h, int m, int s, int f, int sf );
        virtual void                    mf_timesig( MIDIClockTime time, int a, int b, int c, int d );
        virtual void                    mf_tempo( MIDIClockTime time, int b1, int b2, int b3 );
        virtual void                    mf_keysig(MIDIClockTime time, int sf, int majmin );
        virtual void                    mf_sqspecific( MIDIClockTime time, int len, unsigned char *data );
        virtual void                    mf_text( MIDIClockTime time, int type, int len, unsigned char *data );
        virtual void                    mf_eot( MIDIClockTime time );

//
// the following methods are to be overridden for your specific purpose
//

        virtual void                    mf_error( char* err );
        virtual void                    mf_starttrack( int trk );
        virtual void                    mf_endtrack( int trk );
        virtual void                    mf_header( int format_, int ntrks_, int division_ );

//
// Higher level dispatch functions
//

        virtual	void                    ChanMessage( const MIDITimedMessage &msg );

    protected:

        MIDIMultiTrack*                 multitrack;
        int                             cur_track;
        MIDIFileHeader                  header;
};


/// Returns the header of the MIDI file specified by *filename*. You can then inspect the format, (0, 1 or 2),
/// the number of tracks and the division (MIDI ticks per quarter note) of the file. If the header cannot be
/// read these are all 0.
MIDIFileHeader                          GetMIDIFileHeader(const char* filename);

/// Loads the MIDI file specified by *filename* into the given MIDIMultiTrack object. This is the fastest
/// way to get this, without worrying with intermediate reader objects. Returns *false* if the loading is
/// failed.
bool                                    LoadMIDIFile(const char* filename, MIDIMultiTrack* tracks);

bool                                    LoadMIDIFile(const std::string& filename, MIDIMultiTrack* tracks);
//                                            { return LoadMIDIFile (filename.c_str(), tracks); }
#endif
