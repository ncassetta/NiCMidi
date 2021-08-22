/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2020  Nicola Cassetta
 *   https://github.com/ncassetta/NiCMidi
 *
 *   This file is part of NiCMidi.
 *
 *   NiCMidi is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   NiCMidi is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with NiCMidi.  If not, see <http://www.gnu.org/licenses/>.
 */


/// \file
/// Contains the definition of the classes MIDIMultiTrack, MIDIMultiTrackIteratorState, MIDIMultiTrackIterator
/// and MIDIEditMultiTrack.


#ifndef MIDI_MULTITRACK_H
#define MIDI_MULTITRACK_H

#include "track.h"

#include <vector>


class MIDIEditMultiTrack;       // forward declaration

///
/// Holds an array of pointers to MIDITrack objects to be played simultaneously. Every track contains
/// MIDITimedMessage objects representing MIDI events, and all tracks share the same timing (i.e. the events are
/// temporized according to the same MIDI clock per beat). Typically track 0 is the master track and contains only
/// non-channel MIDI events (time, tempo, markers ...) while  other tracks contain the channel events. The
/// MIDIMultiTrack object owns its tracks, so deleting it frees them also. You need to embed this into a MIDISequencer
/// for playing the tracks. Moreover, a MIDIMultiTrackIterator class is supplied for moving along events in temporal
/// order regardless the track number.
///
class MIDIMultiTrack {
    public:
        /// The constructor creates an object with given number of tracks (default no track) and base MIDI clocks
        /// per beat.
        /// \param num_tracks The number of tracks of the multitrack. The tracks are created by the constructor and they are
        /// are initially empty (contain only the EOT message at time 0). The default is no track
        /// \param cl_p_b The number of MIDI ticks per quarter note. All tracks share this to convert the raw
        /// MIDIClockTime stored in their MIDITimedMessage objects into musical values.
                                    MIDIMultiTrack(unsigned int num_tracks = 0,
                                                   unsigned int cl_p_b = DEFAULT_CLKS_PER_BEAT);

        /// The copy constructor
                                    MIDIMultiTrack(const MIDIMultiTrack& mlt);
        /// The destructor: The MIDIMultiTrack owns its tracks, so they are destroyed by this.
        virtual	                    ~MIDIMultiTrack();

        /// The assignment operator.
        MIDIMultiTrack&             operator=(const MIDIMultiTrack& mlt);
        /// Deletes all the tracks in the Multitrack, resizes it to the given number of tracks and resets
        /// _clks_per_beat_.
        void                        Reset(unsigned int num_tracks = 0);
        /// Clears tracks events but mantains the tracks and their parameters. If _mantain_end_ is **true**
        /// doesn't change the time of EOT events, otherwise sets them to 0.
        void                        ClearTracks(bool mantain_end = false);

        /// Returns the MIDI clocks per beat of all tracks (i.e.\ the number of MIDI ticks in a quarter note).
        unsigned int                GetClksPerBeat() const          { return clks_per_beat; }
        /// Returns the pointer to the track
        /// \param trk_num The track number
        MIDITrack*                  GetTrack(int trk_num)           { return tracks[trk_num]; }
        /// Returns the pointer to the track
        /// \param trk_num The track number
        const MIDITrack*            GetTrack(int trk_num) const     { return tracks[trk_num]; }
        /// Returns the number of the pointed track, -1 if the track is not in the multitrack.
        int                         GetTrackNum(MIDITrack* trk) const;
        /// Returns the number of allocated tracks.
        unsigned int	            GetNumTracks() const			{ return tracks.size(); }
        /// Returns the number of tracks with events (other than EOT).
        unsigned int                GetNumTracksWithEvents() const;
        /// Returns the total number of MIDI events in the multitrack (for every track there is at least the EOT).
        /// If you want to know if the MIDIMultiTrack is empty, use GetNumTracksWithEvents() instead.
        unsigned int                GetNumEvents() const;
        /// Returns the end time of the longest track.
        MIDIClockTime               GetEndTime() const;
        /// Returns **true** if _trk_num_ is in thee range 0 ... GetNumTracks() - 1.
        bool                        IsValidTrackNumber(int trk_num) const
                                                                    { return (0 <= trk_num && (unsigned)trk_num < tracks.size()); }
        /// Returns **true** if there are no events in the tracks and the end time is 0.
        bool                        IsEmpty() const                 { return (GetNumEvents() == 0 && GetEndTime() == 0); }

