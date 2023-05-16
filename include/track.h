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
/// Contains the definition of the classes MIDITrack and MIDITrackIterator.


#ifndef _NICMIDI_TRACK_H
#define _NICMIDI_TRACK_H


#include "manager.h"
#include "midi.h"
#include "sysex.h"
#include "msg.h"

#include <vector>
#include <string>

/// \addtogroup GLOBALS
///@{

/// Defines the default behavior of the methods MIDITrack::InsertEvent() and MIDITrack::InsertNote()
/// when inserting events.
/// If they are trying to insert an event into a track and find an equal or similar event at same MIDI time
/// (see MIDITimedMessage::IsSameKind()) they can replace it with the new event or insert it
/// without deleting the older. This is determined by a static attribute of the class MIDITrack and can
/// be changed by the MIDITrack::SetInsertMode() method (the default is INSMODE_INSERT_OR_REPLACE).
/// When the above methods are called with default argument *_ins_mode* they follow the default behavior,
/// this can be overriden giving them one of the other values as last parameter
enum tInsMode
{
    INSMODE_DEFAULT,    ///< follow the default behaviour (only used as default argument in methods MIDITrack::InsertEvent() and MIDITrack::InsertNote()
    INSMODE_INSERT,     ///< always insert events, if a same kind event was found  keep both.
    INSMODE_REPLACE,    ///< replace if a same kind event was found, otherwise do nothing.
    INSMODE_INSERT_OR_REPLACE,          ///< replace if a same kind event was found, otherwise insert.
    INSMODE_INSERT_OR_REPLACE_BUT_NOTE  ///< as above, but allow two same note events at same time (don't replace, insert a new note).
};


/// Defines the behaviour of the method MIDITrack::FindEventNumber() when searching events.
enum
{
    COMPMODE_EQUAL,     ///< the method searches for an event matching equal operator.
    COMPMODE_SAMEKIND,  ///< the method searches for an event matching the MIDITimedMessage::IsSameKind() method.
    COMPMODE_TIME       ///< the method searches for the first event with time equal to the event time.
};
///@}


