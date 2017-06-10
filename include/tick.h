#ifndef TICK_H_INCLUDED
#define TICK_H_INCLUDED

#include "timer.h"


class MIDITICK {
    public:

    protected:

        static void                 StaticTickProc(tMsecs sys_time, MIDITICK *p)    { p->TickProc(sys_time); }
        void                        TickProc(tMsecs sys_time);

};

#endif // TICK_H_INCLUDED
