#ifndef TICK_H_INCLUDED
#define TICK_H_INCLUDED

#include <atomic>
#include <mutex>
#include "timer.h"


typedef enum {
    PR_FIRST,
    PR_PRE_SEQ,
    PR_SEQ,
    PR_POST_SEQ,
    PR_LAST
} tPriority;

/// This is the abstract base class for all objects which have a callback procedure called at every MIDITimer tick.
/// You can use this feature to send, receive or manipulate MIDI messages with accurate timing: the MIDISequencer,
/// MIDIThru and MIDIRecorder classes inherit by this.
/// To use a MIDITickComponent object you must add it to the MIDIManager queue with the MIDIManager::AddTick() method;
/// then you can call the Start() and Stop() methods to start and stop the callback.
class MIDITickComponent {
    public:
        ///
                                    MIDITickComponent(tPriority pr, MIDITick func) : tickp(func),
                                                      dev_time_offset(0), sys_time_offset(0),
                                                      priority(pr), running(false) {}

        /// Returns the address of the StaticTickProc method, which will be called by the MIDIManager at every
        /// clock tick.
        MIDITick*                   GetFunc() const                 { return tickp; }
        /// Returns the priority.
        tPriority                   GetPriority() const             { return priority; }

        /// Sets the running status as *true* and starts the MIDIManager timer.
        /// You must call this in
        virtual void                Start(tMsecs dev_offs = 0);
        virtual void                Stop();

        /// Returns *true* if the callback procedure is active.
        bool                        IsPlaying() const               { return running.load(); }


    protected:
        /// This is the static procedure which you must implement.
        /// The first parameter is the system time from the start of the timer, the second is tipically the
        /// _this_ pointer of the object instance. This should only cast the void pointer to an object pointer and
        /// then call the pt->TickProc(sys_time)
        static void                 StaticTickProc(tMsecs sys_time, void* pt)   {}
        virtual void                TickProc(tMsecs sys_time) = 0;

        const MIDITick*             tickp;

        std::atomic<tMsecs>         dev_time_offset;    ///<
        std::atomic<tMsecs>         sys_time_offset;    ///< The system time of the
        std::mutex                  proc_lock;

    private:
        const tPriority             priority;
        std::atomic<bool>           running;
};

#endif // TICK_H_INCLUDED
