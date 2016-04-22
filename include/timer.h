#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <chrono>
#include <thread>
#include <atomic>

#include <iostream>     // for debug

using namespace std::chrono;

typedef unsigned long tMsecs;
typedef  void (*MIDITick)(tMsecs, void*);


void Wait(unsigned int msecs);


class MIDITimer {
    public:

        typedef std::chrono::steady_clock::time_point timepoint;
		typedef std::chrono::milliseconds duration;

                                    MIDITimer(int res = DEFAULT_RESOLUTION);

        int                         GetResolution() const           { return resolution; }
        void                        SetResolution(int res);

        MIDITick                    GetMIDITick() const             { return tick; }
        void                        SetMIDITick(MIDITick t, void* tp);

        bool                        IsOpen() const                  { return timer_on;  }

        tMsecs                      GetSysTimeMs() const
                                        { return duration_cast<milliseconds>(steady_clock::now() - sys_clock_base).count(); }

        bool                        Start();
        void                        Stop();

    protected:

        static const unsigned int   DEFAULT_RESOLUTION = 10;

        static void                 ThreadProc( MIDITimer* timer );

        int                         resolution;
        MIDITick                    tick;
        void*                       tick_param;
        std::thread                 bg_thread;
        std::atomic<bool>           timer_on;
        time_point<steady_clock>    sys_clock_base;
};




/*
	This is part of CFugue, a C++ Runtime for MIDI Score Programming
	Copyright (C) 2009 Gopalakrishna Palem

	For links to further information, or to contact the author,
	see <http://cfugue.sourceforge.net/>.

    $LastChangedDate: 2014-05-10 10:56:34 +0530 (Sat, 10 May 2014) $
    $Rev: 203 $
    $LastChangedBy: krishnapg $
*/

/*

	///<Summary>Plays the role of a pseudo MIDI Timer for MIDI Sequencer</Summary>
	class MidiTimer
	{
	public:
		typedef std::chrono::steady_clock::time_point TimePoint;
		typedef std::chrono::milliseconds Duration;

		///<Summary>
		/// Returns current time point for MIDI Sequencing.
		/// It is usually measure as the time elapsed since epoch.
		/// Taking the difference of two consequent calls of this gives the elapsed time for MIDI.
		/// @return psuedo time tick offset that is suitable for MIDI sequencer
		///</Summary>
		static TimePoint Now()
		{
			return std::chrono::steady_clock::now();
		}

		///<Summary>
		/// Causes the calling thread to sleep
		/// @param ms sleep duration (in milli-seconds)
		///</Summary>
		static void Sleep(unsigned long ms)
		{
			std::this_thread::sleep_for(Duration(ms));
		}
	};
*/


#endif // TIMER_H_INCLUDED
