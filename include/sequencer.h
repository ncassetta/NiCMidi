/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2021, 2022  Nicola Cassetta
 *   https://github.com/ncassetta/NiCMidi
 *
 *   This file is part of NiCMidi.
 *
 *   NiCMidi is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation, either version 3 of
 *   the License, or (at your option) any later version.
 *
 *   NiCMidi is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with NiCMidi. If not, see <http://www.gnu.org/licenses/>.
 */


/// \file
/// Contains the definitions of the classes MIDISequencerTrackState, MIDISequencerState and MIDISequencer.

#ifndef _NICMIDI_SEQUENCER_H
#define _NICMIDI_SEQUENCER_H

#include "multitrack.h"
#include "matrix.h"
#include "processor.h"
#include "notifier.h"
#include "tick.h"

#include <string>
#include <mutex>
#include <atomic>


class MIDISequencer;        // forward declaration



///
/// Stores current MIDI parameters for a sequencer track.
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
        /// The destructor does nothing.
        virtual         ~MIDISequencerTrackState() {}

        // Copy constructor and assignment operator generated by the compiler
        //                MIDISequencerTrackState(const MIDISequencerTrackState &s);

        /// Resets default values.
        virtual void    Reset();

        int16_t         program;		    ///< the current program change, or -1 if undefined
        int16_t         bender_value;		///< the last seen bender value
        std::string     track_name;	        ///< the track name
        bool            notes_are_on;       ///< true if there are notes currently on
        MIDIMatrix      note_matrix;        ///< to keep track of all notes on
        int16_t         control_values[C_ALL_NOTES_OFF];
                                            ///< an array of current control change values, or -1 if not defined
        bool            got_good_track_name;///< internal use
};


///
/// Stores current MIDI general parameters for a MIDISequencer object, embedding a MIDISequencerTrackState for each track.
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
        /// The constructor is called by the MIDISequencer class constructor, which sets appropriate
        /// values for parameters.
                                MIDISequencerState(MIDIMultiTrack *multitrack_,
                                                   MIDISequencerGUINotifier *n = 0);
        /// The copy constructor. \note This only copies the pointers to the sequencer and the notifier,
        /// so you can use it only if you are copying different states of the same MIDISequencer
        /// class instance.
                                MIDISequencerState(const MIDISequencerState &s);
        /// The destructor. The sequencer and notifier pointers are not owned by the class and they
        /// won't freed.
        virtual                 ~MIDISequencerState();
        /// The assignment operator. See the note to the copy constructor.
        const MIDISequencerState& operator= (const MIDISequencerState &s);

        /// Resets the state to default values. These are: cur_clock = 0, tempo = 120 bpm,
        /// time = 4/4, keysig = C Maj, no marker. Moreover resets all track states (see
        /// MIDISequencerTrackState::Reset()).
        void                    Reset();
        /// This is the process function inherited from MIDIProcessor. When you get a MIDI message
        /// from the sequencer, it is processed by the state, which updates its parameters and
        /// notifies the GUI if required.
        /// \return **true** if _msg_ is a real MIDI message, **false** if it is a service message
        /// (a NoOp or beat marker).
        bool                    Process(MIDITimedMessage* msg);
        /// Notifies the GUI when something happens (a parameter was changed,
        /// current time is moved, etc.)
        void                    Notify(int group, int item = 0) const;
        /// These are used for notifying the GUI when something happens (a parameter was changed,
        /// current time is moved, etc.)
        void                    NotifyTrack(int item) const;
        /// Sets the cur time to the given time. This assumes t > cur_time and **no events in the interval**.
        /// It is used, for example, by the sequencer when it must go to a time with no event, or
        /// after the end time in UNBOUNDED mode.
        /// \warning this doesn't check anything and doesn't process any event.
        void                    GoForwardNoEvent(MIDIClockTime t);



        MIDISequencerGUINotifier* notifier;         ///< The notifier
        MIDIMultiTrack*         multitrack;         ///< The MIDIMultiTrack holding MIDI messages
        MIDIMultiTrackIterator  iterator;           ///< The iterator for moving along the multitrack

        MIDIClockTime           cur_clock;          ///< The current MIDI clock in MIDI ticks
        float                   cur_time_ms;        ///< The current clock in milliseconds
        unsigned int            cur_beat;           ///< The current beat in the measure (1st beat is 0)
        unsigned int            cur_measure;        ///< The current measure (1st measure is 0)
        MIDIClockTime           beat_length;        ///< The duration of a beat
        unsigned int            number_of_beats;    ///< Number of beats in the measure
        MIDIClockTime           next_beat_time;     ///< The MIDI time of the next beat (for internal use)

        float                   tempobpm;           ///< The current tempo in beats per minute
        unsigned int            tempo_scale;        ///< The tempo scale in percentage (100 = true time)
        unsigned char           timesig_numerator;  ///< The numerator of current time signature
        unsigned char           timesig_denominator;///< The denominator of current time signature
        signed char             keysig_sharpflat;   ///< The current key signature accidents (
        unsigned char           keysig_mode;        ///< Major mode (0) or minor (1)
        std::string             marker_text;        ///< The current marker
        std::vector<MIDISequencerTrackState*>
                                track_states;       ///< A track state for every track
        int                     last_event_track;   ///< Internal use
        MIDIClockTime           last_beat_time;     ///< Internal use
        float                   ms_per_clock;       ///< Internal use
        float                   last_time_ms;       ///< Internal use
        MIDIClockTime           last_tempo_change;  ///< Internal use
        MIDIClockTime           count_in_time;      ///< Internal use

        // TODO: change the name of this variable for example count_in_stop
        unsigned char           playing_status;     ///< Flag affecting the TickProc (count in, auto stop, etc\.)
        static int              metronome_mode;     ///< Flag affecting how metronome beat is calculated
};


