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



#ifndef MIDI_MULTITRACK_H
#define MIDI_MULTITRACK_H

#include "track.h"

#include <vector>


class MIDIEditMultiTrack;

///
/// This class holds an array of pointers to MIDITrack objects to be played simultaneously. Every track contains
/// MIDITimedMessage objects representing MIDI events, and all tracks share the same timing (i.e. the events are
/// temporized according to the same MIDI clock per beat). Tipically track 0 is the master track and contains only
/// non-channel MIDI events (time, tempo, markers ...) while  other tracks contain the channel events. The
/// MIDIMultiTrack object owns its tracks, so deleting it frees them also. You need to embed this into a MIDISequencer
/// for playing the tracks. Moreover, a MIDIMultiTrackIterator class is supplied for moving along events in temporal
/// order regardless the track number.
///
class MIDIMultiTrack {
    public:
        /// The constructor creates an object with given number of tracks (default no track) and base MIDI clocks
        /// per beat. Created tracks are initially empty (contain only the EOT message at time 0).
                                    MIDIMultiTrack(unsigned int num_tracks = 0,
                                                   unsigned int cl_p_b = DEFAULT_CLKS_PER_BEAT);

        /// The copy constructor
                                    MIDIMultiTrack(const MIDIMultiTrack& mlt);
        /// The destructor frees the track pointers.
        virtual	                    ~MIDIMultiTrack();

        /// The assignment operator
        MIDIMultiTrack&             operator=(const MIDIMultiTrack& mlt);

        /// Changes the value of the clock per beat parameter for the tracks, updating the times of all MIDI
        /// events. This may lead to loss in precision or rounding error if the new clocks per beat
        /// is not a multiple of the old, so it's better to call this function before inserting any event in
        /// the multitrack.
        void                        SetClksPerBeat(unsigned int cl_p_b);
        /// Returns the MIDI clocks per beat of all tracks (i.e.\ the number of MIDI ticks in a quarter note).
        unsigned int                GetClksPerBeat() const          { return clks_per_beat; }
        /// Returns a pointer to the track _track_num_.
        MIDITrack*                  GetTrack(int trk)               { return tracks[trk]; }
        const MIDITrack*            GetTrack(int trk) const         { return tracks[trk]; }
        /// Returns the number of allocated tracks.
        unsigned int	            GetNumTracks() const			{ return tracks.size(); }
        /// Returns the number of tracks with events (other than EOT).
        unsigned int                GetNumTracksWithEvents() const;
        /// Gets the total number of MIDI events in the multitrack (for every track there is at least the EOT).
        /// If you want to know if the MIDIMultiTrack is empty, use GetNumTracksWithEvents() instead.
        unsigned int                GetNumEvents() const;
        /// Returns *true* if _n_ is in thee range 0 ... GetNumTracks() - 1.
        bool                        IsValidTrackNumber(int trk) const
                                                                    { return (0 <= trk && (unsigned)trk < tracks.size()); }
        /// Returns the end time of the longest track.
        MIDIClockTime               GetEndTime() const;
        /// Deletes all tracks leaving the multitrack empty.
        void                        Clear();
        /// Clears all content in the Multitrack and resize it to the given number of tracks
        void                        ClearAndResize(unsigned int num_tracks);
        /// Clears tracks events but mantains the tracks (leaves only the EOT). If _mantain_end_ is *true*
        /// doesn't change the time of EOT events, otherwise sets them to 0.
        void                        ClearTracks(bool mantain_end = false);

