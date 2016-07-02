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



class MessageQueue {
    public:
                                        MessageQueue(unsigned int size = DEFAULT_QUEUE_SIZE);
        virtual                         ~MessageQueue()             {}
        void                            Reset();
        void                            PutMessage(const MIDITimedMessage& msg);
        MIDITimedMessage                GetMessage();
        bool                            IsEmpty() const             { return next_in == next_out; }
        bool                            IsFull() const              { return ((next_in + 1) % buffer.size()) == next_out; }

        static const unsigned int       DEFAULT_QUEUE_SIZE = 256;

    protected:

        unsigned int                    next_in;
        unsigned int                    next_out;
        std::vector<MIDITimedMessage>   buffer;
};



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

        // to set and get the current sequencer
    void                        SetSeq( MIDISequencer *seq );
    MIDISequencer*              GetSeq()                        { return sequencer; }
    const MIDISequencer*        GetSeq() const                  { return sequencer; }

        // to set and get the open and close ports policy
    void                        SetOpenPolicy(int p)            { open_policy = p; }
    int                         GetOpenpolicy() const           { return open_policy; }

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

    void                        AddTickProc(MIDITick proc, unsigned int n);
    void                        RemoveTickProc(unsigned int n);
    void                        RemoveTickProc(MIDITick proc);

        // to manage the playback of the sequencer
    void                        SeqPlay();
    void                        SeqStop();
    void                        SetRepeatPlay( bool on_off, unsigned int start_measure, unsigned int end_measure );

       // To set and get the MIDI thru
    void                        SetThruEnable(bool f)                   {}
    bool                        GetThruEnable() const                   { return thru_enable; }

    void                        AllNotesOff();

        // status request functions
    bool                        IsSeqPlay() const       { return play_mode; }
    //bool                        IsSeqStop() const       { return stop_mode; }
    bool                        IsSeqRepeat() const     { return repeat_play_mode && play_mode; }

    /// This is the main tick procedure
    static void                 TickProc(tMsecs sys_time_, void* p);

    enum { AUTO_OPEN, EXT_OPEN };

protected:

    void                        MIDIThruProc(tMsecs sys_time_);
    void                        SequencerPlayProc(tMsecs sys_time_);

    std::vector<MIDIOutDriver*> MIDI_outs;
    static std::vector<std::string> MIDI_out_names;
    std::vector<MIDIInDriver*>  MIDI_ins;
    static std::vector<std::string> MIDI_in_names;

    MIDISequencer*              sequencer;
    MIDISequencerGUINotifier*   notifier;

    MIDITimer*                  timer;

    tMsecs                      sys_time_offset;
    tMsecs                      seq_time_offset;

    volatile bool               play_mode;
    //volatile bool               stop_mode;

    bool                        thru_enable;
    unsigned int                thru_input;
    int                         thru_input_channel;
    unsigned int                thru_output;
    int                         thru_output_channel;

    volatile bool               repeat_play_mode;
    long                        repeat_start_measure;
    long                        repeat_end_measure;

    int                         open_policy;
};



#endif // _JDKMIDI_MANAGER_H
