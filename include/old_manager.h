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

// updated to reflect changes in jdksmidi

#ifndef _JDKMIDI_MANAGER_H
#define _JDKMIDI_MANAGER_H

#include "msg.h"
#include "driver.h"
#include "sequencer.h"
#include "tick.h"


#ifdef _WIN32
#include <windows.h>

inline unsigned long jdks_get_system_time_ms()
{
    return timeGetTime();
}

#elif __linux__
#include "time.h"

inline unsigned long jdks_get_system_time_ms()
{
    static struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    return (unsigned long)tspec.tv_sec + (unsigned long)(tspec.tv_nsec / 1000000);
}

#else

inline unsigned long jdks_get_system_time_ms()
{
    return 0;
}

#endif // _WIN32


class MIDIManager : public MIDITick {
public:
                                MIDIManager( MIDIDriver *drv, MIDISequencerGUIEventNotifier *n=0,
                                             MIDISequencer *seq_=0 );

    virtual                     ~MIDIManager()          {}

    void                        Reset();

        // to set and get the current sequencer
    void                        SetSeq( MIDISequencer *seq );
    MIDISequencer*              GetSeq()                { return sequencer; }
    const MIDISequencer*        GetSeq() const          { return sequencer; }

        // to get the driver that we use
    MIDIDriver*                 GetDriver()             { return driver; }

        // to set and get the system time offset
    void                        SetTimeOffset( unsigned long off )
                                                        { sys_time_offset = off; }
    unsigned long               GetTimeOffset()         { return sys_time_offset; }

        // to get the time in ms from the sequencer start
    unsigned long GetCurrentTimeInMs() const {
        if ( play_mode )
            return jdks_get_system_time_ms() + seq_time_offset - sys_time_offset;
        else
            return 0;
    }

        // to set and get the sequencer time offset
    void                        SetSeqOffset( unsigned long seqoff )
                                                        { seq_time_offset = seqoff; }
    unsigned long               GetSeqOffset()          { return seq_time_offset; }

        // to manage the playback of the sequencer
    void                        SeqPlay();
    void                        SeqStop();
    void                        SetRepeatPlay( bool flag, unsigned long start_measure, unsigned long end_measure );

        // status request functions
    bool                        IsSeqPlay() const       { return play_mode; }
    bool                        IsSeqStop() const       { return stop_mode; }
    bool                        IsSeqRepeat() const     { return repeat_play_mode && play_mode; }

        // inherited from MIDITick
    virtual void                TimeTick( unsigned long sys_time );

protected:

    virtual void                TimeTickPlayMode( unsigned long sys_time_ );
    virtual void                TimeTickStopMode( unsigned long sys_time_ );

    MIDIDriver*                 driver;
    MIDISequencer*              sequencer;

    unsigned long               sys_time_offset;
    unsigned long               seq_time_offset;

    volatile bool               play_mode;
    volatile bool               stop_mode;

    MIDISequencerGUIEventNotifier *notifier;

    volatile bool               repeat_play_mode;
    long                        repeat_start_measure;
    long                        repeat_end_measure;
};



#endif // _JDKMIDI_MANAGER_H