///
/// A MIDITickComponent which implements a basic sequencer, able to play the MIDI events contained in a MIDIMultiTrack.
/// It embeds:
/// - a MIDIMultiTrack for storing MIDI messages
/// - a MIDIMultiTrackIterator, allowing to set a 'now' time, moving it along (manually or by playing)
/// - an (optional) MIDISequencerGUINotifier, that notifies the GUI about MIDI events when they happen
/// - a MIDISequencerState which keeps track of actual sequencer status (tempo, keysig, track parameters, etc.).
///
/// If you want to play events contained in the multitrack you must add your instance to the MIDIManager queue of
/// MIDITickComponent objects with the MIDIManager::AddMIDITick() method, then you can call Start() (aliased by Play())
/// and Stop() to start and stop a separate thread which takes care of playing. All methods are thread safe and can be
/// called during playback.
/// Moreover the class allows the user to:
/// - assign a separate MIDI out port for each track
/// - assign a separate time shift amount (positive or negative) in MIDI ticks for every track
/// - set a separate MIDIProcessor for each track
/// - stretch the global tempo by a percentage (lesser or greater than 100)
/// - set a repeat play loop between two measures
///
/// \note This class is especially suitable for subclassing but has limited playing capacity (for example, if you jump
/// from a time to another it updates the track parameters but doesn't send the appropriate MIDI messages to the drivers,
/// so playing could be inconsistent with showed parameters).
/// The AdvancedSequencer class is an enhanced sequencer with more features and more easy use.
///
class MIDISequencer : public MIDITickComponent {
    public:
        /// The constructor. It raises an exception if in your system there are no MIDI out ports.
        /// \param m a pointer to a MIDIMultiTrack that will hold MIDI messages
        /// \param n a pointer to a MIDISequencerGUINotifier. If you leave 0 the sequencer will not notify
        /// the GUI.
        /// \exception RtMidiError::INVALID_DEVICE if in the system are not present MIDI out ports
                                        MIDISequencer(MIDIMultiTrack* m, MIDISequencerGUINotifier* n = 0);
        /// The destructor. The MIDIMultiTrack and the MIDISequencerGUINotifier are not owned by the MIDISequencer;
        /// the MIDIProcessor objects, instead, (if you have set them with SetProcessor()) are deleted.
        virtual                         ~MIDISequencer();
        /// Resets the MIDISequencer to its initial state. Moves the time to 0 updating the state (see
        /// MIDISequencerState::Reset()), deletes all the track processors (if you have set them with SetProcessor()),
        /// sets all the tracks to MIDI out 0 and no time offset, sets tempo scale to 100.
        /// \note This doesn't affect the multitrack content. If you want to empty it call GetMultiTrack()->Clear()
        /// **before** this (so the sequencer state is correctly updated).
        /// You should call this when the multitrack contents are changed (adding or deleting tracks) to reinitialize
        /// the MIDISequencer according to the new contents.
        virtual void                    Reset();

