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
#include "timer.h"


#include <vector>


class MIDIManager {
public:
                                MIDIManager( MIDISequencerGUINotifier *n=0,
                                             MIDISequencer *seq_=0 );

    virtual                     ~MIDIManager();

    void                        Reset();

        // to set and get the current sequencer
    void                        SetSeq( MIDISequencer *seq );
    MIDISequencer*              GetSeq()                        { return sequencer; }
    const MIDISequencer*        GetSeq() const                  { return sequencer; }

        // to setand get the open and close ports policy
    void                        SetOpenPolicy(int p)            { open_policy = p; }
    int                         GetOpenpolicy() const           { return open_policy; }

        // to get the MIDI in and out ports
    static int                  GetNumMIDIOuts()                { return MIDI_out_names.size(); }
    static const std::string&   GetMIDIOutName(int n)           { return MIDI_out_names[n]; }
    static int                  GetNumMIDIIns()                 { return 0; }
    static const std::string&   GetMIDIInName(int n)            { return std::string("For now no inputs!"); }
    MIDIOutDriver*              GetDriver(int n)                { return MIDI_outs[n]; }
    void                        OpenOutPorts();
    void                        CloseOutPorts();

        // to get the time in ms from the sequencer start
    tMsecs                      GetCurrentTimeInMs() const
                                                { return play_mode ?
                                                         timer->GetSysTimeMs() + seq_time_offset - sys_time_offset : 0; }

/* THESE ARE MANAGED INTERNALLY. NO USER UTILITY
        // to set and get the system time offset
    void                        SetTimeOffset( unsigned long off )
                                                                { sys_time_offset = off; }
    tMsecs                      GetTimeOffset()                 { return sys_time_offset; }

        // to set and get the sequencer time offset
    void                        SetSeqOffset( unsigned long seqoff )
                                                        { seq_time_offset = seqoff; }
    unsigned long               GetSeqOffset()          { return seq_time_offset; }
*/

        // to manage the playback of the sequencer
    void                        SeqPlay();
    void                        SeqStop();
    void                        SetRepeatPlay( bool flag, unsigned long start_measure, unsigned long end_measure );

       // To set and get the MIDI thru
    void                        SetThruEnable( bool f )                 { thru_enable = f; }
    bool                        GetThruEnable() const                   { return thru_enable; }

    void                        AllNotesOff();

        // status request functions
    bool                        IsSeqPlay() const       { return play_mode; }
    bool                        IsSeqStop() const       { return stop_mode; }
    bool                        IsSeqRepeat() const     { return repeat_play_mode && play_mode; }

    static void                 TickProc(unsigned long sys_time, void* p);

    enum { AUTO_OPEN, EXT_OPEN };

protected:

    void                        TimeTickPlayMode( unsigned long sys_time_ );
    void                        TimeTickStopMode( unsigned long sys_time_ );

    std::vector<MIDIOutDriver*> MIDI_outs;
    static std::vector<std::string> MIDI_out_names;
    MIDISequencer*              sequencer;
    MIDISequencerGUINotifier*   notifier;

    MIDITimer*                  timer;

    tMsecs                      sys_time_offset;
    tMsecs                      seq_time_offset;

    volatile bool               play_mode;  // TODO: why two members? Could we eliminate stop_mode and TimeTickStopMode() ?
    volatile bool               stop_mode;
    bool                        thru_enable;

    volatile bool               repeat_play_mode;
    long                        repeat_start_measure;
    long                        repeat_end_measure;

    int                         open_policy;
};



#endif // _JDKMIDI_MANAGER_H