///
/// Manages a std::vector of MIDITimedMessage objects storing MIDI events, with methods for editing them.
/// Events are ordered by time and a MIDITrack has at least the MIDI data end meta-event (EOT)
/// at its end (it cannot be deleted). Moreover, the Analyze() method examines the events in a track,
/// classifying it into various types (for example, a master track contains only MIDI meta events, a single
/// channel track contains events with the same MIDI channel, etc.) useful for editing purposes.
///
/// The following parameters are associated with the track:
/// + The out port for playing
/// + The in port for recording
/// + The recording channel
/// + The time shifting amount (in MIDI ticks)
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
        // TODO: these should not be needed!
        /// The assignment operator.
        MIDITrack&                  operator=(const MIDITrack &trk);
        /// Deletes all events leaving the track empty (i.e.\ with only the EOT event) and
        /// resets time_shift, in_port, out_port to 0.
        void                        Reset();
        /// Deletes all events leaving the track empty (i.e.\ with only the EOT event).
        /// \param mantain_end If it is **true** the method doesn't change the time of the EOT,
        /// otherwise sets it to 0.
        void	                    Clear(bool mantain_end = false);

        /// Returns the number of events in the track (if the track is empty this returns
        /// 1, for the EOT event).
        unsigned int	            GetNumEvents() const                { return events.size(); }
        /// Returns **true** if _ev_num_ is in the range 0 ... GetNumEvents() - 1.
        bool                        IsValidEventNum(int ev_num) const
                                                { return (0 <= ev_num && (unsigned int)ev_num < events.size()); }
        /// Returns **true** if the track has only the EOT event.
        bool                        IsEmpty() const                     { return events.size() == 1; }
        /// Returns the time of the EOT event (i.e\. the time length of the track).
        MIDIClockTime               GetEndTime() const                 { return events.back().GetTime(); }
        /// Returns the MIDI out port id.
        unsigned int                GetOutPort() const                  { return out_port; }
        /// Returns the MIDI in port id.
        unsigned int                GetInPort() const                   { return in_port; }
        /// Returns the track channel (-1 if the track has not type TYPE_CHAN or TYPE_IRREG_CHAN).
        /// See \ref NUMBERING
        /// \note This is **not** const, because it may call Analyze(), causing an update of the track status.
        int                         GetChannel();
        /// Returns the entire status bitfield, so you can test individual properties of the track with an AND
        /// with one of its bits.
        /// \note This is **not** const, because it may call Analyze(), causing an update of the track status.
        int                         GetStatus();
        /// Returns the track type (one of \ref TYPE_MAIN, \ref TYPE_TEXT, \ref TYPE_CHAN, \ref TYPE_IRREG_CHAN,
        /// \ref TYPE_MIXED_CHAN, \ref TYPE_UNKNOWN, \ref TYPE_SYSEX, \ref TYPE_RESET_SYSEX, \ref TYPE_BOTH_SYSEX).
        /// \note This is **not** const, because it may call Analyze(), causing an update of the track status.
        unsigned char               GetType();
        /// Returns the channel for recording (-1 for all channels).
        /// See \ref NUMBERING
        int                         GetRecChannel()                         { return (int)rec_chan; }
        /// Returns the track time shift in MIDI ticks.
        int                         GetTimeShift() const                    { return time_shift; }
        /// Returns non zero if the track contains MIDI SysEx messages. In this case it can return one of
        /// \ref TYPE_SYSEX, \ref TYPE_RESET_SYSEX or \ref TYPE_BOTH_SYSEX.
        /// \note This is **not** const, because it may call Analyze(), causing an update of the track status.
        unsigned char               HasSysex();
        /// Returns the address of an event in the track.
        /// \param ev_num the index of the event in the track (must be in the range 0 ... GetNumEvents() - 1).
        MIDITimedMessage*           GetEventAddress(unsigned int ev_num)    { return &events[ev_num]; }
        /// Returns the address of an event in the track.
        /// \param ev_num the index of the event in the track (must be in the range 0 ... GetNumEvents() - 1).
        const MIDITimedMessage*     GetEventAddress(unsigned int ev_num) const      { return &events[ev_num]; }
        /// Returns a reference to an event in the track.
        /// \param ev_num the index of the event in the track (must be in the range 0 ... GetNumEvents() - 1).
        MIDITimedMessage&           GetEvent(unsigned int ev_num)           { return events[ev_num]; }
        /// Returns a reference to an event in the track.
        /// \param ev_num the index of the event in the track (must be in the range 0 ... GetNumEvents() - 1).
        const MIDITimedMessage&     GetEvent(unsigned int ev_num) const     { return events[ev_num]; }

        /// Sets the time of the EOT event.
        /// \param end_time the new EOT time. If there are events of other type after it the method fails and returns
        /// **false**.
        bool                        SetEndTime(MIDIClockTime end_time);
        /// Sets the channel of all MIDI channel events to _chan_ (_chan_ must be in the range 0 ... 15).
        /// \return **true** if events were changed, **false** otherwise (_ch_ was not a valid channel number).
        bool                        SetChannel(int chan);
        /// Sets the channel for recording (-1 for all channels).
        /// \return **true** if _chan_ was in the correct range, **false** otherwise).
        bool                        SetRecChannel(int chan);
        /// Sets the input port of the track
        /// \return **true** if _port_ is a valid port number, **false** otherwise.
        bool                        SetInPort(unsigned int port);
        /// Sets the output port of the track.
        /// \return **true** if _port_ is a valid port number, **false** otherwise.
        bool                        SetOutPort(unsigned int port);
        /// Sets the track time shift in MIDI ticks.
        void                        SetTimeShift(int t)                 { time_shift = t; }
        /// Sets the time of the EOT event equal to the time of the last (non data end) event
        /// of the track.
        void                        ShrinkEndTime();
        /// Returns the length in MIDI clocks of the given note. _msg_ must be a Note On event present in the track
        /// (otherwise the function will return 0). It returns TIME_INFINITE if doesn't find the corresponding
        /// Note Off event in the track.
        MIDIClockTime               GetNoteLength (const MIDITimedMessage& msg) const;
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
        /// \ref INSMODE_DEFAULT (default), \ref INSMODE_REPLACE, \ref INSMODE_INSERT_OR_REPLACE,
        /// \ref INSMODE_INSERT_OR_REPLACE_BUT_NOTE.
        /// \return **false** in some situations in which the method cannot insert:
        /// + _msg_ was an EndOfTrack (you cannot insert it)
        /// + __ins_mode_ was INSMODE_REPLACE but there is no event to replace
        /// + a memory error occurred.
        /// otherwise **true**.
        bool                        InsertEvent(const MIDITimedMessage& msg, tInsMode mode = INSMODE_DEFAULT);
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
        /// + a memory error occurred.
        /// otherwise **true**.
        /// \bug In the latter case the method could leave the track in an inconsistent state (a Note On without
        /// corresponding Note Off or viceversa).
        bool                        InsertNote(const MIDITimedMessage& msg, MIDIClockTime len,
                                               tInsMode mode = INSMODE_DEFAULT);
        /// Deletes an event from the track. Use DeleteNote() for safely deleting both Note On and Note Off. You cannot
        /// delete the data end event.
        /// \param msg a copy of the event to delete.
        /// \return **false** if an exact copy of the event was not found, or if a memory error occurred, otherwise **true**.
        bool                        DeleteEvent(const MIDITimedMessage& msg);
        /// Deletes a Note On and corresponding Note Off events from the track. Don't use DeleteEvent() for deleting
        /// notes.
        /// \param msg a copy of the Note On event to delete.
        /// \return **false** if an exact copy of the event was not found, or if a memory error occurred, otherwise **true**.
        /// \bug In the latter case the method could leave the track in an inconsistent state (a Note On without
        /// corresponding Note Off or viceversa).
        bool                        DeleteNote(const MIDITimedMessage& msg);
        /// Inserts the event as last, adjusting the data end. This function should be used with caution, as it doesn't
        /// check temporal order and track consistency. You could use it if you would manually copy tracks
        /// (MIDIMultiTrack::AssignEventsToTracks() does it).
        void                        PushEvent(const MIDITimedMessage& msg);
        /// Shifts forward by a _length_ time the track events from _start_ onwards. If _src_ == 0 it leaves the newly
        /// created interval empty, otherwise copies the contents of _src_ into it. The events in the time interval
        /// 0 ...       _length_ in _src_ are copied into the _start_ ... _start_ + _length_ times in the actual track.
        /// If the _src_ end time is greater than _length_ the copy process is truncated.
        /// \warning This is still experimental, not tested enough
        void                        InsertInterval(MIDIClockTime start, MIDIClockTime length, const MIDITrack* src = 0);
                                                    // if src == 0 only shift events of length clocks
        /// Copies events from _start_ to _end_ into the track _interval_. The events from _start_ to _end_ times in the
        /// actual track are copied into the 0 ... _end_ - _start_ times in _interval_.
        /// \warning This is still experimental, not tested enough
        void                        MakeInterval(MIDIClockTime start, MIDIClockTime end, MIDITrack* interval) const;
        /// Deletes events from _start_ to _end_, shifting subsequents.
        /// \warning This is still experimental, not tested enough
        void                        DeleteInterval(MIDIClockTime start, MIDIClockTime end);
        /// Deletes events from _start_ to _end_, leaving subsequents unchanged.
        /// \warning This is still experimental, not tested enough
        void                        ClearInterval(MIDIClockTime start, MIDIClockTime end);
        /// Replaces events from _start_ to _end_ with those in _src_. The events in the time interval
        /// 0 ... _end_ - _start_ in _src_ are copied into the _start_ ... _end_ times in the actual track.
        /// If the _src_ end time is greater than _end_ - _start_ the copy process is truncated.
        /// \warning This is still experimental, not tested enough
        void                        ReplaceInterval(MIDIClockTime start, MIDIClockTime end, const MIDITrack* src);
        /// Cuts note and pedal events (searching to the time _from_) at the time _to_. All sounding notes and
        /// held pedals are truncated (the corresponding off events after the time _to_ are deleted) and the pitch
        /// bend is reset. Events at time _to_ are **not** truncated. This function is intended for "slicing"
        /// a track in cut, copy and paste editing.
        // TODO: restored old version: test this (involves even interval methods)
        void                        CloseOpenEvents(MIDIClockTime from, MIDIClockTime to);
        /// Finds an event in the track matching a given event.
        /// \param[in] msg the event to look for
        /// \param[out] event_num contains the event number in the track if the event was found; otherwise it contains
        /// **-1** if *event time* was invalid, or the number of the first event with the same event time.
        /// \param[in] mode (compare mode) an enum value with the following meaning.
        /// + \ref COMPMODE_EQUAL : the event must be equal to *msg*.
        /// + \ref COMPMODE_SAMEKIND : the event is a same kind event (see MIDITimedMessage::IsSameKind()).
        /// + \ref COMPMODE_TIME : the behaviour is the same of FindEventNumber(time, event_num).
        /// \return **true** if an event matching _msg_ was found, **false** otherwise.
        bool                        FindEventNumber(const MIDITimedMessage& msg, int *event_num,
                                                    int mode = COMPMODE_EQUAL) const;
        /// Finds the first event in the track with the given time.
        /// \param[in] time the time to look for.
        /// \param[out] event_num contains the event number in the track if an event was found; otherwise it contains
        /// **-1** if *time* was invalid, or the number of the last event before *time*.
        /// \return **true** if an event with given time was found, **false** otherwise.
        bool                        FindEventNumber (MIDIClockTime time, int *event_num) const;

        /// Sets the default behaviour for the methods InsertEvent() and InsertNote(). This can be overriden
        /// by them by mean of the last (default) parameter.
        /// \param mode one of \ref ::INSMODE_INSERT, \ref ::INSMODE_REPLACE, #INSMODE_INSERT_OR_REPLACE,
        /// #INSMODE_INSERT_OR_REPLACE_BUT_NOTE; default is #INSMODE_INSERT_OR_REPLACE.
        /// \note #INSMODE_DEFAULT is used only in the insert methods (as default argument) and
        /// does nothing if given as parameter here.
        static void                 SetInsertMode(tInsMode mode);

        /// Used in the content analysis of a track. Values 0 ... 9 are track status types (which you can get with the
        /// GetStatus() method), others are for internal use.
        enum {
            TYPE_EMPTY = 0,         ///< Track is empty
            TYPE_MAIN = 1,          ///< Track has Main meta events (time, tempo, key ...) and no channel events
            TYPE_TEXT = 2,          ///< Track has only text meta events (probably lyrics)
            TYPE_CHAN = 3,          ///< Track is a normal channel track. You can find the channel with GetChannel()
            TYPE_IRREG_CHAN = 4,    ///< Track has channel events mixed with other (it can contain Main meta events)
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
            STATUS_DIRTY = 0x4000   ///< Track was edited, must call Analyze() to update its status
        };


    protected:
        /// Analyses the events in the track upgrading its status attribute. This is called automatically by GetType()
        /// when needed (i.e. the track was edited by one of the above methods and the status is no more valid), so has
        /// no utility for the user.
        void                        Analyze();

        /// \cond EXCLUDED
        std::vector<MIDITimedMessage>
                                    events;     // The buffer of events
        int                         status;     // A bitfield used to determine the track type
        signed char                 rec_chan;   // The channel for recordng, or -1 for all channels
        int                         time_shift; // The time shift in MIDI ticks
        unsigned int                in_port;    // The in port id for recording midi events
        unsigned int                out_port;   // The out port id for playing midi events

        static tInsMode             ins_mode;   // See SetInsertMode()
        /// \endcond
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
        /// Returns a pointer to the track the iterator is attached to.
        MIDITrack*                  GetTrack()                  { return track; }
        /// Returns a pointer to the track the iterator is attached to.
        const MIDITrack*            GetTrack() const 	        { return track;  }
        /// Sets the iterator track (causes a reset).
        void                        SetTrack(MIDITrack* trk);
        /// Returns the current time of the iterator.
        MIDIClockTime               GetCurrentTime() const      { return cur_time; }
        /// Returns the current event number in the track.
        unsigned int                GetCurrentEventNum() const  { return cur_ev_num; }
        /// Returns the current track program (-1 if not set).
        int16_t                     GetProgram() const          { return program; }
        /// Returns the current value for the given control (-1 if not set).
        int16_t                     GetControl(unsigned char c) const
                                                                { return controls[c]; }
        /// Returns the current bender value.
        int16_t                     GetBender() const           { return bender_value; }
        /// Returns the number of notes on at current time.
        int                         GetNotesOn() const          { return num_notes_on; }
        /// Returns **true** if the given note is on at current time.
        bool                        IsNoteOn(unsigned char n) const
                                                                { return (notes_on[n] > 0); }
        /// Returns **true** if the hold pedal is on at current time.
        bool                        IsPedalOn() const           { return controls[64] > 64; }
        /// Finds the next MIDITimedMessage in the track corresponding to the note off for the given note.
        /// \param[in] note : the note on
        /// \param[out] msg : a pointer to the note off message
        /// \return **true** if msg is valid, **false** otherwise (a note off was not found).
        bool                        FindNoteOff(unsigned char note, MIDITimedMessage** msg);
        /// Finds the next hold pedal off MIDITimedMessage in the track.
        /// \param[out] msg : a pointer to the pedal off message
        /// \return **true** if msg is valid, **false** otherwise (a pedal off was not found).
        bool                        FindPedalOff(MIDITimedMessage** msg);
        /// Goes to the given time, which becomes the current time. This sets the current event to the
        /// first event in the track with time greater or equal to _time_.
        bool                        GoToTime(MIDIClockTime time);
        /// Returns the next event in the track.
        /// \param *msg a pointer to the event in the MidiMultiTrack
        /// \return **true** if there is effectively a next event (we aren't at the end of the
        /// track), **false** otherwise (and **msg doesn't contain a valid value).
        bool                        GetNextEvent(MIDITimedMessage** msg);
        /// Gets the time of the next event in the track. It can be different from current time if
        /// at current time there are not events.
        /// \param t here we get the time of next event, if valid.
        /// \return **true** if there is effectively a next event (we aren't at the end of the
        /// track), **false** otherwise (*t doesn't contain a valid time).
        bool                        GetNextEventTime(MIDIClockTime* t) const;
        /// Returns **true** if at the current time there is an event of the same kind of _msg_
        /// (see MIDITimedMessage::IsSameKind()). The time of _msg_ is ignored.
        bool                        EventIsNow(const MIDITimedMessage& msg);
        //bool GoToNextEvent(); unused use GetNextEvent()

    protected:
        /// \cond EXCLUDED

        bool                        Process(const MIDITimedMessage *msg);
        void                        ScanEventsAtThisTime();
                                        // warning: this can be used only when we reach the first
                                        // event at a new time!

        MIDITrack*                  track;
        unsigned int                cur_ev_num;     // number of the current event
        MIDIClockTime               cur_time;       // current time

        int16_t                     program;        // current program change, or -1
        int16_t                     controls[128];  // value of every control change, or -1
        int16_t                     bender_value;	// last seen bender value
        unsigned char               num_notes_on;	// number of notes currently on
        unsigned char		        notes_on[128];  // 0 if off, or velocity
        /// \endcond
};


#endif