        //virtual bool                    IsPlaying() const       { return (MIDITickComponent::IsPlaying() || autostop.load()); }
        /// Returns current MIDIClockTime in MIDI ticks; it is effective even during playback
        MIDIClockTime                   GetCurrentMIDIClockTime() const;
        /// Returns current time in milliseconds; it is effective even during playback
        float                           GetCurrentTimeMs() const;
        /// Returns current measure (1st measure is 0).
        unsigned int                    GetCurrentMeasure() const
                                                                { return state.cur_measure; }
        /// Returns current beat in the measure (1st beat is 0).
        unsigned int                    GetCurrentBeat() const  { return state.cur_beat; }
        /// Returns the current MIDI time offset respect to current beat.
        MIDIClockTime                   GetCurrentBeatOffset() const
                                                                { return state.cur_clock - state.last_beat_time; }
        /// Returns the base MIDI ticks per beat ratio of the internal MIDIMultiTrack. Default value is 120 clocks per
        /// quarter beat. However, loading a MIDIFile into the MIDIMultiTrack can change this according to the file
        /// clock.
        unsigned int                    GetClksPerBeat() const  { return state.multitrack->GetClksPerBeat(); }

        /// Returns a pointer to the internal MIDIMultiTrack.
        MIDIMultiTrack*                 GetMultiTrack()         { return state.multitrack; }
        /// Returns a pointer to the internal MIDIMultiTrack.
        const MIDIMultiTrack*           GetMultiTrack() const   { return state.multitrack; }
        /// Returns a pointer to the given track, or 0 if _num_trk is not a valid number.
        MIDITrack*                      GetTrack(unsigned int trk_num)
                                            { return state.multitrack->IsValidTrackNumber(trk_num) ?
                                                     state.multitrack->GetTrack(trk_num) : 0; }
        /// Returns the number of tracks of the multitrack.
        unsigned int                    GetNumTracks() const	{ return state.multitrack->GetNumTracks(); }
        /// Returns current tempo scale in percentage (100 = no scaling, 200 = twice faster, etc.).
        unsigned int                    GetTempoScale() const   { return state.tempo_scale; }
        /// Returns current tempo (BPM) without scaling.
        float                           GetTempoWithoutScale() const
                                                                { return state.tempobpm; }
        /// Returns current tempo (BPM) taking into account scaling (this is the true actual tempo).
        float                           GetTempoWithScale() const
                                                                { return state.tempobpm * state.tempo_scale * 0.01; }
        /// Returns the repeat play (loop) status on/off.
        bool                            GetRepeatPlay() const   { return repeat_play_mode; }
        /// Returns the repeat play (loop) start measure.
        unsigned int                    GetRepeatPlayStart() const
                                                                { return repeat_start_meas; }
        /// Returns the repeat play (loop) end measure.
        unsigned int                    GetRepeatPlayEnd() const
                                                                { return repeat_end_meas; }
        /// Returns **true** if the count in is enabled.
        bool                            GetCountInEnable() const    { return state.playing_status & COUNT_IN_ENABLED; }
        /// Returns **true** if the count in is pending (the sequencer is counting in).
        bool                            GetCountInPending() const   { return state.playing_status & COUNT_IN_PENDING; }

