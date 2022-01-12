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
/// Contains the definition of the MIDITimer class and some other typedef related to MIDI timing.


#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <chrono>
#include <thread>
#include <atomic>




/// \addtogroup GLOBALS
///@{

/// The type of a variable which can hold the elapsed time in milliseconds.
typedef unsigned long long tMsecs;
/// This is the typedef of the callback function which is called at every timer tick.
typedef  void (MIDITick)(tMsecs, void*);
///@}


///
/// A static class which provides the timing required for MIDI playback, using the C++11 &lt;chrono&gt;
/// methods. It implements a timer which can call a user-defined callback function at a regular pace;
/// when the timer is started, a background thread is created for this task. You can set the timer
/// resolution in milliseconds (default is \ref DEFAULT_RESOLUTION) and the callback function.
/// Moreover, it provides some other timing utilities as static functions: you can stop a thread
/// for a given number of milliseconds and get the system time elapsed from the application
/// start.
/// The MIDIManager class embeds the MIDITimer, controlling its start and stop, so you probably
/// won't have to deal with it.
///
class MIDITimer {
    public:

        /// Type for a variable which can hold a specific time point (internal use).
        typedef std::chrono::steady_clock::time_point timepoint;
        /// Type for a variable which can hold a time duration (in milliseconds).
		typedef std::chrono::milliseconds duration;
        /// The constructor is deleted.
                                    MIDITimer() = delete;
        /// Returns the timer resolution, i.e. the time interval (in milliseconds) between two ticks.
        static unsigned int         GetResolution()                 { return resolution; }
        /// Returns the pointer to the callback function set by the user.
        static MIDITick*            GetMIDITick()                   { return tick_proc; }
        /// Returns **true** if the timer is running
        static bool                 IsOpen()                        { return (num_open > 0);  }

        /// Sets the timer resolution to the given value in milliseconds. This method stops the timer
        /// if it is running.
        static void                 SetResolution(unsigned int res);
        /// Sets the callback function to be called at every timer tick and its parameter.
        /// The function must be of MIDITick type (i.e. void Funct(tMsecs, void*) ) and it's called
        /// with the system time as first parameter and the given void pointer as second. This
        /// method stops the timer if it is running.
        static void                 SetMIDITick(MIDITick* t, void* tp = 0);

        /// Starts the background thread procedure which calls the callback function at every
        /// timer tick. If you call this more than once you must call Stop() an equal number of times
        /// (or call HardStop()) to interrupt the background thread.
        static bool                 Start();
        /// Stops the timer, joining the background thread procedure. If Start() was called more than once
        /// it only decrements the count of calls.
        static void                 Stop();
        /// Stops the timer, joining the background thread procedure, regardless the number of times
        /// Start() was called.
        static void                 HardStop();

        /// Returns the elapsed time in milliseconds since the start of application. The 0 time is
        /// a chrono::steady_clock::timepoint static variable.
        static tMsecs               GetSysTimeMs()
                                        { return std::chrono::duration_cast<std::chrono::milliseconds>
                                                 (std::chrono::steady_clock::now() - sys_clock_base).count(); }
        /// Stops the calling thread for the given number of milliseconds. Other threads continue their
        /// execution.
        static void                 Wait(unsigned int msecs)
                                        { std::this_thread::sleep_for(std::chrono::milliseconds(msecs)); }

    protected:

        static const unsigned int   DEFAULT_RESOLUTION = 10;
                                                        ///< The default timer resolution
        /// The background thread procedure. This calls the tick_proc callback supplied by the user and sleeps
        /// until next tick.
        static void                 ThreadProc();

        /// \cond EXCLUDED
        static unsigned int         resolution;         // The actual timer resolution
        static MIDITick*            tick_proc;          // The callback function set by the user
        static void*                tick_param;         // The callback second parameter set by the user
        static std::thread          bg_thread;          // The background thread
        static std::atomic<int>     num_open;           // The number of times Start() was called without a corresponding Stop()
        static const timepoint      sys_clock_base;     // The base timepoint for calculating system time
        static timepoint            current;            // Internal use
        /// \endcond
};

//extern MIDITimer main_timer;

#endif // TIMER_H_INCLUDED
