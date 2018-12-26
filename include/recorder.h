#ifndef RECORDER_H_INCLUDED
#define RECORDER_H_INCLUDED

#include "tick.h"
#include "manager.h"
#include "multitrack.h"

#include <atomic>


class MIDIRecorder : public MIDITickComponent {
    public:
                                        MIDIRecorder(MIDIMultiTrack* mlt = 0);

        MIDIMultiTrack*                 GetMultiTrack() const           { return multitrack; }
        float                           GetTempo() const                { return tempobpm; }
        void                            SetTempo(float t);
        void                            SetEnableRec(bool on_off)       { rec_on.store(on_off); }
        void                            SetStartTime(MIDIClockTime t)   { start_time = t; }
        MIDIClockTime                   GetStartTime() const            { return start_time; }

        void                            EnablePort(unsigned int port);
        void                            EnableChannel(unsigned int port, unsigned int ch, unsigned int trk);
        void                            EnableAllChannels(unsigned int port);
        void                            DisablePort(unsigned int port);
        void                            DisableChannel(unsigned int port, unsigned int ch);

        virtual void                    Start();
        virtual void                    Stop();

    protected:

        static void             StaticTickProc(tMsecs sys_time, void* pt);
        void                    TickProc(tMsecs sys_time);

        MIDIMultiTrack*         multitrack;

        tMsecs                  rec_time_offset;    ///< The time between time 0 and sequencer start
        tMsecs                  sys_time_offset;    ///< The time between the timer start and the sequencer start
        float                   tempobpm;           ///< The recording tempo
        MIDIClockTime           start_time;         ///< The MIDIClockTime of the beginning of recording

        std::vector<std::vector<MIDITrack*>*> en_ports;

        std::atomic<bool>       rec_on;

    private:
        const bool              OWNS_MULTI;
};



#endif // RECORDER_H_INCLUDED