        /// Returns the time shift mode (on or off). This is the value of the internal parameter, **not**
        /// the actual mode (during the playback the Start() method always sets it to on, while at the end
        /// the Stop() method resets it to this value).
        bool                            GetTimeShiftMode() const    { return time_shift_mode; }
        /// Returns a pointer to the current MIDISequencerState (i.e\. the global sequencer state at
        /// current time). You can easily jump from a time to another saving and retrieving sequencer states.
        MIDISequencerState*             GetState()              { return &state; }
        /// Returns a pointer to the current MIDISequencerState (i.e\. the global sequencer state at
        /// current time). You can easily jump from a time to another saving and retrieving sequencer states.
        const MIDISequencerState*       GetState() const        { return &state; }
        /// Returns the play mode state (see SetPlayMode()).
        bool                            GetPlayMode()           { return play_mode; }
        /// Returns a pointer to the MIDISequencerTrackState for a track.
        /// \param trk_num the track number
        MIDISequencerTrackState*        GetTrackState(unsigned int trk_num)
                                                                { return state.track_states[trk_num]; }
        /// Returns a pointer to the MIDISequencerTrackState for a track.
        /// \param trk_num the track number
        const MIDISequencerTrackState*  GetTrackState(unsigned int trk_num) const
                                                                { return state.track_states[trk_num]; }
        /// Returns the number of the out port assigned to a track.
        /// \param trk_num the track number
        unsigned int                    GetTrackOutPort(unsigned int trk_num) const
                                                                { return state.multitrack->GetTrack(trk_num)->GetOutPort(); }
        /// Returns a pointer to the MIDISequencerTrackProcessor for a track.
        /// \param trk_num the track number
        /// \return the processor pointer (if you have already set it with the SetProcessor() method),
        /// otherwise 0
        MIDIProcessor*                  GetTrackProcessor(unsigned int trk_num)
                                                                { return track_processors[trk_num]; }
        /// Returns a pointer to the MIDISequencerTrackProcessor for a track.
        /// \param trk_num the track number
        /// \return the processor pointer (if you have already set it with the SetProcessor() method),
        /// otherwise 0
        const MIDIProcessor*            GetTrackProcessor(unsigned int trk_num) const
                                                                { return track_processors[trk_num]; }
        /// Returns the time offset (in MIDI ticks) assigned to a track.
        /// \see SetTimeOffset(), SetTimeOffsetMode().
        /// \param trk_num the track number
        int                             GetTrackTimeShift(unsigned int trk_num) const
                                                                { return state.multitrack->GetTrack(trk_num)->GetTimeShift(); }
        /// Sets the repeat play (loop) parameters: you can set the repeat play status on/off, the start and the
        /// end measure.
        /// When the repeat play mode is on, the sequencer will start playing from its current position if it is
        /// before the end measure, or from the start measure otherwise. When it reaches the end of the loop it
        /// jumps to the start.
        /// \param on_off can be 0 (repeat play off), 1 (repeat play on) or -1 (leave the state unchanged). The
        /// latter can be useful if you only want to change the start or stop measure
        /// \param start_meas, end_meas you can set the start and end measures (remember that measure numbers start
        /// with 0). If you set an end measure lesser or equal to the start the loop is automatically disabled. If
        /// you leave the default values the measures are left unchanged (useful if you only want to turn on or off the
        /// loop).
        /// \return **true** if parameters are valid (and the mode has been changed), **false** otherwise (the mode
        /// has been set to **false**).
        virtual bool                    SetRepeatPlay(int on_off, int start_meas = -1, int end_meas = -1);
        /// Sets the count in enable or disable.
        virtual void                    SetCountIn(bool on_off);
        /// Sets the global tempo scale.
        /// \param scale the percentage: 100 = no scaling, 200 = twice faster, 50 = twice slower, etc.
        /// \return **true** if _scale_ is a valid number, **false** otherwise (actually only if it is 0).
        virtual bool                    SetTempoScale(unsigned int scale);
        /// Sets the time shifting of events on and off. If you are editing the multitrack events you probably
        /// want to see the original (not shifted) MIDI time of events, while during playback you want them
        /// shifted. So you can turn time shifting on and off (and all the time related methods of the sequencer
        /// will return the shifted or the original time of events). The Start() method sets time shifting on,
        /// while the Stop() resets it to your choice, so usually you can leave time shifting off.
        virtual void                    SetTimeShiftMode(bool f);
        /// Copies a given MIDISequencerState into the internal sequencer state. This method is thread-safe and
        /// can be called during playback. Notifies the GUI a GROUP_ALL event to signify a full GUI reset.
        /// \param s a pointer to the new state.
        /// \note  You can save and restore the sequencer states for quickly jumping from a time to another;
        /// however you should avoid to save a state, edit the multitrack events and then restore the old state,
        /// because you can get inconsistent state parameters.
        virtual void                    SetState(MIDISequencerState* s);
        /// Sets the play mode.
        /// \param mode an enum value with the following meaning:
        /// + PLAY_BOUNDED: The sequencer auto stops after the last event. You cannot set the 'now' time after it
        /// + PLAY_UNBOUNDED: The sequencer doesn't stops after last event, but continues playing sending only beat
        /// markers events.
        /// \note The play mode affects the behavior of the methods GetNextEvent(), GetNextEventTime(), GetNextEventTimeMs(),
        /// GoToTime(), GoToTimeMs(), GoToMeasure().
        virtual void                    SetPlayMode(int mode);
        /// Sets the MIDI out port for a track. This method is thread-safe and can be called during playback
        /// (in this case the sequencer will send a MIDI AllNotesOff message to the old port).
        /// \param trk_num the track number
        /// \param port the id number of the port (see MIDIManager::GetOutPorts())
        /// \return **true** if parameters are valid (and the port has been changed), **false** otherwise.
        virtual bool                    SetTrackOutPort(unsigned int trk_num, unsigned int port);
        /// Sets a MIDIProcessor for the given track. This can't be done while the sequencer is playing
        /// so it stops it.
        /// \param trk_num the track number
        /// \param p a pointer to a MIDIProcessor; calling this with _p_ = 0 sets the track to no processor
        /// \note the Reset() method deletes all processors set by this method.
        /// \return **true** if _trk_num_ is valid (and the processor has been changed), **false** otherwise.
        virtual bool                    SetTrackProcessor(unsigned int trk_num, MIDIProcessor* p);
        /// Sets the time shift offset (in MIDI ticks) for a track. The offset can be positive or negative; events
        /// shifted include all channel messages and sysex messages (others remain at their time).
        /// If you select a negative offset, be sure not to have shifted events at lesser time than the offset
        /// (they won't be shifted). This method is thread-safe and can be called during playback.
        /// \see SetTimeOffsetMode().
        /// \param trk_num the track number
        /// \param offset the offset in MIDI ticks
        /// \return **true** if _trk_num_ is valid (and the offset has been changed), **false** otherwise.
        virtual bool                    SetTrackTimeShift(unsigned int trk_num, int offset);

