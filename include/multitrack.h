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

//#include "world.h"
#include "track.h"

#include <vector>


class MIDIEditMultiTrack;


///
/// This class holds an array of pointers to MIDITrack objects to be played simultaneously. Every track contains
/// MIDITimedBigMessage objects representing MIDI events, and all tracks share the same timing (i.e. the events are
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
                                MIDIMultiTrack(unsigned int number_of_tracks = 0,
                                               unsigned int cl_p_b = DEFAULT_CLKS_PER_BEAT);

            /// The destructor frees the track pointers.
        virtual	               ~MIDIMultiTrack();

            /// Changes the value of the clock per beat parameter for the tracks, updating the times of all MIDI
            /// events. This may lead to loss in precision or rounding error if the new _clocks_p_b
            /// is not a multiple of the old, so it's better to call this function before inserting any event in
            /// the multitrack.
        void                    SetClksPerBeat(unsigned int cl_p_b);
            /// Returns the MIDI clocks per beat of all tracks (i.e.\ the number of MIDI ticks in a quarter note).
        unsigned int            GetClksPerBeat() const          { return clks_per_beat; }
            /// Returns a pointer to the track _track_num_
        MIDITrack*              GetTrack( int trk )             { return tracks[trk]; }
        const MIDITrack*        GetTrack( int trk ) const       { return tracks[trk]; }
            /// Returns the number of allocated tracks.
        unsigned int	        GetNumTracks() const			{ return tracks.size(); }
            /// Returns the number of tracks with events. This is probably the number of tracks effectively used by
            /// your MIDIMultiTrack.
        unsigned int            GetNumTracksWithEvents() const;
            /// Gets the total number of MIDI events in the multitrack.
        unsigned int            GetNumEvents() const;
            /// Returns *true* if _n_ is in thee range 0 ... GetNumTracks() - 1.
        bool                    IsValidTrackNumber(int n) const{ return (0 <= n && (unsigned)n < tracks.size()); }
            /// Deletes all tracks leaving the multitrack empty.
        void                    Clear();
            /// Clears all content in the Multitrack and resize it to the given number of tracks
        void                    ClearAndResize(unsigned int n);
            /// Clears tracks events but mantains the tracks. If _mantain_end_ is *true* doesn't change the time of
            /// EOT events, otherwise sets them to 0.
        void                    ClearTracks(bool mantain_end = false);

            /// This function is useful in dealing with MIDI format 0 files (with all events in an unique track).
            /// It remakes the MIDIMultiTrack object with 17 tracks (_src_ track can be a member of multitrack obiect
            /// himself), moves _src_ track channel events to tracks 1-16 according their channel, and all other types
            /// of events to track 0.
        void                    AssignEventsToTracks ( const MIDITrack *src );

            /// The same as previous, but argument is track number of multitrack object himself
        void                    AssignEventsToTracks (int track_num = 0)
                                        { return AssignEventsToTracks(GetTrack(track_num)); }

            /// Inserts a new empty track at position _trk_ (_trk_ must be in the range 0 ... GetNumTracks() - 1). If
            /// _trk_ == -1 append the track at the end.
            /// \return *true* if the track was effectively inserted
        bool                    InsertTrack(int trk = -1);

            /// Deletes the track _trk_ and its events. _trk_ must be in the range 0 ... GetNumTracks() - 1.
            /// \return *true* if the track was effectively deleted
        bool                    DeleteTrack(int trk);

            /// Moves a track from the position _from_ to the position _to_. ( _from_ e _to_ must be in the range
            /// 0 ... GetNumTracks() - 1).
            /// \return *true* if the track was effectively moved
        bool                    MoveTrack(int from, int to);


            /// Inserts the event _msg_ in the track _trk_. See MIDITrack::InsertEvent() for details.
            /// \return *true* if the event was effectively inserted
        bool                    InsertEvent(int trk,  const MIDITimedMessage& msg, int _ins_mode = INSMODE_DEFAULT);

            /// Inserts a Note On and a Note Off event into the track. See MIDITrack::InsertNote() for details.
        bool                    InsertNote(int trk, const MIDITimedMessage& msg,
                                           MIDIClockTime len, int _ins_mode = INSMODE_DEFAULT);

            /// Deletes the event _msg_ from the track _trk_. See MIDITrack::DeleteEvent() for details.
        bool                    DeleteEvent(int trk,  const MIDITimedMessage& msg);

            /// Deletes the note _msg_ (_msg_ must be a Note On) from the track _trk_. See MIDITrack::DeleteNote() for details.
        bool                    DeleteNote( int trk, const MIDITimedMessage& msg );


        void                    EditCopy(MIDIClockTime start, MIDIClockTime end, int tr_start,
                                             int tr_end, MIDIEditMultiTrack* edit);
                                        // makes a new interval in *edit, with tracks from tr_start to tr_end
        void                    EditCut(MIDIClockTime start, MIDIClockTime end, MIDIEditMultiTrack* edit);
                                        // deletes events and shifts subsequents (only on entire multitrack)
        void                    EditClear(MIDIClockTime start, MIDIClockTime end, int tr_start, int tr_end);
                                        // erase events in tracks tr_start ... tr_end
        void                    EditInsert(MIDIClockTime start, int tr_start, int times,
                                           bool sysex, MIDIEditMultiTrack* edit);
                                        // insert interval <edit> in <time_start>
                                        // (if <edit> == 0 insert a blank interval)
        void                    EditReplace(MIDIClockTime start, int tr_start, int times,
                                            bool sysex, MIDIEditMultiTrack* edit);

    private:

        int 	                clks_per_beat;      ///< The common clock per beat timing parameter
        std::vector<MIDITrack*> tracks;             ///< The array of pointers to the MIDITrack objects
};


