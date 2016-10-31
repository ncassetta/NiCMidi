/*
 * ADAPTED FROM
 *
 * libjdkmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  *** RELEASED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) April 27, 2004 ***
 *
 *
 * BY NICOLA CASSETTA
 *
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef _JDKMIDI_SEQUENCER_H
#define _JDKMIDI_SEQUENCER_H

#include "multitrack.h"
#include "matrix.h"
#include "processor.h"
#include "notifier.h"

#include <string>


class MIDISequencer;        // forward declaration

///
/// This class inherits from the pure virtual MIDIProcessor and is a multi-purpose processor
/// implementing muting, soloing, rechannelizing, velocity scaling and transposing.
/// Moreover, you can set a custom MIDIProcessor pointer which extra-processes messages.
/// The MIDISequencer class contains an independent MIDISequencerTrackProcessor for every MIDI Track.
/// Advanced classes like MIDISequencer and AdvancedSequencer allow you to set muting, transposing,
/// etc. by dedicated methods without dealing with it: the only useful function for the user is the
/// extra processing hook.
/// However, you could subclass this if you want to get new features.
///
class MIDISequencerTrackProcessor : public MIDIProcessor {
    public:
        /// The constructor. Default is no processing (MIDI messages leave the processor unchanged).
                        MIDISequencerTrackProcessor();
        /// The destructor
        virtual         ~MIDISequencerTrackProcessor() {}
        /// Resets all values to default state (no processing at all).
        virtual void    Reset();
        /// Sets the extra processor for the track. The user processing is done before all internal
        /// processing, and if the user Process() function returns _false_ it is not done at all.
        /// If you want to eliminate an already set processor call this with 0 as parameter
        void            SetExternalProcessor(MIDIProcessor* proc)
                                                { extra_proc = proc; }
        /// Processes message msg, changing its parameters according to the state of the processor
        virtual bool    Process ( MIDITimedMessage *msg );

        bool            mute;                   ///< track is muted
        bool            solo;                   ///< track is soloed
        int             velocity_scale;         ///< current velocity scale value for note ons, 100=normal
        int             rechannel;              ///< rechannelization value, or -1 for none
        int             transpose;              ///< amount to transpose note values
        MIDIProcessor   *extra_proc;            ///< extra midi processing for this track
};


///
/// This class stores current MIDI parameters for a track.
/// It stores track name, program, pitch bend, all control changes values and a matrix with notes on and off.
/// The MIDISequencerState class contains a MIDISequencerTrackState for every MIDI Track, and it take care of
/// updating parameters. You can ask the MIDISequencerTrackState if you want to know actual track parameters,
/// however advanced class AdvancedSequencer allows you to get them without dealing with it, so the use of this
/// class is mainly internal. However, you could subclass it if you want to keep track of other parameters.
///
class MIDISequencerTrackState {
    public:
        /// The constructor.
        /// Initial attributes are program = -1 (undefined),  bender_value = 0, all controls = -1,
        /// track_name = "", all notes off.
                        MIDISequencerTrackState();
        virtual         ~MIDISequencerTrackState() {}

        // Copy constructor used but unneeded!

        /// Resets default values.
        virtual void    Reset();

        char            program;		    ///< current program change, or -1
        int             bender_value;		///< last seen bender value
        std::string     track_name;	        ///< track name
        bool            notes_are_on;       ///< true if there are notes currently on
        MIDIMatrix      note_matrix;        ///< to keep track of all notes on
        char            control_values[C_ALL_NOTES_OFF];        // NOT unsigned to allow -1 if not changed
                                            ///< array of current control change values
        bool            got_good_track_name;///< internal use
};


///
/// This class stores current MIDI general parameters for a MIDISequencer object.
/// It contains a MIDIMultiTrackIterator, allowing to set a 'now' time: when current time changes (because
/// the sequencer is playing or time is changed by the user) the object keep tracks of current timesig, keysig,
/// tempo(BPM), marker, measure and beat data. Moreover it contains an independent MIDISequencerTrackState
/// for every MIDI Track of the MIDISequencer, and you can examine them for knowing actual track parameters.
/// It inherits from the pure virtual MIDIProcessor: the MIDISequencer sends to it MIDI messages and it
/// processes them remembering actual parameters and notifying changes to the GUI.
/// All methods and attributes are public because they are used by MIDISequencer class; the advanced class
/// AdvancedSequencer allows you to know actual parameters without directly examining them, so the use of this
/// class is mainly internal.
/// However, you could subclass it if you want to keep track of other parameters.
///
class MIDISequencerState : public MIDIProcessor {
// Doesn't inherit from MIDISequencerGUINotifier because notifier and sequencer must be independent objects
// (notifier is used also by the MIDIManager)
// All is public: used by various classes
    public:
        /// The constructor. Appropriate values are set by the sequencer when it creates the object.
                                MIDISequencerState(MIDIMultiTrack *multitrack_,
                                                   MIDISequencerGUINotifier *n = 0);
        /// The copy constructor.
                                MIDISequencerState(const MIDISequencerState &s);
        /// The destructor does nothing.
                                ~MIDISequencerState();
        /// The equal operator.
        const MIDISequencerState& operator= (const MIDISequencerState &s);

        /// Returns the number of tracks of the multitrack.
        unsigned int            GetNumTracks() const        {return multitrack->GetNumTracks();}
        /// Resets the state to default values. These are: cur_clock = 0, tempo = 120 bpm,
        /// time = 4/4, keysig = C Maj, no marker. Moreover resets all track states (see
        /// MIDISequencerTrackState::Reset()).
        void                    Reset();
        /// This is the process function inherited from MIDIProcessor. When you get a MIDI message
        /// from the sequencer, it is processed by the state, which update its parameters and
        /// notifies the GUI if required.
        bool                    Process( MIDITimedMessage* msg );

        /// These are used internally for notifying the GUI when a something happens (a parameter
        /// was changed, time is moved, etc.)
        void                    Notify(int group, int item = 0) const;
        void                    NotifyTrack(int item) const;

        MIDISequencerGUINotifier* notifier;
        MIDIMultiTrack*         multitrack;
        MIDIMultiTrackIterator  iterator;

        MIDIClockTime           cur_clock;          ///< The current MIDI clock in MIDI ticks
        double                  cur_time_ms;        ///< The current clock in milliseconds
        int                     cur_beat;           ///< The current beat in the measure (1st beat is 0)
        int                     cur_measure;        ///< The current measure (1st measure is 0)
        MIDIClockTime           beat_length;        ///< The duration of a beat
        MIDIClockTime           next_beat_time;     ///< The MIDI time of the next beat (for internal use)

        double                  tempobpm;           ///< The current tempo in beats per minute
        char                    timesig_numerator;  ///< The numerator of current time signature
        char                    timesig_denominator;///< The denominator of current time signature
        char                    keysig_sharpflat;   ///< The current key signature accidents (
        char                    keysig_mode;        ///< Major mode (0) or minor (1)
        std::string             marker_text;        ///< The current marker
        std::vector<MIDISequencerTrackState*>
                                track_states;       ///< A track state for every track
        int                     last_event_track;   ///< Internal use
};


///
/// This class implements a complete sequencer. It holds:
/// - a MIDIMultiTrack for storing MIDI messages
/// - a MIDISequencerTrackProcessor for every track, allowing muting, soloing, transposing, ecc.
/// - a MIDIMultiTrackIterator, allowing to set a 'now' time, moving it along
/// - an (optional) MIDISequencerGUINotifier, that notifies the GUI about MIDI events
/// - a MIDISequencerState (which embeds the multitrack, the iterator and the notifier) to keep track
///   of actual parameters (tempo, keysig, track parameters, etc.).
/// Moreover it allows the user to assign a separate MIDI port for each track and to shift the time of
/// track events by a given amount (positive or negative) of MIDI ticks.
/// \note This class has no playing capacity. For playing MIDI content you must use it together with a
/// MIDIManager. See the example files for effective using. AdvancedSequencer is an all-in-one class for
/// sequencing and playing
///
class MIDISequencer {
    public:
        /// The constructor.
        /// \param m a pointer to a MIDIMultitrack that will hold MIDI messages
        /// \param n a pointer to a MIDISequencerGUIEventNotifier. If you leave 0 the sequencer will not notify
        /// the GUI.
                                        MIDISequencer(MIDIMultiTrack* m, MIDISequencerGUINotifier* n = 0);
        /// The destructor. The MIDIMultitrack and the MIDISequencerGUINotifier are not owned by the MIDISequencer.
        virtual                         ~MIDISequencer();
        /// Resets the state of the sequencer (see MIDISequencerState:Reset()) and all the processors,
        /// sets all the tracks to MIDI out 0 and no time offset.
        void                            Reset();
        /// Returns current MIDIClockTime in MIDI ticks.
        MIDIClockTime                   GetCurrentMIDIClockTime() const
                                                                { return state.cur_clock; }
        /// Returns current time in milliseconds.
        double                          GetCurrentTimeInMs() const
                                                                { return state.cur_time_ms; }
        /// Returns current beat in the measure (1st beat is 0).
        int                             GetCurrentBeat() const  { return state.cur_beat; }
        /// Returns current measure (1st measure is 0).
        int                             GetCurrentMeasure() const
                                                                { return state.cur_measure; }
        /// Returns current tempo scale (1.00 = no scaling, 2.00 = twice faster, etc.).
        double                          GetTempoScale() const
                                                                { return ((double)tempo_scale) * 0.01; }

        /// Returns current tempo (BPM) without scaling ( actual tempo is GetCurrentTempo() * GetCurrentTempoScale() ).
        double                          GetTempo() const        { return state.tempobpm; }

        /// Returns current MIDISequencerState (i.e. the global sequencer state at current time). You can easily
        /// jump from a time to another saving and retrieving sequencer states.
        MIDISequencerState*             GetState()              { return &state; }
        const MIDISequencerState*       GetState() const        { return &state; }
        /// Returns the MIDISequencerTrackState for track _trk_.
        MIDISequencerTrackState*        GetTrackState(int trk)  { return state.track_states[trk]; }
        const MIDISequencerTrackState*  GetTrackState(int trk) const
                                                                { return state.track_states[trk]; }
        /// Returns the MIDISequencerTrackProcessor for track _trk_.
        MIDISequencerTrackProcessor*    GetTrackProcessor(int trk)
                                                                { return track_processors[trk]; }
        const MIDISequencerTrackProcessor* GetTrackProcessor(int trk) const
                                                                { return track_processors[trk]; }
        /// Returns the number of tracks of the multitrack.
        unsigned int                    GetNumTracks() const	{ return state.GetNumTracks(); }
        /// Returns *true* if any track is soloed.
        bool                            GetSoloMode() const     { return solo_mode; }
        /// Returns the time offset (in MIDI ticks) assigned to track _trk_. See SetTimeOffset(),
        /// SetTimeOffsetMode().
        int                             GetTrackTimeShift(int trk) const
                                                                { return time_shifts[trk]; }
        /// Return the number of the port assigned to track _trk_.
        unsigned int                    GetTrackPort(int trk) const
                                                                { return track_ports[trk]; }

        /// Copies the MIDISequencerState _s_ into the internal sequencer state. You can easily
        /// jump from a time to another saving and retrieving sequencer states.
        void                            SetState(MIDISequencerState* s)
                                                                { state = *s; }
        /// Set the global tempo scale (1.00 = no scaling, 2.00 = twice faster, etc.).
        void                            SetTempoScale(double scale);
        /// Soloes/unsoloes a track
        /// \param m on/off
        /// \param trk the nunber of the track if m is true, otherwhise you can leave default value
        void                            SetSoloMode(bool m, int trk = -1);
        /// Sets the time shift offset (in MIDI ticks) for track _trk_. The offset can be positive or
        /// negative; events shifted include all channel messages and sysex messages (others remain at
        /// their time).
        /// If you select a negative offset, be sure not to have shifted events at lesser time than the
        /// offset (they won't be shifted). See also SetTimeOffsetMode().
        void                            SetTrackTimeShift(int trk, int offset)
                                                                { time_shifts[trk] = offset; }
                                                                        // TODO: reset the iterator ?
        /// Sets the MIDI port for track _trk_.
        /// \note This only checks if _port_ is a valid port number (eventually normalizing it) and sets
        /// an internal parameter, because the MidISequencer doesn't know the address of the MIDI ports.
        /// So changing ports while the sequencer is playing could leave stuck notes. You can call
        /// MIDIOutDriver::AllNotesOff() on the old MIDI out port before this or, better, use the
        /// corresponding AdvancedSequencer method which does all for you.
        void                            SetTrackPort(int trk, unsigned int port);
        /// Inserts a new empty track with default track parameters (transpose, time offset, etc. ) at
        /// position _trk_ (_trk_ must be in the range 0 ... GetNumTracks() - 1). If _trk_ == -1 (default)
        /// appends the track at the end.
        /// \return *true* if the track was effectively inserted
        /// \note You shouldn't use the corresponding method of MIDIMultiTrack class, as it doesn't sync the
        /// iterator and the sequencer internal arrays. If you change the number of tracks directly in the
        /// multitrack (for example when loading a MIDI file) you must then call MIDISequencer::Reset() for
        /// updating the sequencer, but this will reset all track parameters to the default (no transposing,
        /// muting, time offset, etc...)
        bool                            InsertTrack(int trk = -1);
        /// Deletes the track _trk_ and its events. _trk_ must be in the range 0 ... GetNumTracks() - 1.
        /// \return *true* if the track was effectively deleted
        /// \see note to InsertTrack()
        bool                            DeleteTrack(int trk);
        /// Moves a track from the position _from_ to the position _to_. ( _from_ e _to_ must be in the range
        /// 0 ... GetNumTracks() - 1).
        /// \return *true* if the track was effectively moved
        /// \see note to InsertTrack()
        bool                            MoveTrack(int from, int to);
        /// Sets the 'now' time to the beginning of the song, upgrading the internal status.
        /// Notifies the GUI a GROUP_ALL event to signify a GUI reset.
        void                            GoToZero();
        /// Sets the 'now' time to the MIDI time _time_clk_, upgrading the internal status.
        /// Notifies the GUI a GROUP_ALL event to signify a GUI reset
        /// \return _true_ if the time _time_clk_ is effectively reached, _false_ otherwise
        /// (_time_clk_ is after the end of the song)
        bool                            GoToTime (MIDIClockTime time_clk);
        /// Same as GoToTime(), but time is given in milliseconds.
        bool                            GoToTimeMs (double time_ms);
        /// Sets the 'now' time to the given measure and beat, upgrading the internal status.
        /// Notifies the GUI a GROUP_ALL event to signify a GUI reset
        /// \return see GoToTime()
        bool                            GoToMeasure (int measure, int beat = 0);
        /// Gets the time of the next event
        /// \param[out] t: the requested time in MIDI ticks from the beginning
        /// \return _true_ if there is effectively a next event (and *t is a valid time) _false_ if we are at
        /// the end of the song (*t is undefined)
        bool                            GetNextEventTime (MIDIClockTime *time_clk);
        /// Same of GetNextEventTime(), but time is returned in milliseconds from the beginning.
        bool                            GetNextEventTimeMs (double *time_ms);
        /// Gets the next event (respect current position). This queries the state for the next event in the
        /// multitrack, then processes it with the corresponding track processor (eventually changing the
        /// original event, for example if transposing) and updates the state. Moreover it sends appropriate
        /// messages to the GUI. If there are no events before the next metronome click you will get a
        /// Beat Marker internal event.
        /// \param[out] tracknum: the track of the next event
        /// \param[out] msg the MIDI event
        /// \return _true_ if there is effectively a next event (and the parameters are valid), _false_ otherwise
        /// (parameters are undefined)
        bool                            GetNextEvent (int *trk, MIDITimedMessage *msg);

        /// Converts the time_time_clk_, given in MIDi ticks, into milliseconds, taking account of tempo changes.
        double                          MIDItoMs(MIDIClockTime time_clk);  // new : added by me
        /// This function is the equivalent of GoToTime(state.cur_time) and should be used to update the sequencer
        /// state after an edit in the multitrack (adding, deleting or editing events, for changes in the track
        /// structure see InsertTrack(), DeleteTrack() and MoveTRack()). If you have edited the multitrack, call
        /// this before moving time, getting events or playing.
        void                            UpdateStatus()  { GoToTime(state.cur_clock); }
        /// Sets the time shifting of events on and off (default is off). Time shift should only be on when
        /// playing. If you use the MIDISequencer together with a MIDIManager for playing, the manager turns
        /// automatically on and off this, so you have no need to use this function
        void                            SetTimeShiftMode(bool f)
                                                        { state.iterator.GetState().SetTimeShiftMode(f); }

    protected:

         /// Internal use: scans events at 'now' time upgrading the sequencer state
         void                           ScanEventsAtThisTime();

        MIDITimedMessage                beat_marker_msg;
        bool                            solo_mode;
        int                             tempo_scale;
        std::vector<MIDISequencerTrackProcessor*>
                                        track_processors;
        std::vector<int>                time_shifts;
        std::vector<unsigned int>       track_ports;
        MIDISequencerState              state;
};


#endif





/*
 *  libjdksmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  *** RELEASED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) April 27, 2004 ***
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
//
// Copyright (C) 2010 V.R.Madgazin
// www.vmgames.com vrm@vmgames.com
//

//
// MODIFIED by N. Cassetta  ncassetta@tiscali.it
// search /* NC */ for modifies
//