        /// This function is useful in dealing with MIDI format 0 files (with all events in an unique track).
        /// It remakes the MIDIMultiTrack object with 17 tracks (_src_ track can be a member of multitrack obiect
        /// himself), moves _src_ track channel events to tracks 1-16 according their channel, and all other types
        /// of events to track 0. This is automatically done when loading a MIDI format 0 file.
        void                        AssignEventsToTracks (const MIDITrack *src);
        /// The same as previous, but argument is track number of multitrack object himself
        void                        AssignEventsToTracks (int trk = 0)
                                            { return AssignEventsToTracks(GetTrack(trk)); }
        /// Finds the channel of the first MIDIchannel event in the track. This is probably the channel of all
        /// other channel events.
        /// \return -1 if the track is empty or doesn't contain channel events (the main track, for example).
        /// Otherwise the channel range is 0 ... 15.
        //int                         FindFirstChannelOnTrack(int trk) const;
        /// Inserts a new empty track at position _trk_ (_trk_ must be in the range 0 ... GetNumTracks() - 1). If
        /// _trk_ == -1 appends the track at the end.
        /// \return *true* if the track was effectively inserted.
        /// \warning if you are using the MIDIMultiTrack into a MIDISequencer class don't call this directly
        /// but the corresponding MIDISequencer method (which adjust the MIDISequencer internal values also).
        bool                        InsertTrack(int trk = -1);
        /// Deletes the track _trk_ and its events. _trk_ must be in the range 0 ... GetNumTracks() - 1.
        /// \return *true* if the track was effectively deleted
        /// \see warning in the InsertTrack() method
        bool                        DeleteTrack(int trk);
        /// Moves a track from the position _from_ to the position _to_. ( _from_ e _to_ must be in the range
        /// 0 ... GetNumTracks() - 1).
        /// \return *true* if the track was effectively moved
        /// \see warning in the InsertTrack() method
        bool                        MoveTrack(int from, int to);

        /// Inserts the event _msg_ in the track _trk_. See MIDITrack::InsertEvent() for details.
        /// \return *true* if the event was effectively inserted
        bool                        InsertEvent(int trk,  const MIDITimedMessage& msg, int _ins_mode = INSMODE_DEFAULT);
        /// Inserts a Note On and a Note Off event into the track. See MIDITrack::InsertNote() for details.
        bool                        InsertNote(int trk, const MIDITimedMessage& msg,
                                            MIDIClockTime len, int _ins_mode = INSMODE_DEFAULT);
        /// Deletes the event _msg_ from the track _trk_. See MIDITrack::DeleteEvent() for details.
        bool                        DeleteEvent(int trk,  const MIDITimedMessage& msg);
        /// Deletes the note _msg_ (_msg_ must be a Note On) from the track _trk_. See MIDITrack::DeleteNote() for details.
        bool                        DeleteNote(int trk, const MIDITimedMessage& msg);


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

        /// The default clocks per beat parameter
        static const int            DEFAULT_CLKS_PER_BEAT = 120;
    private:

        int 	                    clks_per_beat;      ///< The common clock per beat timing parameter
        std::vector<MIDITrack*>     tracks;             ///< The array of pointers to the MIDITrack objects
};


///
/// This class is used by the MIDIMultiTrackIterator to keep track of the current state of the iterator. It
/// remembers the current time and keep track of the next event in every track. You usually don't need to
/// deal with it, and the only useful thing is getting and restoring a state for faster processing (see
/// MIDIMultiTrack::SetStatus()).
/// \note This class is responsible for the time shifting of the track events. For this it relies on an external
/// std::vector<int> which must be passed by address by the SetTimeShiftMode() method. The class only reads values
/// in this vector and never modifies it; when the class is copied only the address is copied. This "strange" design
/// is in order to allow an higher-level class (tipically the MIDISequencer) to handle time shifting, without
/// overcharging this.
///
class MIDIMultiTrackIteratorState {
    friend class MIDIMultiTrackIterator;

    public:
        /// The constructor.
                                    MIDIMultiTrackIteratorState(int n_tracks);

                    // . . . no need for dtor and others . . .

        /// Gets the number of tracks of the state.
        int	                        GetNumTracks() const	                { return num_tracks; }
        /// Gets the track of the next event.
        int	                        GetCurEventTrack() const    			{ return cur_event_track; }
        /// Gets current time.
        MIDIClockTime               GetCurTime() const				    	{ return cur_time; }
        /// Changes the number of tracks (this causes a reset of the state).
        void                        SetNumTracks(int n);
        /// Sets the time to 0 and an undefined first event and track.
        /// \warning this is, in general, *not* a valid state. Don't call this but MIDIMultiTrackIterator::Reset().
        void                        Reset();

        /// Turns time shifting on and off. If time shifting is off events are sorted according to their
        /// time, otherwise an offset (positive or negative) is added to channel and sysex events time (other
        /// events time remain unchanged). The first time you call this you must set the external vector which
        /// holds track offsets
        /// \return *true* if the function succeeded (it could fail if you try to set the mode on without having
        /// set a vector, of you try to change the vector address while the mode is on).
        bool                        SetTimeShiftMode(bool f, std::vector<int>* v = 0);
        /// Returns *true* if time shifting is on.
        bool                        GetTimeShiftMode() const                { return time_shift_mode; }
        /// Returns the time of the given MIDITimedMessage in the given track, taking into account the time
        /// shifting. This the same time of the message if time shifting is off or _msg_ is not a channel
        /// or sysex message, otherwise this will return the message time plus the track offset.
        MIDIClockTime               GetShiftedTime(const MIDITimedMessage* msg, int trk);

