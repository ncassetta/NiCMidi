/*  Adapted from
 *  libjdkmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  *** RELEASED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) April 27, 2004 ***
 *
 *  by Nicola Cassetta
 *
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/// \file
/// Contains the definition of the classes MIDITrack and MIDITrackIterator.


#ifndef MIDI_TRACK_H_INCLUDED
#define MIDI_TRACK_H_INCLUDED


#include "midi.h"
#include "sysex.h"
#include "msg.h"

#include <vector>


/// Defines the default behavior of the methods MIDITrack::InsertEvent() and MIDITrack::InsertNote()
/// when inserting events.
/// If they are trying to insert an event into a track and find an equal or similar event at same MIDI time
/// (see MIDITimedMessage::IsSameKind()) they can replace it with the new event or insert it
/// without deleting the older. This is determined by a static attribute of the class MIDITrack and can
/// be changed by the MIDITrack::SetInsertMode() method (the default is INSMODE_INSERT_OR_REPLACE).
/// When the above methods are called with default argument *_ins_mode* they follow the default behavior,
/// this can be overriden giving them one of the other values as last parameter
enum TInsmode
{
    INSMODE_DEFAULT,    ///< follow the default behaviour (only used as default argument in methods MIDITrack::InsertEvent() and MIDITrack::InsertNote()
    INSMODE_INSERT,     ///< always insert events, if a same kind event was found  duplicate it.
    INSMODE_REPLACE,    ///< replace if a same kind event was found, otherwise do nothing.
    INSMODE_INSERT_OR_REPLACE,          ///< replace if a same kind event was found, otherwise insert.
    INSMODE_INSERT_OR_REPLACE_BUT_NOTE  ///< as above, but allow two same note events at same time (don't replace, insert a new note).
};


/// Defines the behaviour of the method MIDITrack::FindEventNunber() when searching events.
enum
{
    COMPMODE_EQUAL,     ///< the method searches for an event matching equal operator.
    COMPMODE_SAMEKIND,  ///< the method searches for an event matching the MIDITimedMessage::IsSameKind() method.
    COMPMODE_TIME       ///< the method searches for the first event with time equal to the event time.
};


///
/// Manages a std::vector of MIDITimedMessage objects storing MIDI events, with methods for editing them.
/// Events are ordered by time and a MIDITrack has at least the MIDI data end meta-event (EOT)
/// at its end (it cannot be deleted). Moreover, the Analyze() method examines the events in a track,
/// classifying it into various types (for example, a master track contains only MIDI meta events, a single
/// channel track contains events with the same MIDI channel, etc.) useful for editing purposes.
///
class  MIDITrack {
    public:
        /// The constructor creates an empty track with only the EOT event.
        /// \param end_time the MIDI clock time of the EOT; this become the "time length" of the track
                                    MIDITrack(MIDIClockTime end_time = 0);
        /// The copy constructor.
                                    MIDITrack(const MIDITrack &trk);
        /// The destructor deletes the events in the track.
        virtual                     ~MIDITrack()                {}
        /// The assignment operator.
        MIDITrack&                  operator=(const MIDITrack &trk);
        /// Deletes all events leaving the track empty (i.e.\ with only the EOT event).
        /// \param mantain_end If it is **true** the method doesn't change the time of the EOT,
        /// otherwise sets it to 0.
        void	                    Clear(bool mantain_end = false);
        /// Returns the number of events in the track (if the track is empty this returns
        /// 1, for the EOT event).
        unsigned int	            GetNumEvents() const                { return events.size(); }
        /// Returns **true** if the track has only the EOT event.
        bool                        IsEmpty() const                     { return events.size() == 1; }
        /// Returns the track channel (-1 if the track has not type TYPE_CHAN or TYPE_IRREG_CHAN).
        char                        GetChannel();
        /// Returns the track type (one of TYPE_MAIN, TYPE_TEXT, TYPE_CHAN, TYPE_IRREG_CHAN, TYPE_MIXED_CHAN,
        /// TYPE_UNKNOWN, TYPE_SYSEX, TYPE_RESET_SYSEX, TYPE_BOTH_SYSEX).
        char                        GetType();
        /// Returns **true** if the track contains MIDI SysEx messages.
        char                        HasSysex();
        /// Returns the address of an event in the track.
        /// \param ev_num the index of the event in the track (must be in the range 0 ... GetNumEvents() - 1).
        MIDITimedMessage*           GetEventAddress(int ev_num)         { return &events[ev_num]; }
        /// Returns the address of an event in the track.
        /// \param ev_num the index of the event in the track (must be in the range 0 ... GetNumEvents() - 1).
        const MIDITimedMessage*     GetEventAddress(int ev_num) const   { return &events[ev_num]; }
        /// Returns a reference to an event in the track.
        /// \param ev_num the index of the event in the track (must be in the range 0 ... GetNumEvents() - 1).
        MIDITimedMessage&           GetEvent(int ev_num)               { return events[ev_num]; }
        /// Returns a reference to an event in the track.
        /// \param ev_num the index of the event in the track (must be in the range 0 ... GetNumEvents() - 1).
        const MIDITimedMessage&     GetEvent(int ev_num) const         { return events[ev_num]; }
        /// Returns the time of the EOT event (i.e.\ the time length of the track).
        MIDIClockTime               GetEndTime() const                 { return events.back().GetTime(); }
        /// Sets the time of the EOT event.
        /// \param end_time the new EOT time. If there are events of other type after it the method fails and returns
        /// **false**.
        bool                        SetEndTime(MIDIClockTime end_time);
        /// Sets the time of the EOT event equal to the time of the last (non data end) event
        /// of the track.
        void                        ShrinkEndTime();
        /// Sets the channel of all MIDI channel events to _ch_ (_ch_ must be in the range 0 ... 15).
        void                        SetChannel(int ch);
        /// Returns **true** if _ev_num_ is in the range 0 ... GetNumEvents() - 1.
        bool                        IsValidEventNum(int ev_num) const
                                                { return (0 <= ev_num && (unsigned int)ev_num < events.size()); }
        /// Returns the length in MIDI clocks of the given note. _msg_ must be a Note On event present in the track
        /// (otherwise the function will return 0). It returns TIME_INFINITE if doesn't find the corresponding
        /// Note Off event in the track.
        MIDIClockTime               GetNoteLength (const MIDITimedMessage& msg) const;
        /// Sets the default behaviour for the methods InsertEvent() and InsertNote(). This can be overriden
        /// by them by mean of the last (default) parameter.
        /// \param mode one of \ref ::INSMODE_INSERT, \ref ::INSMODE_REPLACE, #INSMODE_INSERT_OR_REPLACE,
        /// #INSMODE_INSERT_OR_REPLACE_BUT_NOTE; default is #INSMODE_INSERT_OR_REPLACE.
        /// \note #INSMODE_DEFAULT is used only in the insert methods (as default argument) and
        /// does nothing if given as parameter here.
        static void                 SetInsertMode(int mode);
        /// Inserts a single event into the track. It could be used for inserting Note On and Note Off events,
        /// but this is better done by InsertNote() which inserts both with a single call. The method handles
        /// automatically the data end message, moving it if needed, so you must not deal with it.
        /// It also determines the correct position of events with same MIDI time,
        /// using the MIDITimedMessage::CompareEventsForInsert() method for comparison.
        /// \param msg the event to insert.
        /// \param mode this determines the behaviour of the method if it finds an equal or similar event with
        /// same MIDI time in the track: it may replace the event or insert a new event anyway. If you leave the
        /// default parameter (INSMODE_DEFAULT) the method will follow the behaviour set by the static
        /// method SetInsertMode(), otherwise you may override it giving the last parameter. For details see
        /// INSMODE_DEFAULT (default), INSMODE_REPLACE, INSMODE_INSERT_OR_REPLACE, INSMODE_INSERT_OR_REPLACE_BUT_NOTE.
        /// \return **false** in some situations in which the method cannot insert:
        /// + _msg_ was an EndOfTrack (you cannot insert it)
        /// + __ins_mode_ was INSMODE_REPLACE but there is no event to replace
        /// + a memory error occurred.
        /// otherwise **true**.
        bool                        InsertEvent( const MIDITimedMessage& msg, int mode = INSMODE_DEFAULT );
        /// Inserts a Note On and a Note Off event into the track. Use this method for inserting note messages as
        /// InsertEvent() doesn't check the correct order of note on/note off. It handles automatically the
        /// EndOfTrack message, moving it if needed, so you must not deal with it. It also determines
        /// the correct position of events with same MIDI time, using the MIDITimedMessage::CompareEventsForInsert()
        /// method for comparison (so a Note Off always precedes a Note On with same time).
        /// \param msg must be a Note On event.
        /// \param len the length of the note: the method will create a Note Off event and put it after _len_ MIDI
        /// clocks in the track.
        /// \param mode the same for InsertEvent() (when replacing a note, the method finds and deletes both
        /// the old Note On and Note Off events).
        /// \return **false** in some situations in which the method cannot insert:
        /// + _msg_ was not a Note On event
        /// + _mode_ was INSMODE_REPLACE but there is no event to replace
        /// + a memory error occurred
        /// otherwise **true**.
        /// \bug In the latter case the method could leave the track in an inconsistent state (a Note On without
        /// corresponding Note Off or viceversa).
        bool                        InsertNote( const MIDITimedMessage& msg, MIDIClockTime len,
                                                int mode = INSMODE_DEFAULT );
        /// Deletes an event from the track. Use DeleteNote() for safely deleting both Note On and Note Off. You cannot
        /// delete the data end event.
        /// \param msg a copy of the event to delete.
        /// \return **false** if an exact copy of the event was not found, or if a memory error occurred, otherwise **true**.
        bool                        DeleteEvent( const MIDITimedMessage& msg );
        /// Deletes a Note On and corresponding Note Off events from the track. Don't use DeleteEvent() for deleting
        /// notes.
        /// \param msg a copy of the Note On event to delete.
        /// \return **false** if an exact copy of the event was not found, or if a memory error occurred, otherwise **true**.
        /// \bug In the latter case the method could leave the track in an inconsistent state (a Note On without
        /// corresponding Note Off or viceversa).
        bool                        DeleteNote( const MIDITimedMessage& msg );

        /// Inserts the event as last, adjusting the data end. This function should be used with caution, as it doesn't
        /// check temporal order and track consistency. You could use it if you would manually copy tracks
        /// (MultiTrack::AssignEventsToTracks() does it).
        void                        PushEvent( const MIDITimedMessage& msg);

        /// Inserts a
        void                        InsertInterval(MIDIClockTime start, MIDIClockTime length, const MIDITrack* src = 0);
                                                    // if src == 0 only shift events of length clocks
        /// Copies events from _start_ to _end_ into the track _interval_.
        MIDITrack*                  MakeInterval(MIDIClockTime start, MIDIClockTime end, MIDITrack* interval) const;
                                                    // copies selected interval into another interval
        void                        DeleteInterval(MIDIClockTime start, MIDIClockTime end);
                                                    // deletes events and shifts subsequents
        void                        ClearInterval(MIDIClockTime start, MIDIClockTime end);
                                                    // deletes events leaving subsequents unchanged
        void                        ReplaceInterval(MIDIClockTime start, MIDIClockTime length,
                                                   bool sysex, const MIDITrack* src);
        /// Cuts note and pedal events at the time _t_. All sounding notes and held pedals are truncated (the
        /// corresponding off events after the time are deleted) and the pitch bend is reset. This function is
        /// intended for "slicing" a track in cut, copy and paste editing.
        // TODO: restored old version: test this (involves even interval methods)
        void                        CloseOpenEvents(MIDIClockTime t);

        /// Finds an event in the track matching a given event.
        /// \param[in] msg the event to look for
        /// \param[out] event_num contains the event number in the track if the event was found; otherwise it contains
        /// **-1** if *event time* was invalid, or the number of the first event with the same event time.
        /// \param[in] mode (compare mode) an enum value with the following meaning.
        /// + COMPMODE_EQUAL : the event must be equal to *msg*.
        /// + COMPMODE_SAMEKIND : the event is a same kind event (see MIDITimedMessage::IsSameKind()).
        /// + COMPMODE_TIME : the behaviour is the same of FindEventNumber(time, event_num).
        /// \return **true** if an event matching _msg_ was found, **false** otherwise.
        bool                        FindEventNumber( const MIDITimedMessage& msg, int *event_num,
                                                     int mode = COMPMODE_EQUAL) const;

        /// Finds the first event in the track with the given time.
        /// \param[in] time the time to look for.
        /// \param[out] event_num contains the event number in the track if an event was found; otherwise it contains
        /// **-1** if *time* was invalid, or the number of the last event before *time*.
        /// \return **true** if an event with given time was found, **false** otherwise.
        bool                        FindEventNumber (MIDIClockTime time, int *event_num) const;

        /// Used in the status request
        enum {
            TYPE_EMPTY = 0,         ///< Track is empty
            TYPE_MAIN = 1,          ///< Track has Main meta events (time, tempo, key ...) and no channel events
            TYPE_TEXT = 2,          ///< Track has only text meta events (probably lyrics)
            TYPE_CHAN = 3,          ///< Track is a normal channel track. You can find the channel with GetChannel()
            TYPE_IRREG_CHAN = 4,    ///< Track is an irregular channel track (it can contain Main meta events)
            TYPE_MIXED_CHAN = 5,    ///< Track has events with more than one channel
            TYPE_UNKNOWN = 6,       ///< None of the above
            TYPE_SYSEX = 7,         ///< Track has only common sysex events
            TYPE_RESET_SYSEX = 8,   ///< Track has reset sysex (GM Reset, Gs Reset, XG Reset)
            TYPE_BOTH_SYSEX = 9,    ///< Track has both types of sysex
            INIT_STATUS = 0xff,     ///< Internal use
            HAS_MAIN_META = 0x100,  ///< Flag for Main meta
            HAS_TEXT_META = 0x200,  ///< Flag for text meta
            HAS_ONE_CHAN = 0x400,   ///< Flag for one channel
            HAS_MANY_CHAN = 0x800,  ///< Flag for more channels
            HAS_SYSEX = 0x1000,     ///< Flag for common sysex
            HAS_RESET_SYSEX = 0x2000,///< Flag for reset sysex
            STATUS_DIRTY = 0x4000   ///< Internal use
        };

        /// Analyses events in the track upgrading the track status. When a track is created or loaded by
        /// the AdvancedSequencer class this is automatically called. If you edit the track, changing its
        /// events, you should call this for updating its status.
        void                        Analyze();

    protected :
        std::vector<MIDITimedMessage>
                                    events;     ///< The buffer of events
        int                         status;     ///< A bitfield used to determine the track type
        static int                  ins_mode;   ///< See SetInsertMode()
};


///
/// Forward iterator for moving along a MIDITrack. It defines a current time (initially 0)
/// and a current event. You can move to any time in the track: when moving, it keeps track of
/// program, bender, control changes and notes on.
///
class MIDITrackIterator {
    public:
        /// The constructor. You must specify the track which the iterator is attached to.
                                    MIDITrackIterator(MIDITrack* trk);
        /// Sets the current time to 0, updating the iterator status.
        void                        Reset();
        /// These returns a pointer to the track the iterator is attached to.
        ///<{
        MIDITrack*                  GetTrack()                  { return track; }
        const MIDITrack*            GetTrack() const 	        { return track;  }
        ///<}
        /// Sets the iterator track (causes a reset).
        void                        SetTrack(MIDITrack* trk);
        /// Returns the current time of the iterator.
        MIDIClockTime               GetCurrentTime() const      { return cur_time; }
        /// Returns the current event number in the track.
        int                         GetCurrentEventNum() const  { return cur_ev_num; }
        /// Returns the current track program (-1 if not set).
        char                        GetProgram() const          { return program; }
        /// Returns the current value for the given control (-1 if not set).
        char                        GetControl(unsigned char c) const
                                                                { return controls[c]; }
        /// Returns the current bender value.
        short                       GetBender() const           { return bender_value; }
        /// Returns the track channel (-1 if there aren't channel events in the track).
        //char                        GetMIDIChannel() const      { return channel; }
        /// Returns the number of notes on at current time.
        int                         GetNotesOn() const          { return num_notes_on; }
        /// Returns *true* if the given note is on at current time.
        char                        IsNoteOn(unsigned char n) const
                                                                { return notes_on[n]; }
        /// Returns *true* if the hold pedal is on at current time.
        bool                        IsPedalOn() const           { return controls[64] > 64; }
        /// Finds the MIDITimedMessage in the track corresponding to the note off for the given note.
        /// \param[in] note : the note on
        /// \param[out] msg : a pointer to the note off message
        /// \return *true* if msg is valid, *false* otherwise (a note off was not found).
        bool                        FindNoteOff(unsigned char note, MIDITimedMessage** msg);
        /// Finds the next hold pedal off MIDITimedMessage in the track.
        /// \param[out] msg : a pointer to the pedal off message
        /// \return *true* if msg is valid, *false* otherwise (a pedal off was not found).
        bool                        FindPedalOff(MIDITimedMessage** msg);
        /// Goes to the given time, which becomes the current time, and sets then the current event as the
        /// first event in the track with time greater or equal to _time_.
        bool                        GoToTime(MIDIClockTime time);
        /// Returns the next event in the track.
        /// \param *msg a pointer to the event in the MidiMultiTrack
        /// \return *true* if there is effectively a next event (we aren't at the end of the
        /// track), *false* otherwise (and **msg doesn't contain valid value).
        bool                        GetNextEvent(MIDITimedMessage** msg);
        /// Gets the time of the next event in the track (it can be different from current time if
        /// at current time there are not events).
        /// \param t here we get the time of next event, if valid.
        /// \return *true* if there is effectively a next event (we aren't at the end of the
        /// track), *false* otherwise (*t doesn't contain a valid time).
        bool                        GetNextEventTime(MIDIClockTime* t) const;
        /// Returns *true* if at the current time there is an event of the same kind of _msg_
        /// (see MIDITimedMessage::IsSameKind()). The time of _msg_ is ignored.
        bool                        EventIsNow(const MIDITimedMessage& msg);
        // /// Discards the current event and set as current the subsequent.
        // /// \return *true* if there is effectively a next event (we aren't at the end of the track),
        // /// *false* otherwise.
        //bool                        GoToNextEvent();

    private:

        //void                        FindChannel();
        bool                        Process(const MIDITimedMessage *msg);
        void                        ScanEventsAtThisTime();
                                        // warning: this can be used only when we reach the first
                                        // event at a new time!

        MIDITrack*                  track;
        //char                        channel;        // can be -1
        int                         cur_ev_num;     // number of the current event
        MIDIClockTime               cur_time;       // current time

        char                        program;        // current program change, or -1
        char                        controls[128];  // value of every control change, or -1
        short                       bender_value;	// last seen bender value
        unsigned char               num_notes_on;	// number of notes currently on
        char		                notes_on[128];  // 0 if off, or velocity
};



//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////










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
namespace jdksmidi
{




///
/// The MIDITrack class is a container that manages a vector of MIDITimedBigMessages. It internally
/// stores MIDITimedBigMessage objects. There is a fixed maximum number of events that it can store,
/// which is defined by MIDIChunksPerTrack * MIDITrackChunkSize.
///

class  MIDITrack
{
public:

    /// Construct a MIDITrack object with the specified number of events
    /// @param size The number of events, defaults to 0
    MIDITrack ( int size = 0 );

    /// Copy Constructor for a MIDITrack object
    /// @param t The reference to the MIDITrack object to copy
    MIDITrack ( const MIDITrack &t );

    /// The MIDITrack Destructor, frees all chunks and referenced MIDITimedBigMessage's
    ~MIDITrack();







/*
    /// The same as GetEvent(). It's included only for compatibility with older versions and could be removed
    /// in the future.
    MIDITimedBigMessage * GetEventAddress ( int event_num );
    const MIDITimedBigMessage * GetEventAddress ( int event_num ) const;

    /// Returns a pointer to the *event_num* message stored in the track. If *event_num* is not a valid event
    /// number returns 0.
    MIDITimedBigMessage *GetEvent ( int event_num );
    const MIDITimedBigMessage *GetEvent ( int event_num ) const;



    /// Copies in *msg* the *event_num* message of the track. Returns **true** if *event_num* is a valid
    /// event number (if not, *msg* will be left unchanged).
    bool GetEvent ( int event_num, MIDITimedBigMessage *msg ) const;

    /// Copies the event *msg* at the place *n* in the track (default: as last event in the track).
    /// @note this is a low level function and may leave the track in an inconsistent state (for ex. with
    /// events after the EndOfData). You should use InsertEvent(), InsertNote() for safe editing.
    bool PutEvent ( const MIDITimedBigMessage &msg, int n = -1  );



    /// Changes the event *event_num* in the track to the message _msg_.
    /// @note this is a low level function and may leave the track in an inconsistent state (for ex. with
    /// events after the EndOfData). You shouldn't use it for safe editing.
    bool SetEvent ( int event_num, const MIDITimedBigMessage &msg );

    /// Removes the event *event_num* from the track (it does nothing if *event_num* is not a valid event number).
    /// @note this is a low level function and may leave the track in an inconsistent state (for ex. with
    /// events after the EndOfData). You should use DeleteEvent(), DeleteNote() for safe editing.
    bool RemoveEvent ( int event_num );

    // NEW BY NC

    /// Finds an event in the track matching a given event.
    /// @param[in] msg the event to look for
    /// @param[out] event_num contains the event number in the track if the event was found; otherwise it contains
    /// **-1** if *event time* was invalid, or the number of the first event with the same event time.
    /// @param[in] _comp_mode (compare mode) an enum value with the following meaning.
    /// + @ref COMPMODE_EQUAL : the event must be equal to *msg*.
    /// + @ref COMPMODE_SAMEKIND : the event is a same kind event (see MIDITimedBigMessage::IsSameKind()).
    /// + @ref COMPMODE_TIME : the behaviour is the same of FindEventNumber(time, event_num).
    /// @returns **true** if an event with given time was found, *false* otherwise.
    bool FindEventNumber( const MIDITimedBigMessage& msg, int *event_num, int _comp_mode = COMPMODE_EQUAL) const;

    /// Finds the first event in the track with the given time.
    /// @param[in] time the time to look for.
    /// @param[out] event_num contains the event number in the track if an event was found; otherwise it contains
    /// **-1** if *time* was invalid, or the number of the last event before *time*.
    /// @returns **true** if an event with given time was found, **false** otherwise.
    bool FindEventNumber ( MIDIClockTime time, int *event_num ) const;

    // END OF NEW



};

}

*/

