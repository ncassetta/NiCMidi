/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
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
/// Contains the definition of the static class MIDIManager.


#ifndef _JDKMIDI_MANAGER_H
#define _JDKMIDI_MANAGER_H

#include "msg.h"
#include "driver.h"
#include "notifier.h"
#include "timer.h"
#include "tick.h"


#include <vector>
#include <thread>
#include <mutex>


///
/// A static class that manages computer hardware resources (in and out MIDI ports) and timing.
/// It controls the MIDITimer object and embeds a MIDIInDriver or MIDIOutDriver for every hardware port;
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
                                MIDIManager() = delete;

    /// Stops the timer if it is running, resets all the MIDI in and out ports and flushes the
    /// MIDITickComponent queue.
    static void                 Reset();

    /// Returns the number of MIDI in ports in the system.
    static unsigned int         GetNumMIDIIns();
    /// Returns the system name of the given MIDI in port.
    static const std::string&   GetMIDIInName(unsigned int n);
    /// Returns a pointer to the MIDIInDriver with given port id.
    static MIDIInDriver*        GetInDriver(unsigned int n);
    /// Returns **true** if n is a valid MIDI in port number. If you call this with 0 as argument
    /// and it returns **false** no MIDI in port is present in the system.
    static bool                 IsValidInPortNumber(unsigned int n);
    /// Returns the number of MIDI out ports in the system.
    static unsigned int         GetNumMIDIOuts();
    /// Returns the system name of the given MIDI out port.
    static const std::string&   GetMIDIOutName(unsigned int n);
    /// Returns a pointer to the MIDIOutDriver with given port id.
    static MIDIOutDriver*       GetOutDriver(unsigned int n);
    /// Returns **true** if n is a valid MIDI out port number. If you call this with 0 as argument
    /// and it returns **false** no MIDI out port is present in the system.
    static bool                 IsValidOutPortNumber(unsigned int n);
    /// Returns the pointer to the (unique) MIDITickComponent in the queue with tPriority PR_SEQ
    /// (0 if not found).
    static MIDISequencer*       GetSequencer();

/* TODO: are these useful?
    /// Starts the MIDITimer thread procedure.
    /// It calls, at every timer tick, the TickProc() which in turn calls all the
    /// MIDITickComponent::StaticTickProc() added to the queue via the AddMIDITick() method.
    static bool                 StartTimer();
    /// Stops the MIDITimer thread procedure.
    /// This causes all the MIDITickComponent callbacks to halt.
    static void                 StopTimer();
*/
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

protected:

    /// This is the main callback, called at every tick of the MIDITimer. It calls in turn the StaticTickProc()
    /// method of every queued MIDITickComponent object with running status. The user must not call it directly.
    static void                         TickProc(tMsecs sys_time_, void* p);

    /// This is the initialization function, called the first time a class method is accessed. It creates
    /// - a MIDIOutDriver for every hardware out port
    /// - a MIDIInDriver for every hardware in port.
    /// - an empty queue of MIDITickComponent objects.
    /// Moreover, it redirects the MIDITimer callback pointer to TickProc(), so StartTimer() and StopTimer()
    /// start and stop the callback.
    static void                         Init();

    /// \cond EXCLUDED
    static void                         Exit();         // called at exit

    // WARNING! We MUST use pointers to avoid the "static inizialization order fiasco"
    static std::vector<MIDIOutDriver*>* MIDI_outs;      // A vector of MIDIOutDriver objects (one for each
                                                        // hardware port)
    static std::vector<std::string>*    MIDI_out_names; // The system names of hardware out ports
    static std::vector<MIDIInDriver*>*  MIDI_ins;       // A vector of MIDIInDriver objects (one for each
                                                        // hardware port)
    static std::vector<std::string>*    MIDI_in_names;  // The system names of hardware in ports

    static std::vector<MIDITickComponent*>*
                                        MIDITicks;      // The array of MIDITickCompnent objects, everyone
                                                        // of them has his StaticTickProc() callback

    static std::mutex*                  proc_lock;      // A mutex for thread safe processing
    static bool                         init;
    /// \endcond
};



#endif // _JDKMIDI_MANAGER_H