        /// Changes the value of the clock per beat parameter for the tracks, updating the times of all MIDI
        /// events. This may lead to loss in precision or rounding error if the new clocks per beat
        /// is not a multiple of the old, so it's better to call this function before inserting any event in
        /// the multitrack.
        /// \param cl_p_b see the constructor
        void                        SetClksPerBeat(unsigned int cl_p_b);
        /// Sets the time of the data end event to _end_time_. If there are events of other type after
        /// _end_time_ the function fails and returns **false**.
        bool                        SetEndTime(MIDIClockTime end_time);

        /// Sets the time of the data end event equal to the time of the last event of every track.
        void                        ShrinkEndTime();

        /// This function is useful in dealing with MIDI format 0 files (with all events in an unique track).
        /// It remakes the MIDIMultiTrack object with 17 tracks (_src_ track can be a member of multitrack object
        /// himself), moves _src_ track channel events to tracks 1-16 according their channel, and all other types
        /// of events to track 0. This is automatically called when loading a MIDI format 0 file.
        void                        AssignEventsToTracks (const MIDITrack *src);
        /// The same as previous, but argument is the track number in the multitrack object himself
        void                        AssignEventsToTracks (int trk_num = 0)
                                            { return AssignEventsToTracks(GetTrack(trk_num)); }
        /// Inserts a new empty track at position _trk_num_ (_trk_num_ must be in the range 0 ...\ GetNumTracks() - 1).
        /// If _trk_num_ == -1 appends the track at the end.
        /// \return **true** if the track was effectively inserted.
        /// \warning if you are using the MIDIMultiTrack into a MIDISequencer class don't call this directly
        /// but the corresponding MIDISequencer method (which adjust the MIDISequencer internal values also).
        bool                        InsertTrack(int trk_num = -1);
        /// Deletes the track _trk_num_ and its events. (_trk_num_ must be in the range 0 ... GetNumTracks() - 1).
        /// If _trk_num_ == -1 deletes the last track.
        /// \return **true** if the track was effectively deleted
        /// \see warning in the InsertTrack() method
        bool                        DeleteTrack(int trk_num = -1);
        /// Moves a track from the position _from_ to the position _to_. ( _from_ and _to_ must be in the range
        /// 0 ... GetNumTracks() - 1).
        /// \return **true** if the track was effectively moved
        /// \see warning in the InsertTrack() method
        bool                        MoveTrack(int from, int to);

        /// Inserts the event _msg_ in the track _trk_num_. See MIDITrack::InsertEvent() for details.
        /// \return **true** if the event was effectively inserted
        bool                        InsertEvent(int trk_num,  const MIDITimedMessage& msg, tInsMode _ins_mode = INSMODE_DEFAULT);
        /// Inserts a Note On and a Note Off event into the track _trk_num_. See MIDITrack::InsertNote() for details.
        bool                        InsertNote(int trk_num, const MIDITimedMessage& msg,
                                            MIDIClockTime len, tInsMode _ins_mode = INSMODE_DEFAULT);
        /// Deletes the event _msg_ from the track _trk_num_. See MIDITrack::DeleteEvent() for details.
        bool                        DeleteEvent(int trk_num,  const MIDITimedMessage& msg);
        /// Deletes the note _msg_ (_msg_ must be a Note On) from the track _trk_num. See MIDITrack::DeleteNote() for details.
        bool                        DeleteNote(int trk_num, const MIDITimedMessage& msg);


        void                        EditCopy(MIDIClockTime start, MIDIClockTime end, int tr_start,
                                                int tr_end, MIDIEditMultiTrack* edit);
                                        // makes a new interval in *edit, with tracks from tr_start to tr_end
        void                        EditCut(MIDIClockTime start, MIDIClockTime end, MIDIEditMultiTrack* edit);
                                        // deletes events and shifts subsequents (only on entire multitrack)
        void                        EditClear(MIDIClockTime start, MIDIClockTime end, int tr_start, int tr_end);
                                        // erase events in tracks tr_start ... tr_end
        void                        EditInsert(MIDIClockTime start, int tr_start, int times,
                                            bool sysex, MIDIEditMultiTrack* edit);
                                        // insert interval <edit> in <time_start>
                                        // (if <edit> == 0 insert a blank interval)
        void                        EditReplace(MIDIClockTime start, int tr_start, int times,
                                                bool sysex, MIDIEditMultiTrack* edit);