        /// Inserts into the internal MIDIMultiTrack a new empty track with default track parameters (transpose,
        /// time offset, etc.). This method is thread-safe and can be called during playback. Notifies the GUI a
        /// GROUP_ALL event to signify a full GUI reset.
        /// \param trk_num the track number (it must be in the range 0 ... GetNumTracks() - 1). If you leave the
        /// default value the track will be appended as last.
        /// \return **true** if the track was effectively inserted
        /// \note You should not use the corresponding method of MIDIMultiTrack class, as it does not sync the
        /// iterator and the sequencer internal arrays. If you change the number of tracks directly in the
        /// multitrack (for example when loading a MIDI file) you must then call MIDISequencer::Reset() for
        /// updating the sequencer parameters, but this will reset all track parameters to the default.
        /// \warning if the sequencer is connected to a MIDIRecorder see MIDIRecorder::InsertTrack().
        virtual bool                    InsertTrack(int trk_num = -1);
        /// Deletes a track and all its events from the internal MIDIMultiTrack. This method is thread-safe and can
        /// be called during playback (in this case the sequencer will send a MIDI AllNotesOff message to the old track
        /// port). Notifies the GUI a GROUP_ALL event to signify a full GUI reset.
        /// \param trk_num the track number (must be in the range 0 ... GetNumTracks() - 1). If you leave he default
        /// value the last track wil be deleted.
        /// \return **true** if the track was effectively deleted
        /// \see note to InsertTrack()
        /// \warning if the sequencer is connected to a MIDIRecorder see MIDIRecorder::DeleteTrack().
        virtual bool                    DeleteTrack(int trk_num = -1);
        /// Moves a track from one position to another in the internal MIDIMultiTrack. This method is thread-safe and can
        /// be called during playback (in this case the sequencer will send a MIDI AllNotesOff message to the involved
        /// ports). Notifies the GUI a GROUP_ALL event to signify a full GUI reset.
        /// \param from, to the start and destination track numbers (both must be in the range 0 ... GetNumTracks() - 1).
        /// \return **true** if the track was effectively moved
        /// \see note to InsertTrack()
        /// \warning if the sequencer is connected to a MIDIRecorder see MIDIRecorder::MoveTrack().
        virtual bool                    MoveTrack(int from, int to);
        /// Sets the current time to the beginning of the song, updating the internal status. This method is
        /// thread-safe and can be called during playback. Notifies the GUI a GROUP_ALL event to signify a
        /// full GUI reset.
        virtual void                    GoToZero();
        /// Sets the current time to a given the MIDI time, updating the internal status. This method is thread-safe
        /// and can be called during playback. Notifies the GUI a GROUP_ALL event to signify a full GUI reset
        /// \param time_clk the new time in MIDI ticks
        /// \return **true** if the new time is effectively reached, **false** otherwise (_time_clk_ is after
        /// the end of the song and the play mode is set to PLAY_BOUNDED: in this case the sequencer is leaved
        ///in its original state)
        virtual bool                    GoToTime (MIDIClockTime time_clk);
        /// Same as GoToTime(), but the time is given in milliseconds.
        /// \return see GoToTime()
        virtual bool                    GoToTimeMs (float time_ms);
        /// Sets the current time to the given measure and beat, updating the internal status. This method is
        /// thread-safe and can be called during playback. Notifies the GUI a GROUP_ALL event to signify a GUI reset
        /// \return see GoToTime()
        virtual bool                    GoToMeasure (unsigned int measure, unsigned int beat = 0);
        /// Gets the next event (respect current position). This queries the state for the next event in the
        /// multitrack, then processes it with the corresponding track processor (if you have set it with
        /// SetProcessor)) and updates the state. Moreover it notifies the GUI with appropriate messages. If
        /// there are no events before the next metronome click you will get a Beat Marker internal event.
        /// \param[out] trk_num will return the track number of the next event
        /// \param[out] msg will return the MIDI event
        /// \return **true** if there is effectively a next event (and the parameters are valid), **false** otherwise
        ///
        /// \return **true** if there is effectively a next event (we are not at the end of the song or the
        /// play mode is set to PLAY_UNBOUNDED) , **false** otherwise (in this case *trk_num and *msg are
        /// undefined and the sequencer is leaved in its original state).
        /// \note if we set the play mode to PLAY_UNBOUNDED and go beyond the end of song this will return
        /// the next beat event (on track 0).
        virtual bool                    GetNextEvent (int *trk_num, MIDITimedMessage *msg);
        /// Gets the time of the next event (it can be different from current time if at current time there
        /// are not events).
        /// \param[out] time_clk: will return the requested time in MIDI ticks from the beginning
        /// \return **true** if there is effectively a next event (we are not at the end of the song or the
        /// play mode is set to PLAY_UNBOUNDED) , **false** otherwise (in this case *time_clk is undefined).
        /// \note if we set the play mode to PLAY_UNBOUNDED and go beyond the end of song this will return
        /// the time of the next beat.
        virtual bool                    GetNextEventTime (MIDIClockTime *time_clk);
        /// Same of GetNextEventTime(), but time is returned in milliseconds from the beginning.
        virtual bool                    GetNextEventTimeMs (float *time_ms);
        /// Converts a time from MIDI ticks into milliseconds, taking into account all tempo changes from the
        /// beginning of the song to the given time.
        /// \param time_clk the time to convert
        float                           MIDItoMs(MIDIClockTime time_clk);  // new : added by me
        /// TODO
        MIDIClockTime                   MeasToMIDI(unsigned int meas, unsigned int beat = 0, unsigned int offset = 0);
        /// This is equivalent of GoToTime(state.cur_clock) and should be used to update the sequencer
        /// state after an edit in the multitrack (adding, deleting or editing events, for changes in the track
        /// structure see InsertTrack(), DeleteTrack() and MoveTrack()). If you have edited the multitrack, call
        /// this before moving time, getting events or playing.
        virtual void                    UpdateStatus()  { GoToTime(state.cur_clock); }