    private:
        int                         FindTrackOfFirstEvent();

        int                         num_tracks;
        MIDIClockTime               cur_time;
        int                         cur_event_track;
        std::vector<int>            next_event_number;
        std::vector<MIDIClockTime>  next_event_time;
        std::vector<int>*           time_shifts;
        bool                        time_shift_mode;
};


///
/// This class is a forward iterator for moving along a MIDIMultiTrack. It defines a current time (initially 0)
/// and a current event. You can skip to any time in the MultiTrack and get its events in temporal order, beginning
/// with the first event with time greater or equal current time, regardless their track.
/// When the iterator reaches the end of the multitrack the current event become undefined, and the get methods
/// will return **false**.
///
class MIDIMultiTrackIterator {
    public:

        /// The constructor creates the object and attaches it to the given MIDIMultiTrack.
                                    MIDIMultiTrackIterator (MIDIMultiTrack *mlt);
        /// Syncs _num_tracks_ with the multitrack and resets time to 0.
        void                        Reset();
        /// Gets the current time of the iterator.
        MIDIClockTime               GetCurrentTime() const              { return state.GetCurTime(); }
        /// Gets the current MIDIMultiTackIteratorState. You can save and then restore it for a
        /// faster processing in GoTo operations (admitting the contents of the multitrack are not
        /// changed).
        ///<{
        MIDIMultiTrackIteratorState&        GetState()                  { return state; }
        const MIDIMultiTrackIteratorState&  GetState() const            { return state; }
        ///<}
        /// Sets the given MIDIMultiTrackIteratorState as current state.
        void                        SetState(const MIDIMultiTrackIteratorState& s) { state = s; }
        /// Goes to the given time, which becomes the current time, and sets then the current event as the
        /// first event (in any track) with time greater or equal to _time_. If there are more events with
        /// same time in different tracks their order is not defined, as the iterator tries to rotate
        /// across the tracks rather than to get first all events in a single track. However, temporal order
        /// is always granted.
        /// \return *true* if the given time is effectively reached, *false* otherwise (it is after the end
        /// of the multitrack)
        bool                        GoToTime(MIDIClockTime time);
        /// Gets the next event in the multitrack in temporal order and updates the iterator state. If the
        /// state has time shifting mode on the events are sorted according to their shifted time.
        /// \param track the track of the event, if valid.
        /// \param *msg a pointer to the event in the MidiMultiTrack
        /// \return *true* if there is effectively a next event (we aren't at the end of the
        /// MIDIMultiTrack), *false* otherwise (*track and **msg don't contain undefined values).
        bool                        GetNextEvent(int *track, MIDITimedMessage **msg);
        /// Discards the current event and set as current the subsequent. If the iterator state has time
        /// shifting mode on events are sorted according to their shifted time.
        /// @return *true* if there is effectively a next event (we aren't at the end of the
        /// MIDIMultiTrack, *false* otherwise.
        //bool                        GoToNextEvent() {}
        /// Set as current the next event on track _track_.
        /// @return *true* if there is effectively a next event (we aren't at the end of the
        /// MIDIMultiTrack, *false* otherwise.
        bool                        GetNextEventOnTrack(int track, MIDITimedMessage **msg);
        /// Gets the time of the next event in the multitrack (it can be different from current time if
        /// at current time there are not events). If the iterator state has time shifting mode on it
        /// is the shifted time (see MIDIMultiTrackIteratorState::GetShiftedTime().
        /// \param t here we get the time of next event, if valid.
        /// \return *true* if there is effectively a next event (we aren't at the end of the
        /// MIDIMultiTrack, *false* otherwise (*t doesn't contain a valid time).
        bool                        GetNextEventTime(MIDIClockTime *t) const;
        // Returns a pointer to the MIDIMultiTrack the iterator is attached to.
        //MIDIMultiTrack*             GetMultiTrack()  				    { return multitrack; }
        //const MIDIMultiTrack*       GetMultiTrack() const 	            { return multitrack; }

    protected:
        MIDIMultiTrack*             multitrack;     ///< The MIDIMultiTrack the class refers to
        MIDIMultiTrackIteratorState state;          ///< The iterator state
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
