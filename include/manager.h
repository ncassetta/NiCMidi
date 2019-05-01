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


/// \file
/// Contains the definition of the static class MIDIManager.


#ifndef _JDKMIDI_MANAGER_H
#define _JDKMIDI_MANAGER_H

#include "msg.h"
#include "driver.h"
#include "notifier.h"
#include "sequencer.h"
#include "timer.h"
#include "tick.h"


#include <vector>
#include <thread>
#include <mutex>


///
/// A static class that manages computer hardware resources (in and out MIDI ports) and timing.
/// It embeds a MIDITimer object and a MIDIInDriver or MIDIOutDriver for every hardware port;
/// moreover it manages a queue of MIDITickComponent (objects with a callback procedure to be called at regular
/// pace). When the timer is started the MIDIManager begins to call its main tick procedure at every timer tick;
/// this in turn calls the callback procedure of all the MIDITickComponent objects in the queue; typically
/// one of the MIDITickComponent is a sequencer, which can in this way pick the MIDI messages stored in its
/// internal MIDIMultiTrack object and send them to the out ports. You can add and remove other MIDITickComponent
/// objects with the AddMIDITick() and RemoveMIDITick() methods (for example you could add a MIDIThru or a
/// MIDIRecorder).
///
class MIDIManager {
public:
    /// The constructor. It creates
    /// - a MIDIOutDriver for every hardware out port
    /// - a MIDIInDriver for every hardware in port.
    /// - a MIDITimer for playback timing.
    /// - an empty queue of MIDITickComponent objects.
                                MIDIManager();
    /// The destructor deletes the drivers and the timer.
    virtual                     ~MIDIManager();
    /// Stops the timer if it is running and resets all the MIDI in and out ports.
    /// Doesn't reset the MIDITickComponent queue.
    static void                 Reset();

    /// Returns the number of MIDI out ports in the system.
    static unsigned int         GetNumMIDIOuts()                { return MIDI_out_names.size(); }
    /// Returns the system name of the given MIDI out port.
    static const std::string&   GetMIDIOutName(unsigned int n)  { return MIDI_out_names[n]; }
    /// Returns the number of MIDI in ports in the system.
    static unsigned int         GetNumMIDIIns()                 { return MIDI_in_names.size(); }
    /// Returns the system name of the given MIDI in port.
    static const std::string&   GetMIDIInName(unsigned int n)   { return MIDI_in_names[n]; }
    /// Returns a pointer to the MIDIOutDriver with given port id.
    static MIDIOutDriver*       GetOutDriver(unsigned int n)    { return MIDI_outs[n]; }
    /// Returns a pointer to the MIDIInDriver with given port id.
    static MIDIInDriver*        GetInDriver(unsigned int n)     { return MIDI_ins[n]; }
    /// Returns the pointer to the (unique) MIDITickComponent in the queue with tPriority PR_SEQ
    /// (0 if not found).
    static MIDISequencer*       GetSequencer();
    /// Starts the MIDITimer thread procedure.
    /// It calls, at every timer tick, the TickProc() which in turn calls all the MIDITickComponent::StaticTickProc()
    /// added to the queue via the AddMIDITick() method.
    static bool                 StartTimer()                    { return MIDITimer::Start(); }
    /// Stops the MIDITimer thread procedure.
    /// This causes all the MIDITickComponent callbacks to halt.
    static void                 StopTimer()                     { MIDITimer::Stop(); }
    /// Opens all the system MIDI In ports.
    /// It calls the MIDIInDriver::OpenPort() method for every port.
    static void                 OpenInPorts();
    /// Closes all the system MIDI In ports.
    /// It calls the MIDIInDriver::ClosePort() method for every port.
    static void                 CloseInPorts();
    /// Opens all the system MIDI Out ports.
    /// It calls the MIDIOutDriver::OpenPort() method for every port.
    static void                 OpenOutPorts();
    /// Closes all the system MIDI Out ports.
    /// It calls the MIDIOutDriver::OpenPort() method for every port.
    static void                 CloseOutPorts();