        // Inherited from MIDITICK
        /// Starts the sequencer playing from the current time.
        virtual void                    Start();
        /// Stops the sequencer playing.
        virtual void                    Stop();
        /// This is an alias of Start().
        virtual void                    Play()         { Start(); }

        /// Values for the SetMetronomeMode() method.
        enum {
            FOLLOW_MIDI_TIMESIG_MESSAGE,    ///< follow the value stored in the last seen MIDI TimeSig message
            FOLLOW_TIMESIG_DENOMINATOR,     ///< follow the denominator of the time signature
            FOLLOW_THEORETIC_VALUE          ///< follow the music theory value
        };

        /// Selects the way the sequencer calculates metronome beat. You have three choices:
        /// - FOLLOW_MIDI_TIMESIG_MESSAGE  the sequencer uses as metronome beat the value stored in the last seen
        ///   MIDI TimeSig message
        /// - FOLLOW_TIMESIG_DENOMINATOR   the sequencer uses as metronome beat the denominator of the time
        ///   signature (so 4/4 => quarter note, 6/8 -> eighth note etc.)
        /// - FOLLOW_THEORETICAL_VALUE     the sequencer uses as metronome beat the music theory value, taking into
        ///   account simple and composite time signatures (so 4/4 => quarter note, 6/8 => dotted quarter note, etc.)
        ///
        /// Don't use this while the sequencer is playing.
        static void                     SetMetronomeMode(int mode)
                                                        { MIDISequencerState::metronome_mode = mode; }

