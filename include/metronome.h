#ifndef METRONOME_H_INCLUDED
#define METRONOME_H_INCLUDED

#include "tick.h"
#include "manager.h"


class Metronome : public MIDITickComponent {
    public:
        /// The constructor.
        /// \param n a pointer to a MIDISequencerGUIEventNotifier. If you leave 0 the sequencer will not notify
        /// the GUI.
                                        Metronome(MIDISequencerGUINotifier* n = 0);
        /// The destructor. The MIDISequencerGUINotifier is not owned by the MIDISequencer.
        virtual                         ~Metronome() {}
        /// Moves the time to 0 updating the state (see MIDISequencerState:Reset()), resets all the processors
        ///to their default, sets all the tracks to MIDI out 0 and no time offset, sets tempo scale to default and
        /// no solo.
        void                            Reset();
        /// Returns current MIDIClockTime in MIDI ticks.
        MIDIClockTime                   GetCurrentMIDIClockTime() const         { return cur_clock; }
        /// Returns current time in milliseconds. It is effective even during playback
        float                           GetCurrentTimeMs() const;
        /// Returns current measure (1st measure is 0).
        unsigned int                    GetCurrentMeasure() const
                                                                { return cur_measure; }
        /// Returns current beat in the measure (1st beat is 0).
        unsigned int                    GetCurrentBeat() const  { return cur_beat; }

        /// Returns current tempo scale in percentage (100 = no scaling, 200 = twice faster, etc.).
        unsigned int                    GetTempoScale() const   { return tempo_scale; }
        /// Returns current tempo (BPM) without scaling.
        float                           GetTempoWithoutScale() const
                                                                { return tempobpm; }
        /// Returns current tempo (BPM) taking into account scaling (this is the true actual tempo).
        float                           GetTempoWithScale() const
                                                                { return tempobpm * tempo_scale * 0.01; }


        /// Returns the number of the MIDI out port assigned to the metronome.
        unsigned int                    GetOutPort() const      { return port; }
        /// Returns the number of the MIDI channel assigned to the metronome.
        unsigned int                    GetOutChannel() const    { return chan; }
        /// Returns the number .
        unsigned char                   GetMeasNote() const     { return meas_note; }

        /// Return the number of the port assigned to track _trk_.
        unsigned char                   GetBeatNote() const     { return beat_note; }

        /// Return the number of the port assigned to track _trk_.
        unsigned char                   GetSubdNote() const     { return subd_note; }
        unsigned char                   GetSubdType() const     { return subd_type; }
        ///
        bool                            IsMeasOn() const        { return meas_on; }
        ///
        bool                            IsSubdOn() const        { return subd_on; }
        ///
        unsigned char                   GetTimeSigNumeratot() const
                                                                { return timesig_numerator; }



        void                            SetTempo(float t);

        /// Sets the global tempo scale (_scale_ is the percentage: 100 = no scaling, 200 = twice faster, etc.).
        void                            SetTempoScale(unsigned int scale);


        /// Sets the MIDI out port for the metronome.
        /// \note This only checks if _port_ is a valid port number (eventually normalizing it) and sets
        /// an internal parameter, because the MidISequencer doesn't know the address of the MIDI ports.
        /// So changing ports while the sequencer is playing could leave stuck notes. You can call
        /// MIDIOutDriver::AllNotesOff() on the old MIDI out port before this or, better, use the
        /// corresponding AdvancedSequencer method which does all for you.
        void                            SetOutPort(unsigned int p);

        /// Returns the MIDI channel for the metronome.
        void                            SetOutChannel(unsigned char ch);
        /// Returns the number of the port assigned to track _trk_.
        void                            SetMeasNote(unsigned char note);

        /// Return the number of the port assigned to track _trk_.
        void                            SetBeatNote(unsigned char note);

        /// Return the number of the port assigned to track _trk_.
        void                            SetSubdNote(unsigned char note);

        /// Returns the number of the port assigned to track _trk_.
        void                            SetMeasEnable(bool on_off);

        /// Return the number of the port assigned to track _trk_.
        void                            SetSubdEnable(bool on_off);

        void                            SetSubdType(unsigned char type);

        ///
        void                            SetTimeSigNumerator(unsigned char n);

        // Inherited from MIDITICK

        virtual void Start();
        virtual void Stop();


/*
        enum {
            FOLLOW_MIDI_TIMESIG_MESSAGE,
            FOLLOW_TIMESIG_DENOMINATOR,
            FOLLOW_THEORETIC_VALUE
        };
*/
        static void                     SetMetronomeMode(int mode)
                                                        { metronome_mode = mode; }

    protected:
        /// Inherited by MIDITICK
        static void                     StaticTickProc(tMsecs sys_time, void* pt);
        virtual void                    TickProc(tMsecs sys_time);

        MIDISequencerGUINotifier*       notifier;

        int                             port;               ///< The out port id (-1 if unassigned)
        unsigned int                    chan;               ///< The MIDI channel for sound output
        unsigned char                   meas_note;          ///< The MIDI note number for the first note of a measure
        unsigned char                   beat_note;          ///< The MIDI note number for ordinary beat notes
        unsigned char                   subd_note;          ///< The MIDI note number for subdivision notes
        bool                            meas_on;            ///< First note of the measure on/off
        bool                            subd_on;            ///< Subdivisions on/off
        unsigned char                   subd_type;          ///< Number of subdivisions (can be 2, 3, 4, 5, 6)
        float                           tempobpm;           ///< The current tempo in beats per minute
        unsigned char                   timesig_numerator;  ///< The numerator of current time signature
        unsigned char                   timesig_denominator;///< The denominator of current time signature
        unsigned int                    tempo_scale;        ///< The tempo scale in percentage (100 = true time)


        MIDIClockTime                   cur_clock;          ///< The current MIDI clock in MIDI ticks from the metronme start
        float                           cur_time_ms;        ///< The current clock in milliseconds from the metronoe start
        unsigned int                    cur_beat;           ///< The current beat in the measure (1st beat is 0)
        unsigned int                    cur_measure;        ///< The current measure (1st measure is 0)

        MIDIClockTime                   beat_length;        ///< The duration of a beat
        float                           next_time_on;       ///< The time of the next Note On message (for internal use)
        float                           next_time_off;      ///< The time of the next Noteoff meaasge (for internal use)
        float                           msecs_per_beat;
        float                           onoff_time;

        static int                      metronome_mode;     ///< Flag affecting how metronome beat is calculated
        MIDITimedMessage                msg_beat;

        std::mutex                      proc_lock;

        static const MIDIClockTime      QUARTER_LENGTH = 120;
        static const unsigned char      MEAS_NOTE_VEL = 120;
        static const unsigned char      BEAT_NOTE_VEL = 100;
        static const unsigned char      SUBD_NOTE_VEL = 80;
        static const int                MIN_NOTE_LEN = 30;      // C++ trouble if you declare it float!
};


#endif // METRONOME_H_INCLUDED