    /* Now in MIDISequencer
    /// Returns the elapsed time in ms from the sequencer start (0 if the sequencer is not playing).
    static tMsecs               GetCurrentTimeMs()
                                                { return play_mode ?

                                                         MIDITimer::GetSysTimeMs() + seq_time_offset - sys_time_offset : 0; }
    */
    /*
    void                        AddTickProc(MIDITick proc, unsigned int n);
    void                        RemoveTickProc(unsigned int n);
    void                        RemoveTickProc(MIDITick proc);

    /// Sets the procedure executed when the sequencer reaches the end of MIDI data. This actually
    /// stops the playback.
    static void                 SetAutoStopProc(void(proc)(void*), void* param)
                                                                { auto_stop_proc = proc; auto_stop_param = param; }
*/
/*
    /// Sets the sequencer pointer (if you hadn't already done it in the ctor, you must set it).
    /// It resets the repeat play and, if the sequencer was playing, stops it.
    static void                 SetSequencer(MIDISequencer *seq);
*/

   /*
    /// Starts the sequencer playback.
    static void                 SeqPlay();
    /// Stops the sequencer playback.
    static void                 SeqStop();
    /// Returns *true* if the sequencer is playing.
    static bool                 IsSeqPlay()                    { return sequencer->IsPlaying(); }
*/


    /// Sends a MIDI AllNotesOff message to all open out ports.
    static void                 AllNotesOff();
    /// Inserts a MIDITickComponent object into the queue. The objects are queued according to their
    /// \ref tPriority parameter; you can add only one of them with \ref PR_SEQ priority (i.e.\ a
    /// sequencer). Advanced classes (as AdvancedSequencer) auto add themselves to the manager queue
    /// when they are created, so they are ready to play. For other MIDITickComponent derived classes
    /// you must do this by this method if you want their callback become effective.
    static void                 AddMIDITick(MIDITickComponent *tick);
    /// Removes the given MIDITickComponent pointer from the queue. It does nothing if the pointer is
    /// not in the queue. The destructor of a MIDITickComponent call this before destroying the object,
    /// preventing the manager from using an invalid pointer, so usually you don't need to call this.
    static bool                 RemoveMIDITick(MIDITickComponent *tick);
    /// Removes all MIDITickComponent objects from the queue. Be careful with this, because some
    /// advanced class (as AdvancedSequencer) auto adds itself to the queue: if you remove it
    /// you must to re-add it manually with the AddMIDITick() if you want to continue to use it.

protected:

    /// This is the main tick procedure. This calls the TickProc() method of every queued MIDITickComponent
    /// object with running status. The user must not call it directly.
    static void                 TickProc(tMsecs sys_time_, void* p);

/*
    // void
                          MIDIThruProc(tMsecs sys_time_); TODO: unneeded????
    /// This is the sequencer play procedure. It examines events at current time and sends them to the
    /// MIDI out ports, manages repeat play (loop) and calls AutoStopProc() when the sequencer reaches
    /// the end of MIDI events.
    void                        SequencerPlayProc(tMsecs sys_time_);
*/


    static std::vector<MIDIOutDriver*>  MIDI_outs;      ///< A vector of MIDIOutDriver classes (one for each hardware port)
    static std::vector<std::string>     MIDI_out_names; ///< The system names of hardware out ports
    static std::vector<MIDIInDriver*>   MIDI_ins;       ///< A vector of MIDIInDriver classes (one for each hardware port)
    static std::vector<std::string>     MIDI_in_names;  ///< The system names of hardware in ports

    static std::vector<MIDITickComponent*>
                                        MIDITicks;      ///< The array of MIDITICK objects, everyone of them has his MIDITick callback

    //static MIDISequencer*               sequencer;      ///< The MIDISequencer

    static std::mutex                   proc_lock;      ///< A mutex for thread safe

/* OLD!!!! Delete if all OK
    /// This is called by SequencerPlayProc() when the sequencer reaches the end of MIDI events. The default
    /// procedure only calls SeqStop().
    //static void                 AutoStopProc(void* p);
    //static void                 Notify(const MIDISequencerGUIEvent& ev);
    static MIDITimer*           timer;              ///< The timer which temporizes MIDI playback
    static std::atomic<bool>    play_mode;          ///< True if the sequencer is playing
    static bool                 auto_seq_open;
    static void                 (*auto_stop_proc)(void *);
                                                    ///< The auto stop procedure
    static void*                auto_stop_param;    ///< The parameter given to the auto stop procedure
    std::atomic<int>            times;
*/

};



#endif // _JDKMIDI_MANAGER_H