        /// Values for the play mode.
        enum {
            PLAY_BOUNDED,                   ///< See SetPlayMode()
            PLAY_UNBOUNDED                  ///< See SetPlayMode()
        };

        /// Values for count_in_status
        enum {
            COUNT_IN_ENABLED  = 1,          ///< 0 if no count in before starting, 1 if yes
            COUNT_IN_PENDING  = 2,          ///< 0 if count in is done, 2 if it is pending
            AUTO_STOP_PENDING = 4           ///< 0 if no autostop, 4 if yes
        };

    protected:
        /// Implements the static method inherited by MIDITickComponent and called at every timer tick.
        /// It only calls the member TickProc().
        static void                     StaticTickProc(tMsecs sys_time, void* pt);
        /// Implements the pure virtual method inherited from MIDITickComponent (you must not call it directly).
        virtual void                    TickProc(tMsecs sys_time);
        /// Internal use for auto stop.
        static void                     StaticStopProc(MIDISequencer* p)    { p->Stop(); }

        /// \cond EXCLUDED
        // Internal use: scans events at 'now' time upgrading the sequencer state
        void                            ScanEventsAtThisTime();

        // Internal use: prepares the count in
        void                            CountInPrepare();

        MIDITimedMessage                beat_marker_msg;    // Used by the sequencer to send beat marker messages

        bool                            repeat_play_mode;   // Enables the repeat play mode
        unsigned int                    repeat_start_meas;  // The loop start measure
        unsigned int                    repeat_end_meas;    // The loop end measure
        bool                            time_shift_mode;    // The time shift on/off (during playback time shift is always on)
        int                             play_mode;          // PLAY_BOUNDED or PLAY_UNBOUNDED

        std::vector<MIDIProcessor*>     track_processors;   // A MIDIProcessor for every track
        MIDISequencerState              state;              // The sequencer state
        /// \endcond
};


#endif