///
/// This class is used by the MIDIMultiTrackIterator to keep track of the current state of the iterator. You
/// usually don't need to deal with it, and the only useful thing is getting and restoring a state for faster
/// processing (see MIDIMultiTrackk::SetStatus()).
///
class MIDIMultiTrackIteratorState {
    friend class MIDIMultiTrackIterator;

    public:
            /// The constructor.
                                MIDIMultiTrackIteratorState(int n_tracks);
            /// The copy constructor.
                                MIDIMultiTrackIteratorState(const MIDIMultiTrackIteratorState &m);
            /// The destructor.
        virtual                ~MIDIMultiTrackIteratorState();
            /// The equal operator.
        const MIDIMultiTrackIteratorState& operator=(const MIDIMultiTrackIteratorState &m);
            /// Gets the number of tracks of the state.
        int	                    GetNumTracks() const	                { return num_tracks; }
            /// Gets the track of the next event.
        int	                    GetCurEventTrack() const    			{ return cur_event_track; }
            /// Gets current time.
        MIDIClockTime           GetCurTime() const				    	{ return cur_time; }
            /// Changes the number of tracks (this causes a reset of the state).
        void                    SetNumTracks(int n);
            /// Sets the time to 0 and an undefined first event and track.
            /// @warning this is, in general, *not* a valid state. Don't call this but MIDIMultiTrackIterator::Reset().
        void                    Reset();
            /// The MIDIMultiTrackIterator calls this after an event get to find the track of the next
            /// event
        int                     FindTrackOfFirstEvent();

