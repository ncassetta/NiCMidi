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
#include "notifier.h"
#include "sequencer.h"
#include "timer.h"


#include <vector>




///
/// This class manages MIDI playback, picking MIDI messages from a MIDISequencer and sending them to the open
/// MIDIDriver (and then to MIDI ports).
/// It inherits from pure virtual MIDITick, i.e. a class with a callback method TimeTick() to be called at every
/// timer tick: when the sequencer is playing the MIDIManager uses the callback for moving MIDI messages from
/// the sequencer to the driver. For effective playback you must have a MIDISequencer (holding MIDI messages),
/// a MIDIDriver (sending messages to hardware) and a MIDIManager (managing the process): the AdvancedSequencer
/// is an all-in-one class embedding all these. See example files for effective using.
///
class MIDIManager {
public:
                                MIDIManager( MIDISequencerGUINotifier *n=0,
                                             MIDISequencer *seq_=0 );

    virtual                     ~MIDIManager();

    void                        Reset();

        // to set and get the open and close ports policy
    //void                        SetOpenPolicy(int p)            { open_policy = p; }
    //int                         GetOpenpolicy() const           { return open_policy; }

        // to get the MIDI in and out ports
    static int                  GetNumMIDIOuts()                { return MIDI_out_names.size(); }
    static const std::string&   GetMIDIOutName(int n)           { return MIDI_out_names[n]; }
    static int                  GetNumMIDIIns()                 { return MIDI_in_names.size(); }
    static const std::string&   GetMIDIInName(int n)            { return MIDI_in_names[n]; }

    MIDIOutDriver*              GetOutDriver(int n)             { return MIDI_outs[n]; }
    MIDIInDriver*               GetInDriver(int n)              { return MIDI_ins[n]; }

    void                        OpenOutPorts();
    void                        CloseOutPorts();

        // to get the time in ms from the sequencer start
    tMsecs                      GetCurrentTimeInMs() const
                                                { return play_mode ?
                                                         timer->GetSysTimeMs() + seq_time_offset - sys_time_offset : 0; }

    /*
    void                        AddTickProc(MIDITick proc, unsigned int n);
    void                        RemoveTickProc(unsigned int n);
    void                        RemoveTickProc(MIDITick proc);
    */

        // to set and get the current sequencer
    void                        SetSequencer(MIDISequencer *seq);
    MIDISequencer*              GetSequencer()                  { return sequencer; }
    const MIDISequencer*        GetSequencer() const            { return sequencer; }
    void                        SetAutoSeqOpen(bool f)          { auto_seq_open = f;}
    bool                        GetAutoSeqOpen() const          { return auto_seq_open; }

        // to manage the playback of the sequencer
    void                        SeqPlay();
    void                        SeqStop();
    bool                        IsSeqPlay() const       { return play_mode; }

       // To set and get the MIDI thru
    bool                        SetThruEnable(bool f);
    bool                        GetThruEnable() const                   { return thru_enable; }
    bool                        SetThruPorts(unsigned int in_port, unsigned int out_port);
    void                        SetThruChannels(char in_chan, char out_chan);
    int                         GetThruInPort() const                   { return thru_input; }
    int                         GetThruInChannel() const                { return MIDI_ins[thru_input]->GetThruChannel(); }
    int                         GetThruOutPort() const                  { return thru_output; }
    int                         GetThruOutChannel() const               { return MIDI_outs[thru_output]->GetThruChannel(); }

    void                        SetRepeatPlay( bool on_off, unsigned int start_measure, unsigned int end_measure );
    bool                        GetRepeatPlay() const           { return repeat_play_mode; }
    int                         GetRepeatPlayStart() const      { return repeat_start_measure; }
    int                         GetRepeatPlayEnd() const        { return repeat_end_measure; }

    void                        AllNotesOff();



    /// This is the main tick procedure
    static void                 TickProc(tMsecs sys_time_, void* p);

protected:

    void                        MIDIThruProc(tMsecs sys_time_);
    void                        SequencerPlayProc(tMsecs sys_time_);

    std::vector<MIDIOutDriver*>     MIDI_outs;
    static std::vector<std::string> MIDI_out_names;
    std::vector<MIDIInDriver*>      MIDI_ins;
    static std::vector<std::string> MIDI_in_names;

    MIDISequencer*              sequencer;
    MIDISequencerGUINotifier*   notifier;

    MIDITimer*                  timer;

    tMsecs                      sys_time_offset;
    tMsecs                      seq_time_offset;

    volatile bool               play_mode;

    bool                        thru_enable;
    int                         thru_input;
    int                         thru_output;

    volatile bool               repeat_play_mode;
    long                        repeat_start_measure;
    long                        repeat_end_measure;

    bool                        auto_seq_open;
};



#endif // _JDKMIDI_MANAGER_H
