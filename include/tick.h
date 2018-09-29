#ifndef TICK_H_INCLUDED
#define TICK_H_INCLUDED

#include <atomic>

#include "timer.h"


typedef enum {
    PR_FIRST,
    PR_PRE_SEQ,
    PR_SEQ,
    PR_POST_SEQ,
    PR_LAST
} tPriority;


class MIDITickComponent {
    public:
                                    MIDITickComponent(tPriority pr, MIDITick func) : p(this), tickp(func),
                                                      priority(pr), running(false) {}

        void*                       GetPointer() const              { return p; }
        MIDITick*                   GetFunc() const                 { return tickp; }
        tPriority                   GetPriority() const             { return priority; }

        virtual void                Start()                         { running = true; MIDITimer::Start(); }
        virtual void                Stop()                          {  MIDITimer::Stop(); running = false; }

        bool                        IsPlaying() const               { return running; }


    protected:
        static void                 StaticTickProc(tMsecs, void*)   {}
        virtual void                TickProc(tMsecs sys_time) = 0;

        void*                       p;
        MIDITick*                   tickp;

    private:
        const tPriority             priority;
        std::atomic<bool>           running;
};

#endif // TICK_H_INCLUDED
