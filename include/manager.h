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
#include <thread>




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
                                MIDIManager(MIDISequencer *seq = 0, MIDISequencerGUINotifier *n = 0);

    virtual                     ~MIDIManager();

    void                        Reset();

        /// Returns the number of MIDI in out ports in the system.
    static int                  GetNumMIDIOuts()                { return MIDI_out_names.size(); }
        /// Returns the system name of the given out port.
    static const std::string&   GetMIDIOutName(int n)           { return MIDI_out_names[n]; }
        /// Returns the number of MIDI out ports in the system.
    static int                  GetNumMIDIIns()                 { return MIDI_in_names.size(); }
        /// Returns the system name of the given in port.
    static const std::string&   GetMIDIInName(int n)            { return MIDI_in_names[n]; }

        /// Returns a pointer to the MIDIOutDriver with given port id.
    MIDIOutDriver*              GetOutDriver(int n)             { return MIDI_outs[n]; }
        /// Returns a pointer to the MIDIInDriver with given port id.
    MIDIInDriver*               GetInDriver(int n)              { return MIDI_ins[n]; }

    void                        OpenOutPorts();
    void                        CloseOutPorts();

        /// Returns the elapsed time in ms from the sequencer start (0 if the sequencer is not playing).
    tMsecs                      GetCurrentTimeMs() const
                                                { return play_mode ?
                                                         timer->GetSysTimeMs() + seq_time_offset - sys_time_offset : 0; }

    /*
    void                        AddTickProc(MIDITick proc, unsigned int n);
    void                        RemoveTickProc(unsigned int n);
    void                        RemoveTickProc(MIDITick proc);
    */
        /// Sets the procedure executed when the sequencer reaches the end of MIDI data. This actually
        /// stops the playback.
    void                        SetAutoStopProc(void(proc)(void*), void* param)
                                                                { auto_stop_proc = proc; auto_stop_param = param; }

        /// Sets the pointer to the sequencer (if you hadn't already done it in the ctor, you must set it).
        /// It resets the repeat play and, if the sequencer was playing, stops it.
    void                        SetSequencer(MIDISequencer *seq);
        /// Returns the pointer to the current sequencer.
    MIDISequencer*              GetSequencer()                  { return sequencer; }
    const MIDISequencer*        GetSequencer() const            { return sequencer; }
    void                        SetAutoSeqOpen(bool f)          { auto_seq_open = f;}
    bool                        GetAutoSeqOpen() const          { return auto_seq_open; }

        /// Starts the sequencer playback.
    void                        SeqPlay();
        /// Stops the sequencer playback.
    void                        SeqStop();
        /// Returns *true* if the sequencer is playing.
    bool                        IsSeqPlay() const       { return play_mode; }

       /// Sets the MIDI thru enable on and off. For effective MIDI thru you must have already
       /// set in and out thru ports (with SetThruPorts()) otherwise the method will fail and return
       /// *false*.
    bool                        SetThruEnable(bool f);
        /// Returns the MIDI thru enable status.
    bool                        GetThruEnable() const                   { return thru_enable; }
        /// Sets the MIDI thru in and out ports. When MIDI thru is enabled (by SetThruEnable()) MIDI
        /// messages incoming from the in port will be repeated on the out port. When the MIDIManager
        /// is created, if at least one in and out MIDI ports exist on the system, these are set to 0, 0
        /// ports, otherwise these are left undefined and you won't be able to enable MIDI thru.
    bool                        SetThruPorts(unsigned int in_port, unsigned int out_port);
        /// Sets the MIDI thru in and out channels (see MIDIInDriver::SetThruChannel() and
        /// MIDIOutDriver::SetThruChannel()).
    void                        SetThruChannels(char in_chan, char out_chan);
        /// Returns the MIDI thru in port, that is the port from which thru messages are received.
    int                         GetThruInPort() const                   { return thru_input; }
        /// Returns the MIDI thru in channel (see MIDIInDriver::SetThruChannel()).
    int                         GetThruInChannel() const                { return MIDI_ins[thru_input]->GetThruChannel(); }
        /// Returns the MIDI thru out port, that is the port to whom thru messages are sent.
    int                         GetThruOutPort() const                  { return thru_output; }
        /// Returns the MIDI thru out channel (see MIDIOutDriver::SetThruChannel()).
    int                         GetThruOutChannel() const               { return MIDI_outs[thru_output]->GetThruChannel(); }
        /// Turns on and off the repeat play (loop) mode of the sequencer
    void                        SetRepeatPlay( bool on_off, unsigned int start_measure, unsigned int end_measure );
        /// Returns the repeat play (loop) status.
    bool                        GetRepeatPlay() const           { return repeat_play_mode; }
        /// Returns the repeat play (loop) start measure.
    int                         GetRepeatPlayStart() const      { return repeat_start_measure; }
        /// Returns the repeat play (loop) end measure.
    int                         GetRepeatPlayEnd() const        { return repeat_end_measure; }

    void                        AllNotesOff();


protected:

        /// This is the main tick procedure. This actually only calls the (non-static) SequencerPlayProc()
    static void                 TickProc(tMsecs sys_time_, void* p);

    // void                        MIDIThruProc(tMsecs sys_time_); TODO: unneeded????
        /// This is the sequencer play procedure. It examines events at current time and sends them to the
        /// MIDI out ports, manages repeat play (loop) and calls AutoStopProc() when the sequencer reaches
        /// the end of MIDI events.
    void                        SequencerPlayProc(tMsecs sys_time_);
        /// This is called by SequencerPlayProc() when the sequencer reaches the end of MIDI events. The default
        /// procedure only calls SeqStop().
    static void                 AutoStopProc(void* p);

    std::vector<MIDIOutDriver*>     MIDI_outs;      ///< A vector of MIDIOutDriver classes (one for each hardware port)
    static std::vector<std::string> MIDI_out_names; ///< The system names of hardware out ports
    std::vector<MIDIInDriver*>      MIDI_ins;       ///< A vector of MIDIInDriver classes (one for each hardware port)
    static std::vector<std::string> MIDI_in_names;  ///< The system names of hardware in ports

    MIDISequencer*              sequencer;          ///< The MIDISequencer
    MIDISequencerGUINotifier*   notifier;           ///< The MIDISequencerGUINotifier

    MIDITimer*                  timer;              ///< The timer which temporizes MIDI playback

    tMsecs                      sys_time_offset;    ///< The time between now and timer start
    tMsecs                      seq_time_offset;    ///< The time between the sequencer start and the timer start

    volatile bool               play_mode;          ///< True if the sequencer is playing

    bool                        thru_enable;        ///< Enables the MIDI thru
    int                         thru_input;         ///< The id of the MIDI thru in port
    int                         thru_output;        ///< The id of the MIDI thru out port

    volatile bool               repeat_play_mode;   ///< Enables the repeat play mode
    long                        repeat_start_measure;
                                                    ///< The loop start measure
    long                        repeat_end_measure; ///< The loop end measure

    bool                        auto_seq_open;

    void                        (*auto_stop_proc)(void *);
                                                    ///< The auto stop procedure
    void*                       auto_stop_param;    ///< The parameter given to the auto stop procedure
};



#endif // _JDKMIDI_MANAGER_H
