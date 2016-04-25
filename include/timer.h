#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <chrono>
#include <thread>
#include <atomic>

#include <iostream>     // for debug

using namespace std::chrono;

typedef unsigned long tMsecs;
typedef  void (*MIDITick)(tMsecs, void*);


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

        bool                        Start();
        void                        Stop();

        static tMsecs               GetSysTimeMs()
                                        { return duration_cast<milliseconds>(steady_clock::now() - sys_clock_base).count(); }

        static void                 Wait(unsigned int msecs)
                                        { std::this_thread::sleep_for(milliseconds(msecs)); }


    protected:

        static const unsigned int   DEFAULT_RESOLUTION = 10;

        static void                 ThreadProc( MIDITimer* timer );

        int                         resolution;
        MIDITick                    tick;
        void*                       tick_param;
        std::thread                 bg_thread;
        std::atomic<bool>           timer_on;
        static timepoint            sys_clock_base;
};


#endif // TIMER_H_INCLUDED