    protected:
        unsigned int 	            clks_per_beat;      ///< The common clock per beat timing parameter for all tracks
                                                        ///< (this is the number of MIDI ticks for a quarter note).
        /// \cond EXCLUDED
        std::vector<MIDITrack*>     tracks;             // The array of pointers to the MIDITrack objects
        /// \endcond
};


///
/// Used by the MIDIMultiTrackIterator to keep track of the current state of the iterator. It
/// remembers the current time and the next event in every track. The user does not need to
/// deal with it, and the only useful thing is getting and restoring a state for faster processing (see
/// MIDIMultiTrack::SetStatus()), so the details are not documented.
///
class MIDIMultiTrackIteratorState {
/// \cond EXCLUDED
    // This is used only by the MIDIMultiTrackIterator, so all is protected
    friend class MIDIMultiTrackIterator;

    public:
        // The constructor.
                                    MIDIMultiTrackIteratorState(int n_tracks);

                    // . . . no need for dtor and others . . .
    protected:
        // Changes the number of tracks (this causes a reset of the state).
        void                        SetNumTracks(int n);
        // Sets the time to 0 and an undefined first event and track.
        // \warning this is, in general, **not** a valid state. Don't call this but MIDIMultiTrackIterator::Reset().
        void                        Reset();
        // Internal use.
        int                         FindTrackOfFirstEvent();

        int                         num_tracks;         // The number of tracks
        MIDIClockTime               cur_time;           // The current time
        int                         cur_event_track;    // The track of the next event
        std::vector<int>            next_event_number;  // Array holding the next event for every track
        std::vector<MIDIClockTime>  next_event_time;    // Array holding the next event time for every track
        bool                        time_shift_mode;    // Time shift on/off
        /// \endcond
};


///
/// A forward iterator for moving along a MIDIMultiTrack. It defines a current time (initially 0) and a
/// current event, and stores its status in a MIDIMultiTrackIteratorState object. You can jump to any time in
/// the multitrack and get its events in chronological order, starting with the first event with time greater
/// than or equal to the current time, regardless of their track.
/// \note This class also handles the time shifting of track events. For this it relies on an external
/// **std::vector<int>** which must be passed by address to the SetTimeShiftVector() method. The class only reads
/// the values in this vector and never changes it: you can turn time shifting on and off but not set the values
/// for the tracks. This "strange" design is intended to allow a higher level class (typically the MIDISequencer)
/// to handle the time shift, without overloading it.
///
class MIDIMultiTrackIterator {
    public:

        /// The constructor creates the object and attaches it to the given MIDIMultiTrack.
                                    MIDIMultiTrackIterator (MIDIMultiTrack *mlt);
        /// Syncs _num_tracks_ with the multitrack and resets time to 0.
        void                        Reset();
        /// Gets the current time of the iterator.
        MIDIClockTime               GetCurrentTime() const              { return state.cur_time; }
        /// Returns **true** if time shifting is on.
        bool                        GetTimeShiftMode() const            { return state.time_shift_mode; }
        /// Gets the current MIDIMultiTrackIteratorState. You can save and then restore it for a
        /// faster processing in GoTo operations (admitting the contents of the multitrack are not
        /// changed).
        MIDIMultiTrackIteratorState&        GetState()                  { return state; }
         /// Gets the current MIDIMultiTrackIteratorState. You can save and then restore it for a
        /// faster processing in GoTo operations (admitting the contents of the multitrack are not
        /// changed).
        const MIDIMultiTrackIteratorState&  GetState() const            { return state; }
        /// Turns time shifting on and off. If time shifting is off events are sorted according to their
        /// time, otherwise an offset (positive or negative) is added to channel and sysex events time (other
        /// events time remain unchanged).
        /// \return **true** if the function succeeded (it could fail if you try to set the mode on without having
        /// set a vector).
        void                        SetTimeShiftMode(bool f);
        /// Sets the given MIDIMultiTrackIteratorState as current state.
        void                        SetState(const MIDIMultiTrackIteratorState& s) { state = s; }
        /// Goes to the given time, which becomes the current time, and sets then the current event as the
        /// first event (in any track) with time greater or equal to _time_. If there are more events with
        /// same time in different tracks their order is not defined, as the iterator tries to rotate
        /// across the tracks rather than to get first all events in a single track. However, temporal order
        /// is always granted.
        /// \return **true** if the given time is effectively reached, *false* otherwise (it is after the end
        /// of the multitrack)
        bool                        GoToTime(MIDIClockTime time);
        /// Gets the next event in the multitrack in temporal order and updates the iterator state. If the
        /// time shifting mode is on the events are sorted according to their shifted time.
        /// \param track the track of the event, if valid.
        /// \param *msg a pointer to the event in the MidiMultiTrack
        /// \return **true** if there is effectively a next event (we are not at the end of the
        /// MIDIMultiTrack), **false** otherwise (and *track and **msg contain undefined values).
        bool                        GetNextEvent(int *track, MIDITimedMessage **msg);
        /// Gets the next event in the multitrack on track _track_ and updates the iterator state.
        /// \param track the track of the event, if valid.
        /// \param *msg a pointer to the event in the MidiMultiTrack
        /// \return **true** if there is effectively a next event (we are not at the end of the
        /// track), **false** otherwise (and **msg contains an undefined value).
        bool                        GetNextEventOnTrack(int track, MIDITimedMessage **msg);
        /// Gets the time of the next event in the multitrack (it can be different from current time if
        /// at current time there are not events). This does not change the iterator state.
        /// \param t here we get the time of next event, if valid. If the time shifting mode is on it
        /// is the shifted time.
        /// \return **true** if there is effectively a next event (we aren't at the end of the
        /// MIDIMultiTrack, **false** otherwise (*t doesn't contain a valid time).
        bool                        GetNextEventTime(MIDIClockTime *t) const;
        // Returns a pointer to the MIDIMultiTrack the iterator is attached to.
        //MIDIMultiTrack*             GetMultiTrack()  				    { return multitrack; }
        //const MIDIMultiTrack*       GetMultiTrack() const 	            { return multitrack; }

    protected:
        /// \cond EXCLUDED
        // Returns the time of the given MIDITimedMessage in the given track, taking into account the time
        // shifting. This is the same time of the message if time shifting is off or _msg_ is not a channel
        // or sysex message, otherwise this will return the message time plus the track offset.
        MIDIClockTime               GetShiftedTime(const MIDITimedMessage* msg, int trk);

        MIDIMultiTrack*             multitrack;     // The MIDIMultiTrack the class refers to
        MIDIMultiTrackIteratorState state;          // The iterator state
        /// \endcond
};



class MIDIEditMultiTrack : public MIDIMultiTrack {
    public:
                                    MIDIEditMultiTrack(int cl_p_b = DEFAULT_CLKS_PER_BEAT) :
                                    MIDIMultiTrack(cl_p_b), start_track(0), end_track(0) {}
        virtual	                    ~MIDIEditMultiTrack() {}

        void                        SetStartTrack(int trk)      { start_track = trk; }
        void                        SetEndTrack(int trk)        { end_track = trk; }
        int                         GetStartTrack() const       { return start_track; }
        int                         GetEndTrack() const         { return end_track; }

                    // these must be disabled in derived class: so they are declared but not implemented
        void                        EditCopy(MIDIClockTime start, MIDIClockTime end, int tr_start,
                                                int tr_end, MIDIEditMultiTrack* edit) {}
        void                        EditCut(MIDIClockTime start, MIDIClockTime end, MIDIEditMultiTrack* edit) {}
        void                        EditClear(MIDIClockTime start, MIDIClockTime end, int tr_start, int tr_end) {}
        void                        EditInsert(MIDIClockTime start, int tr_start, int times,
                                                bool sysex, MIDIEditMultiTrack* edit) {}
        void                        EditReplace(MIDIClockTime start, int tr_start, int times,
                                                bool sysex, MIDIEditMultiTrack* edit) {}
                    ///////////////////


        //void                    CopyTrack(int n, MIDIMultiTrack* trk);
                                // use operator= instead!
        void                        CopyAll(MIDIMultiTrack* m);
                                // does a Clear and copy

    private:

        int 	                    start_track;
        int                         end_track;


};


#endif