#ifndef JDKSMIDI_SEQUENCER_H
#define JDKSMIDI_SEQUENCER_H



namespace jdksmidi
{




/*



///
/// This class stores current MIDI general parameters.
/// It contains a MIDIMultiTrackIterator, allowing to set a 'now' time: when the current time changes (because
/// the sequencer is playing or it is moved by the user) the class stores current timesig, keysig, tempo(BPM),
/// marker, measure and beat data. Furthermore it inherits from the pure virtual MIDIProcessor:
/// the MIDISequencer sends to it MIDI messages and it processes them  remembering actual parameters and
/// notifying chamges to the GUI.
/// This class contains an independent MIDISequencerTrackState for every MIDI Track and you can
/// ask it for knowing actual parameters. However, advanced class AdvancedSequencer allows you to know them
/// without dealing with this, so the use of this class is mainly internal.
/// However, you could subclass it if you want to keep track of other parameters.
///

class MIDISequencerState : public MIDIProcessor
{
public:

    /// The constructor sets sequencer current time to 0. It allocates memory to hold a MIDISequencerTrackState
    /// for every track. Parameters are not owned.
    MIDISequencerState ( const MIDISequencer *s,
                         const MIDIMultiTrack *multitrack_,
                         MIDISequencerGUINotifier *n );

    /// The copy constructor
    MIDISequencerState ( const MIDISequencerState &s );

    /// The destructor frees allocated memory
    ~MIDISequencerState();

    /// The equal operator
    const MIDISequencerState & operator = ( const MIDISequencerState &s );

    /// Resets the state to current time = 0.
    void Reset();                                       /* NC    // new

    /// Processes the message msg: if it is a channel message (or a track name meta) send it to a
    /// MIDISequencerTrackState, otherwise notify the GUI directly
    bool Process(MIDITimedMessage* msg);                /* NC    // new

    /// Notifies events to GUI
    void Notify( int group, int item = 0 );             /* NC    // new

    MIDISequencerGUINotifier *notifier;
    const MIDISequencer* seq;                           /* NC for notifying
    const MIDIMultiTrack *multitrack;
    int num_tracks;                                 ///< nunber of tracks of the sequencer

    MIDISequencerTrackState *track_state[64];
    MIDIMultiTrackIterator iterator;
    MIDIClockTime cur_clock;                        ///< current time MIDI clock
    float cur_time_ms;                              ///< current time in ms
    int cur_beat;                                   ///< current beat
    int cur_measure;                                ///< current measure
    MIDIClockTime last_beat_time;   /* NC           ///< used internally by Process()
    MIDIClockTime next_beat_time;                   ///< used internally by Process()
    float tempobpm;                                 ///< current tempo in beats per minute
    char timesig_numerator;                         ///< numerator of current time signature
    char timesig_denominator;                       ///< denominator of current time signature
    char keysig_sharpflat;                          ///< current key signature accidentals
    char keysig_mode;                               ///< major/minor mode
    char marker_name[40];                           ///< current marker name
    int last_event_track;                           ///< used internally by Process()
};


///
/// This class implements a complete sequencer. This class holds:
/// - a MIDIMultiTrack for storing MIDI messages
/// - a MIDISequencerTrackProcessor for every track, allowing muting, soloing, transposing, ecc.
/// - a MIDIMultiTrackIterator, allowing to set a 'now' time, moving it along
/// - a MIDISequencerGUIEventNotifier, that notifies the GUI about MIDI events
/// - a MIDISequencerState (which embeds the multitrack, the iterator and the notifier) to keep track
/// of actual parameters (tempo, keysig, track parameters, etc.)
/// \note This class has no playing capacity. For playing MIDI content you must use it together with a
/// MIDIManager. See the example files for effective using. AdvancedSequencer is an all-in-one class for
/// sequencing and playing
///
class MIDISequencer {
    public:

            /// The constructor.
            /// \param m a pointer to a MIDIMultitrack that will hold MIDI messages
            /// \param n a pointer to a MIDISequencerGUIEventNotifier. If you leave 0 the sequencer
            /// will not notify the GUI.
                                        MIDISequencer (const MIDIMultiTrack *m, MIDISequencerGUINotifier *n = 0);
            /// The destructor frees allocated memory. The MIDIMultiTrack and the MIDISequencerGUINotifier
            /// are not owned by the MIDISequencer
        virtual                         ~MIDISequencer();

            /// Resets the corresponding MIDISequencerTrackState and MIDISequencerTrackProcessor.
            /// See MIDISequencerTrackState::Reset() and MIDISequencerTrackProcessor::Reset()
        void                            ResetTrack (int trk);

        /// Call ResetTrack() for all tracks
        void                            ResetAllTracks();

        /// Returns current MIDIClockTime
        MIDIClockTime                   GetCurrentMIDIClockTime() const     { return state.cur_clock; }

        /// Returns current time in milliseconds
        float                           GetCurrentTimeInMs() const          { return state.cur_time_ms; }

        /// Returns current beat
        int                             GetCurrentBeat() const              { return state.cur_beat; }

    /// Returns current measure
    int GetCurrentMeasure() const
    {
        return state.cur_measure;
    }

    /// Returns curremt tempo scale (1.00 = no scaling, 2.00 = twice faster, etc.)
    double GetCurrentTempoScale() const
    {
        return ( ( double ) tempo_scale ) * 0.01;
    }

    ///< Returns current tempo (BPM) without scaling ( actual tempo is GetCurrentTempo() * GetCurrentTempoScale() )
    double GetCurrentTempo() const
    {
        return state.tempobpm;
    }

    /// Returns current MIDISequencerState (i.e. the global sequencer state at current time). You can easily
    /// jump from a time to another saving and retrieving sequencer states.
    MIDISequencerState *GetState()
    {
        return &state;
    }

    const MIDISequencerState *GetState() const
    {
        return &state;
    }

    /// Returns the MIDISequencerTrackState for track trk
    MIDISequencerTrackState * GetTrackState ( int trk )
    {
        return state.track_state[trk];
    }

    const MIDISequencerTrackState * GetTrackState ( int trk ) const
    {
        return state.track_state[trk];
    }

    /// Returns the MIDISequencerTrackProcessor for track trk
    MIDISequencerTrackProcessor * GetTrackProcessor ( int trk )
    {
        return track_processors[trk];
    }

    const MIDISequencerTrackProcessor * GetTrackProcessor ( int trk ) const
    {
        return track_processors[trk];
    }

    /// Returns the number of tracks of the MIDIMultiTrack
    int GetNumTracks() const
    {
        return state.num_tracks;
    }

    /// Returns the solo mode on/off
    bool GetSoloMode() const
    {
        return solo_mode;
    }

    /// Copies the MIDISequencerState s into the internal sequencer state. You can easily
    /// jump from a time to another saving and retrieving sequencer states.
    void SetState ( MIDISequencerState *s )
    {
        state = *s;
    }

    /// Sets the tempo scale (1.00 = no scaling, 2.00 = twice faster, etc)
    void SetCurrentTempoScale ( float scale )
    {
        tempo_scale = ( int ) ( scale * 100 );
    }

    /// Soloes/unsoloes a track
    /// \param m on/off
    /// \param trk the nunber of the track if m is true, otherwhise you can leave default value
    void SetSoloMode ( bool m, int trk = -1 );

    /// Sets the 'now' time to the beginning of the song, upgrading the internal status.
    /// Notifies the GUI a GROUP_ALL notifier event to signify a GUI reset
    void GoToZero();

    /// Sets the 'now' time to the MIDI time time_clk, upgrading the internal status.
    /// Notifies the GUI a GROUP_ALL notifier event to signify a GUI reset
    /// \return _true_ if the time time_clk is effectively reached, _false_ otherwise (if time_clk is after
    /// the end of the song)
    bool GoToTime ( MIDIClockTime time_clk );

    /// Same as GoToTime(), but the time is given in milliseconds
    bool GoToTimeMs ( float time_ms );

    /// Sets the 'now' time to the given measure and beat, upgrading the internal status.
    /// \return see GoToTime()
    bool GoToMeasure ( int measure, int beat = 0 );


    bool GetNextEventTimeMs ( float *t );
    bool GetNextEventTimeMs ( double *t );

    /// Get the time of the next event (respect to the 'now' time) in MIDI ticks
    /// \return _true_ if there is a next event (and *t is a valid time) _false_ otherwise (*t is undefined)
    bool GetNextEventTime ( MIDIClockTime *t );

    /// Get the next event respect to the 'now' time.
    /// \param[out] tracknum: the track of the next event
    /// \param[out] msg the MIDI event
    /// \return _true_ if there is a next event (and the paraneters are valid), _false_ otherwise (parameters undefined)
    bool GetNextEvent ( int *tracknum, MIDITimedMessage *msg );



protected:

    /// Internal use: scans events at 'now' time upgrading the sequencer status
    void ScanEventsAtThisTime();

    MIDITimedMessage beat_marker_msg;

    bool solo_mode;
    int tempo_scale;

    int num_tracks;
    MIDISequencerTrackProcessor *track_processors[64];

    MIDISequencerState state;
} ;
*/
}

#endif
