#ifndef ADVANCEDRECORDER_H_INCLUDED
#define ADVANCEDRECORDER_H_INCLUDED

#include "advancedsequencer.h"
#include "notifier.h"
#include "metronome.h"
#include "recorder.h"


class RecNotifier: public MIDISequencerGUINotifier {
    public:
                                    RecNotifier(MIDISequencerGUINotifier * n,
                                                const MIDISequencer* = 0);

        virtual void                Notify(const MIDISequencerGUIEvent &ev);
};


class AdvancedRecorder : public MIDIRecorder {
    public:
                                    AdvancedRecorder(AdvancedSequencer* seq);
        virtual                     ~AdvancedRecorder();

        virtual void                Reset();

        AdvancedSequencer*          GetSequencer()              { return sequencer; }

        // Inherited from MIDIRecorder
        /// Sets the current time to the beginning of the song, updating the internal status. This method is
        /// thread-safe and can be called during playback. Notifies the GUI a GROUP_ALL event to signify a
        /// full GUI reset.
        void                GoToZero()                      { GoToTime(0); }
        /// Sets the current time to the given MIDI time, updating the internal status. This method is
        /// thread-safe and can be called during playback. Notifies the GUI a GROUP_ALL event to signify a
        /// full GUI reset.
        bool                GoToTime(MIDIClockTime time_clk);
        /// Same as GoToTime(), but the time is given in milliseconds.
        bool                GoToTimeMs(float time_ms);
        /// Sets the current time to the given measure and beat, updating the internal status. This is as
        /// MIDISequencer::GoToMeasure() but uses a better algorithm and sends to the MIDI out ports all the
        /// appropriate sysex, patch, pitch bend and control change messages.
        bool                GoToMeasure(int measure, int beat = 0);



        virtual void                Start();
        virtual void                Stop();

    protected:
        enum {
                                    ST_PRE_COUNT,
                                    ST_WITH_SEQ,
                                    ST_ALONE,
                                    ST_STOP
        };

        static void                 StaticTickProc(tMsecs sys_time, void* pt);
        void                        TickProc(tMsecs sys_time);

        AdvancedSequencer*          sequencer;
        Metronome                   metronome;
        tMsecs                      metro_delay;
        int                         status;

};



#endif // ADVANCEDRECORDER_H_INCLUDED