    private:
        int                     num_tracks;
        MIDIClockTime           cur_time;
        int                     cur_event_track;
        int*                    next_event_number;
        MIDIClockTime*          next_event_time;
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
                                MIDIMultiTrackIterator( MIDIMultiTrack *mlt );
            /// The destructor.
        virtual                ~MIDIMultiTrackIterator();
            /// Syncs _num_tracks_ with the multitrack and resets time to 0.
        void                    Reset();
            /// Goes to the given time, which becomes the current time, and sets then the current event as the
            /// first event (in any track) with time greater or equal to _time_. If there are more events with
            /// same time in different tracks their order is not defined, as the iterator tries to rotate
            /// across the tracks rather than to get first all events in a single track. However, temporal order
            /// is always granted.
        void                    GoToTime(MIDIClockTime time);
            /// Gets the current time of the iterator.
        MIDIClockTime           GetCurTime() const  { return state.GetCurTime(); }
            /// Gets the time of the next event in the multitrack (it can be different from current
            /// time if at the current time there aren't events).
            /// @param t here we get the time of next event, if valid.
            /// @return *true* if there is effectively a next event (we aren't at the end of the
            /// MIDIMultiTrack, *false* otherwise (*t doesn't contain a valid time).
        bool                    GetCurEventTime(MIDIClockTime *t) const;
            /// Gets the next event in the multitrack (it can be different from current
            /// time if at the current time there aren't events).
            /// @param track the track of the event, if valid.
            /// @param *msg a pointer to the event in the MidiMultiTrack
            /// @return *true* if there is effectively a next event (we aren't at the end of the
            /// MIDIMultiTrack, *false* otherwise (*track and **msg don't contain valid values).
        bool                    GetCurEvent(int *track, MIDITimedMessage **msg);
            /// Discards the current event and set as current the subsequent.
            /// @return *true* if there is effectively a next event (we aren't at the end of the
            /// MIDIMultiTrack, *false* otherwise.
        bool                    GoToNextEvent();
            /// Set as current the next event on track _track_.
            /// @return *true* if there is effectively a next event (we aren't at the end of the
            /// MIDIMultiTrack, *false* otherwise.
        bool                    GoToNextEventOnTrack(int track);
            /// Gets the current MIDIMultiTackIteratorState. You can save and then restore it for a
            /// faster processing in GoTo operations (admitting the contents of the multitrack are not
            /// changed):
        MIDIMultiTrackIteratorState& GetState()                     { return state; }
        const MIDIMultiTrackIteratorState &GetState() const         { return state; }
        void                    SetState(const MIDIMultiTrackIteratorState& s) { state = s; }
            /// Returns a pointer to the MIDIMultiTrack the iterator is attached to.
        MIDIMultiTrack*         GetMultiTrack()  				    { return multitrack; }
        const MIDIMultiTrack*   GetMultiTrack() const 	            { return multitrack; }

    protected:
        MIDIMultiTrack*         multitrack;     ///< The MIDIMultiTrack the class refers to
        MIDIMultiTrackIteratorState state;      ///< Holds current time data(See MIDIMultiTrackIteratorState class).
};


class MIDIEditMultiTrack : public MIDIMultiTrack {
    public:
                                MIDIEditMultiTrack(int cl_p_b = DEFAULT_CLKS_PER_BEAT,
                                              int max_tracks = DEFAULT_MAX_NUM_TRACKS) :
                                    MIDIMultiTrack(cl_p_b, max_tracks), start_track(0), end_track(0) {}
        virtual	               ~MIDIEditMultiTrack() {}

        void                    SetStartTrack(int trk)      { start_track = trk; }
        void                    SetEndTrack(int trk)        { end_track = trk; }
        int                     GetStartTrack() const       { return start_track; }
        int                     GetEndTrack() const         { return end_track; }

                    // these must be disabled in derived class: so they are declared but not implemented
        void                    EditCopy(MIDIClockTime start, MIDIClockTime end, int tr_start,
                                             int tr_end, MIDIEditMultiTrack* edit) {}
        void                    EditCut(MIDIClockTime start, MIDIClockTime end, MIDIEditMultiTrack* edit) {}
        void                    EditClear(MIDIClockTime start, MIDIClockTime end, int tr_start, int tr_end) {}
        void                    EditInsert(MIDIClockTime start, int tr_start, int times,
                                           bool sysex, MIDIEditMultiTrack* edit) {}
        void                    EditReplace(MIDIClockTime start, int tr_start, int times,
                                            bool sysex, MIDIEditMultiTrack* edit) {}
                    ///////////////////


        //void                    CopyTrack(int n, MIDIMultiTrack* trk);
                                // use operator= instead!
        void                    CopyAll(MIDIMultiTrack* m);
                                // does a Clear and copy

    private:

        int 	                start_track;
        int                     end_track;


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
/*
** Copyright 1986 to 1998 By J.D. Koftinoff Software, Ltd.
**
** All rights reserved.
**
** No one may duplicate this source code in any form for any reason
** without the written permission given by J.D. Koftinoff Software, Ltd.
**
*/
//
// Copyright (C) 2010 V.R.Madgazin
// www.vmgames.com vrm@vmgames.com
//

//
// MODIFIED by N. Cassetta  ncassetta@tiscali.it
// search /* NC */ for modifies
//


/*
namespace jdksmidi {


class MIDIMultiTrack
{

public:

    /// The constructor. If deletable_ = true it allocates memory for the tracks and deletes it in the destructor,
    /// otherwise the user must manually assign tracks to the array.
    MIDIMultiTrack ( int max_num_tracks_ = 64, bool deletable_ = true );

    /// The destructor frees the track pointers if they were allocated by the constructor.
    virtual ~MIDIMultiTrack();

    /// Assigns the MIDITrack pointed by _track_ to the _track_num_ position in the array. You may want to use
    /// this function if you want manually assign already initialized MIDITracks objects to the track numbers
    /// (in this case, construct the MIDIMultiTrack with the _deletable__ parameter = _false_, and it will not
    /// own its tracks). If you don't need this, the constructor with default parameter will give you all the
    /// tracks already allocated.
    /// \warning This function doesn't check if the previously assigned track was allocated, and eventually
    /// doesn't free its memory.
    void SetTrack ( int track_num, MIDITrack *track )
    {
        tracks[track_num] = track;
    }







    /// Returns the number of tracks with events. This is probably the number of tracks effectively used by
    /// your MIDIMultiTrack.
    int GetNumTracksWithEvents() const;



    /// Delete all tracks and remake the MIDIMultiTrack with _num_tracks_ empty tracks.
    bool ClearAndResize ( int num_tracks );

    /// This function is useful in dealing with MIDI format 0 files (with all events in an unique track). It remake
    /// the MIDIMultiTrack object with 17 tracks (src track can be a member of multitrack obiect), moves _src_
    /// track channel events to tracks 1-16 according their channel, and all other types of events to track 0
    bool AssignEventsToTracks ( const MIDITrack *src );

    /// The same as previous, but argument is track number of multitrack object himself
    bool AssignEventsToTracks ( int track_num = 0 )
    {
        return AssignEventsToTracks( GetTrack( track_num ) );
    }

    /// Erases all events from the tracks, leaving them empty.
    void Clear();





    /// Gets the total number of MIDI events in the multitrack.
    int GetNumEvents() const
    {
        int num_events = 0;
        for ( int i = 0; i < number_of_tracks; ++i )
            num_events += tracks[i]->GetNumEvents();
        return num_events;
    }

    // NEW BY NC //

    /// Returns **true** if track _n_ is allocated.
    bool IsValidTrackNum ( int n )
    {
        return ( n >= 0 && n < number_of_tracks && tracks[n] != 0 );
        // the latter may happen if !deletable;
    }

    /// Inserts the event _msg_ in the track _trk_. See MIDITrack::InsertEvent() for details.
    bool InsertEvent( int trk,  const MIDITimedBigMessage& msg, int _ins_mode = INSMODE_DEFAULT );

    /// Inserts a Note On and a Note Off event into the track. See MIDITrack::InsertNote() for details.
    bool InsertNote( int trk, const MIDITimedBigMessage& msg, MIDIClockTime len, int _ins_mode = INSMODE_DEFAULT );

    /// Deletes the event _msg_ from the track _trk_. See MIDITrack::DeleteEvent() for details.
    bool DeleteEvent( int trk,  const MIDITimedBigMessage& msg );

    /// Deletes the note _msg_ (_msg_ must be a Note On) from the track _trk_. See MIDITrack::DeleteNote() for details.
    bool DeleteNote( int trk, const MIDITimedBigMessage& msg );

    // END OF NEW BY NC //

protected:

    MIDITrack **tracks;                     ///< The array of pointers to the MIDITrack objects
    int number_of_tracks;                   ///< The number of allocated tracks
    bool deletable;                         ///< If **true** tracks are owned by the multitrack

    int clks_per_beat;                      ///< The common clock per beat timing parameter
};


/// This class is used by the MIDIMultiTrackIterator to keep track of the current state of the iterator. You
/// usually don't need to deal with it, and the only useful thing is getting and restoring a state for faster
/// processing (see MIDIMultiTrackk::SetStatus()).

class MIDIMultiTrackIteratorState
{
public:

    /// The constructor creates a MIDIMultiTrackIteratorState with a given number of tracks.
    MIDIMultiTrackIteratorState ( int num_tracks_ = 64 );

    /// The copy constructor.
    MIDIMultiTrackIteratorState ( const MIDIMultiTrackIteratorState &m );

    /// The destructor.
    virtual ~MIDIMultiTrackIteratorState();

    /// The equal operator
    const MIDIMultiTrackIteratorState & operator = ( const MIDIMultiTrackIteratorState &m );

    /// Returns the numver of tracks
    int GetNumTracks() const
    {
        return num_tracks;
    }

    /// Returns the track of current event
    int GetCurEventTrack() const
    {
        return cur_event_track;
    }

    /// Returns the current time
    MIDIClockTime GetCurrentTime() const
    {
        return cur_time;
    }

    /// Resets the state at the time 0.
    void Reset();

    /// Finds the tracks of the first event. Used internally by the MIDIMultiTrackIterator
    int FindTrackOfFirstEvent();

    MIDIClockTime cur_time;                     ///< The current time
    int cur_event_track;                        ///< The track of current event
    int num_tracks;                             ///< The number of tracks
    int *next_event_number;                     ///< An array of integers (the number of next event in every track)
    MIDIClockTime *next_event_time;             ///< An array of MIDIClockTime (the time of the next event in every track)
};

///
/// This class is a forward iterator for moving along a MIDIMultiTrack. It defines a current event (initially
/// the first event), and you can get it, move to the next event (in temporal order) in the multitrack, or move
/// to the first event with a given time. When the iterator reaches the end of the multitrack the current event
/// become undefined, and the get methods will return **false**.
///

class MIDIMultiTrackIterator
{
public:

    /// The constructor.
    MIDIMultiTrackIterator ( const MIDIMultiTrack *mlt );

    /// The destructor.
    virtual ~MIDIMultiTrackIterator();

    /// Sets as current the first event with time equal to _time_ (or less, if there aren't events at the
    /// given time).
    void GoToTime ( MIDIClockTime time );

    /// Gets the MIDI time of the current event.
    /// \return **true** if there is effectively a current event, **false** if we are at the end of the multitrack.
    bool GetCurEventTime ( MIDIClockTime *t ) const;

    /// Gets the current event, returning its track and a pointer to it.
    /// \param [out] *track the track of the current event
    /// \param [out] **msg a pointer to the current event
    /// \return **true** if there is effectively a current event, **false** if we are at the end of the multitrack
    /// \warning this function doesn't move the iterator to the next event; you must call GoToNextEvent().
    bool GetCurEvent ( int *track, const MIDITimedBigMessage **msg ) const;

    /// Moves the iterator to the next event.
    /// \return **true** if the operation was successful (the current event was defined and wasn't the last
    /// event), **false** otherwise.
    bool GoToNextEvent();

    /// Moves the iterator to the next event on track _track_.
    /// \return **true** if the operation was successful (the current event was defined and wasn't the last
    /// event on the track), **false** otherwise.
    bool GoToNextEventOnTrack ( int track );

    //@{
    /// Gets the state of the iterator (see MultiTrackIteratorState).
    const MIDIMultiTrackIteratorState &GetState() const
    {
        return state;
    }

    MIDIMultiTrackIteratorState &GetState()
    {
        return state;
    }
    //@}

    /// Sets the state of the iterator (see MultiTrackIteratorState). If you want perform something with
    /// the iterator and then return to the initial situation you can save your state with GetState(), do
    /// what you want and then restore the saved state with this.
    void SetState ( const MIDIMultiTrackIteratorState &s )
    {
        state = s;
    }

    /// Gets the associated MIDIMultiTrack.
    const MIDIMultiTrack * GetMultiTrack() const
    {
        return multitrack;
    }

protected:

    const MIDIMultiTrack *multitrack;           ///< The associated multitrack
    MIDIMultiTrackIteratorState state;          ///< The state of the iterator
};

}
*/

