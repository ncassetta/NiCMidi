#ifndef TICK_H_INCLUDED
#define TICK_H_INCLUDED

#include "timer.h"


typedef enum                {
                                        PR_FIRST,
                                        PR_PRE_SEQ,
                                        PR_SEQ,
                                        PR_POST_SEQ,
                                        PR_LAST
                                    } tPriority;

 typedef void (*tick_static)(tMsecs, void*);



class MIDITICK {
    public:
                                    MIDITICK(tPriority pr) : p(this), static_func(StaticTickProc), priority(pr) {}

        void*                       GetPointer() const              { return p; }
        tick_static                 GetFunc() const                 { return static_func; }
        tPriority                   GetPriority() const             { return priority; }

    protected:
        static void                 StaticTickProc(tMsecs sys_time, void* pt)   {}
        virtual void                TickProc(tMsecs sys_time) = 0;

        void*                       p;
        void                        (*static_func)(tMsecs, void*);
        tPriority                   priority;




};

#endif // TICK_H_INCLUDED
