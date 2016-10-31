#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <chrono>
#include <thread>
#include <atomic>

#include <iostream>     // for debug


/// The type of a variable which can hold the elapsed time in milliseconds.
typedef unsigned long long tMsecs;
/// This is the typedef of the callback function which is called at every timer tick.
typedef  void (*MIDITick)(tMsecs, void*);

///
/// The MIDITimer class provides the timing required for MIDI playback. It implements
/// a clock which can call a user-defined callback function at a regular pace; when you start
/// the timer, a background thread is created for this task. You can set the timer resolution
/// in milliseconds (default is DEFAULT_RESOLUTION) and the  callback function.
/// Moreover, it provides some other timing utility as static functions: you can stop a thread
/// for a given number of milliseconds and can get the system time elapsed from the application
/// start.
/// If you only want to play MIDI files (i.e. use a MIDISequencer) you don't need to deal with
/// this directly, but you can use the MIDIManager class (which creates and properly handles a
/// MIDITimer by itself).
///
class MIDITimer {
    public:

        typedef std::chrono::steady_clock::time_point timepoint;
		typedef std::chrono::milliseconds duration;

        /// The constructor creates a timer with the given resolution (time interval between
        /// two ticks). The timer is initially off and you must set the callback function before
        /// starting it (otherwise the timer will do nothing).
                                    MIDITimer(unsigned int res = DEFAULT_RESOLUTION);

        /// Returns the timer resolution, i.e. the time interval (in milliseconds) between two ticks.
        int                         GetResolution() const           { return resolution; }
        /// Sets the timer resolution to the given value in milliseconds. This method stops the timer
        /// if it is running.
        void                        SetResolution(unsigned int res);

        /// Returns the pointer to the callback function.
        MIDITick                    GetMIDITick() const             { return tick; }

        /// Sets the callback function to be called at every timer tick and its parameter.
        /// The function must be of MIDITick type (i.e. void Funct(tMsecs, void*) ) and it's called
        /// with the system time as first parameter and the given void pointer as second. This
        /// method stops the timer if it is running.
        void                        SetMIDITick(MIDITick t, void* tp = 0);

        /// Returns *true* if the timer is running
        bool                        IsOpen() const                  { return timer_on;  }

        /// Starts the background thread procedure which calls the callback function at every
        /// timer tick.
        bool                        Start();
        /// Stops the timer, joining the background thread procedure.
        void                        Stop();

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
        /// The background thread procedure which calls the callback function
        static void                 ThreadProc( MIDITimer* timer );

        unsigned int                resolution;         ///< The timer resolution
        MIDITick                    tick;               ///< The callback function
        void*                       tick_param;         ///< The callback second parameter
        std::thread                 bg_thread;          ///< The background thread
        std::atomic<bool>           timer_on;           ///< True if the timer is running
        static const timepoint      sys_clock_base;     ///< The base timepoint for calculating system time
};


#endif // TIMER_H_INCLUDED
