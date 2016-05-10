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
//
// Copyright (C) 2010 V.R.Madgazin
// www.vmgames.com vrm@vmgames.com
//

// Questo file e il relativo .h sono stati confrontati con l'equivalente in jdksmid ed aggiornati

#include "../include/track.h"
#include "../include/matrix.h"


/* ************************************************************************************/
/* *                        C L A S S   M I D I T r a c k                             */
/* ************************************************************************************/

int MIDITrack::ins_mode = INSMODE_INSERT_OR_REPLACE;        /* NEW BY NC */


MIDITrack::MIDITrack(MIDIClockTime end_t) {
// a track always contains at least the MIDI_END event, so num_events > 0
    MIDITimedBigMessage msg;
    msg.SetDataEnd();
    msg.SetTime(end_t);
    events.push_back(msg);
}


MIDITrack::MIDITrack(const MIDITrack& trk) {
    events = trk.events;
}


MIDITrack::~MIDITrack() {}


MIDITrack& MIDITrack::operator=(const MIDITrack& trk) {
    events = trk.events;
    return *this;
}


void MIDITrack::Clear(bool mantain_end) {
    MIDIClockTime end = mantain_end ? GetEndTime() : 0;
    events.resize(0);       // destroys messages clearing sysex
    MIDITimedBigMessage msg;
    msg.SetDataEnd();
    msg.SetTime(end);
    events.push_back(msg);
}


bool MIDITrack::SetEndTime(MIDIClockTime end_t) {
    if (end_t < GetEndTime() && events.size() > 1)
        if (events[events.size() - 2].GetTime() > end_t) return false;
            // we tried to insert an MIDI_END before last (musical) event
    events.back().SetTime(end_t);
    return true;
}


void MIDITrack::SetChannel(int ch) {
    ch &= 0x0f;
    for (unsigned int i = 0; i < events.size(); i++) {
        if (GetEvent(i).IsChannelMsg())
            GetEvent(i).SetChannel(ch);
    }
}


MIDIClockTime MIDITrack::GetNoteLength(const MIDITimedBigMessage& msg) const {
    if(!msg.IsNoteOn()) return 0;                           // msg isn't aNoteOn
    int ev_num;
    if (!FindEventNumber(msg, &ev_num)) return 0;           // msg isn't in the track
    while (IsValidEventNum(ev_num)) {                       // search for the corresponding Note Off
        if (events[ev_num].IsNoteOff() &&
            events[ev_num].GetChannel() == msg.GetChannel() &&
            events[ev_num].GetNote() == msg.GetNote())
            return events[ev_num].GetTime() - msg.GetTime();// returns the length of the note
        ev_num++;
    }
        // this should not happen: there wasn't the corresponding Note Off
        // TODO: must we raise a warning?
    return TIME_INFINITE;
}


void MIDITrack::SetInsertMode( int mode )
{
    if ( mode != INSMODE_DEFAULT  )
        ins_mode = mode;
}

/* OLD!
bool MIDITrack::Insert(const MIDITimedBigMessage& msg) {
    if (msg.IsDataEnd()) return false;                  // DATA_END only managed by PutEvent

    if (GetEndTime() <= msg.GetTime()) {                // insert as last event
        SetEndTime(msg.GetTime());                      // adjust DATA_END
        return PutEvent(num_events-1, msg);             // insert just before DATA_END
    }
                                                        // binsearch copied from binsearch.h
    int min = 0,
        max = num_events - 1,
        mid = (max + min) / 2;
    while (max - min > 1) {
        if (GetEvent(mid) < msg) min = mid;
        else max = mid;
        mid = (max + min) / 2;
    }
    if (min < max && GetEvent(mid) < msg)       // max-min = 1; mid could be one of these
        mid++;

    WARNING! THIS IS DIFFERENT FROM ANALOGUE IN int_track.cpp AS MIDI MESSAGES AREN'T TOTALLY
    ORDERED (see midi_msg.operator== and midi_msg.operator< )

    min = mid;                                  // dummy, for remembering
    while (!EndOfTrack(mid) && GetEvent(mid).GetTime() == msg.GetTime()) {
        if (GetEvent(mid) == msg)               // element found
            return SetEvent(mid, msg);          // WARNING: I changed this (from false): review!
        mid++;
    }
    return PutEvent(min, msg);
}
*/



bool MIDITrack::InsertEvent(const MIDITimedBigMessage& msg, int mode) {
    if (msg.IsDataEnd()) return false;                  // DATA_END only auto managed

    if (GetEndTime() < msg.GetTime()) {                 // insert as last event
        SetEndTime(msg.GetTime());                      // adjust DATA_END
        events.insert(events.end() - 1, msg);           // insert just before DATA_END
        return true;
    }

    int ev_num;
        // find the first event in the track with time <= msg time
    FindEventNumber(msg.GetTime(), &ev_num);            // must return true
    int old_ev_num = ev_num;

    if (mode == INSMODE_DEFAULT)
        mode = ins_mode;                                // set inert mode to default behaviour

    switch (mode) {
        case INSMODE_INSERT:                            // always insert the event
                // find the right place among events with same time
            while (MIDITimedBigMessage::CompareEventsForInsert(msg, events[ev_num]) == 1)
                ev_num++;
            events.insert(events.begin() + ev_num, msg);
            return true;

        case INSMODE_REPLACE:                           // replace a same kind event, or do nothing
                // find a same kind event at same time
            while (IsValidEventNum(ev_num) && events[ev_num].GetTime() == msg.GetTime()) {
                if (MIDITimedBigMessage::IsSameKind(events[ev_num], msg)) {
                    events[ev_num] = msg;               // replace if found
                    return true;
                }
                ev_num++;
            }
            return false;                               // return false if not found

        case INSMODE_INSERT_OR_REPLACE:                 // replace a same kind, or insert
        case INSMODE_INSERT_OR_REPLACE_BUT_NOTE:        // (always insert notes)
                // first search for a same kind event, as above
            while (IsValidEventNum(ev_num) && events[ev_num].GetTime() == msg.GetTime()) {
                if (MIDITimedBigMessage::IsSameKind(events[ev_num], msg) &&
                     (mode == INSMODE_INSERT_OR_REPLACE || !msg.IsNote())) {
                    events[ev_num] = msg;               // replace if found
                    return true;
                }
                ev_num++;
            }
                // if not found a same kind insert the event
            ev_num = old_ev_num;
            while (MIDITimedBigMessage::CompareEventsForInsert(msg, events[ev_num]) == 1)
                ev_num++;
            events.insert(events.begin() + ev_num, msg);
            return true;                                // insert
    }

    return false;                                       // this should not happen
}


bool MIDITrack::InsertNote( const MIDITimedBigMessage& msg, MIDIClockTime len, int mode ) {
    if (!msg.IsNoteOn()) return false;

    MIDITimedBigMessage msgoff(msg);                    // set our NOTE_OFF message
    msgoff.SetType(NOTE_OFF);
    msgoff.SetTime(msg.GetTime() + len);

    int ev_num;

    if (mode == INSMODE_DEFAULT)
        mode = ins_mode;                                // set insert mode to default behaviour

    switch (mode) {
        case INSMODE_INSERT:                            // always insert the event
        case INSMODE_INSERT_OR_REPLACE_BUT_NOTE:
            InsertEvent(msg, mode);                     // always return true
            InsertEvent(msgoff, mode);
            return true;

        case INSMODE_REPLACE:                           // replace a same kind event, or do nothing
            if (FindEventNumber(msg, &ev_num, COMPMODE_SAMEKIND)) {
                                                        // search for a note on event (with same note)
                events.erase(events.begin() + ev_num);  // remove it
                while (IsValidEventNum(ev_num) ) {      // search for the note off
                    if (events[ev_num].IsNoteOff() && events[ev_num].GetNote() == msg.GetNote()) {
                        events.erase(events.begin() + ev_num);  // and remove
                        break;
                    }
                    ev_num++;
                }
                InsertEvent(msg, INSMODE_INSERT);       // insert note on (always return true)
                InsertEvent(msgoff, INSMODE_INSERT);    // insert note off
                return true;
            }
            return false;                               // return false if not found

        case INSMODE_INSERT_OR_REPLACE:                 // replace a same kind, or insert
            if (FindEventNumber(msg, &ev_num, COMPMODE_SAMEKIND)) {
                                                        // search for a note on event (with same note)
                events.erase(events.begin() + ev_num);  // remove it
                while (IsValidEventNum(ev_num)) {       // search for the note off
                    if (events[ev_num].IsNoteOff() && events[ev_num].GetNote() == msg.GetNote()) {
                        events.erase(events.begin() + ev_num);  // and remove
                        break;
                    }
                    ev_num++;
                }
            }
            InsertEvent(msg, INSMODE_INSERT);           // insert note on
            InsertEvent(msgoff, INSMODE_INSERT);        // insert note off
            return true;
    }
// NOTE: this calls InsertEvent and RemoveEvent always with correct arguments, so they should return true
// In the case of a memory allocation error, this would leave the track in an inconsistent state (notes erased
// without corresponding note inserted). The solution would be a backup of the whole track at the beginning, but
// this would be very expensive (for example in a MIDI format 0 file with all events in a track)
    return false;
}


/* OLD!!!!!
bool MIDITrack::Delete(const MIDITimedBigMessage& msg) {
    if (msg.IsDataEnd()) return false;                  // we can't delete MIDI_END event
    int min = 0,                                        // binsearch copied from binsearch.h
        max = num_events - 1,
        mid = (max + min) / 2;
    while (max - min > 1) {
        if (GetEvent(mid) < msg) min = mid;
        else max = mid;
        mid = (max + min) / 2;
    }
    if (min < max && GetEvent(mid) < msg)               // max-min = 1; mid could be one of these
        mid++;

    if (GetEvent(mid) == msg) {                         // mid is the place of msg
        DelEvent(mid);                                  // delete
        return true;
    }
    return false;                                       // not found
}
*/


bool MIDITrack::DeleteEvent( const MIDITimedBigMessage& msg )
{
    if (msg.IsDataEnd())
        return false;
    int ev_num;
    if (!FindEventNumber(msg, &ev_num))
        return false;
    events.erase(events.begin() + ev_num);
    return true;
}


bool MIDITrack::DeleteNote( const MIDITimedBigMessage& msg ) {
    if (!msg.IsNoteOn())
        return false;

    int ev_num;
    if (!FindEventNumber(msg, &ev_num))
        return false;
    events.erase(events.begin() + ev_num);
    while (IsValidEventNum(ev_num)) {
        if (events[ev_num].IsNoteOff() && events[ev_num].GetNote() == msg.GetNote()) {
            events.erase(events.begin() + ev_num);
            break;
        }
        ev_num++;
    }
    return true;
}


void MIDITrack::InsertInterval(MIDIClockTime start, MIDIClockTime length, const MIDITrack* src) {
    if (length == 0) return;

    CloseOpenEvents(start);                             // truncate notes, pedal, bender at start
    int start_n;
    if (FindEventNumber(start, &start_n)) {             // there are events after start time
        for (unsigned int i = start_n; i < events.size(); i++)
            events[i].AddTime(length);                  // moves these events
    }
    if(!src) return;                                    // we want only move events

    for(unsigned int i = 0; i < src->GetNumEvents(); i++) {
                                                        // else inserts events in src
        MIDITimedBigMessage msg = src->events[i];
        if (msg.GetTime() >= length) break;             // edit is too long
        msg.AddTime(start);                             // TODO: don't insert time, key and tempo messages if
                                                        // already present
        InsertEvent(msg);
    }
    CloseOpenEvents(start + length);                    // truncate at end
}


MIDITrack* MIDITrack::MakeInterval(MIDIClockTime start, MIDIClockTime end, MIDITrack* interval) const {
    interval->Clear();
    if (end > GetEndTime())                         // adjust end time
        end = GetEndTime();
    if (end <= start) return interval;              // return an empty track

    MIDITrack edittrack(*this);                     // copy original track to make edits on it;

    interval->SetEndTime(end - start);
    edittrack.CloseOpenEvents(start);               // truncate open events BEFORE start (so delete
                                                    // note off, etc in edit track)
    edittrack.CloseOpenEvents(end);                 // truncate at end

    int ev_num;                                     // now copy edittrack events to interval
    edittrack.FindEventNumber(start, &ev_num);      // an event surely exists
    MIDITimedBigMessage msg = edittrack.events[ev_num];
    for (; msg.GetTime() <= end && !msg.IsDataEnd(); ev_num++, msg = edittrack.events[ev_num]) {
        if (msg.GetTime() == start &&
            (msg.IsNoteOff() || msg.IsPedalOff() ||
            (msg.IsPitchBend() && msg.GetBenderValue() == 0)))
            continue;                               // skip these events at the beginning of the interval
       if (msg.GetTime() == end &&
            (msg.IsNoteOn() || msg.IsPedalOn() ||
            (events[ev_num].IsPitchBend() && events[ev_num].GetBenderValue() != 0)))
            continue;

        msg.SubTime(start);                         // adjust message time
        interval->InsertEvent(msg);                 // insert it in interval
    }
    return interval;
}


void MIDITrack::DeleteInterval(MIDIClockTime start, MIDIClockTime end) {
    if (end > GetEndTime())
        end = GetEndTime();                         // adjust end time
    if (end <= start) return;                       // nothing to do
    ClearInterval(start, end);
    int ev_num;
    FindEventNumber(end, &ev_num);                  // an event surely exists
    for (unsigned int i = ev_num; i < events.size(); i++)
        events[i].SubTime(end - start);             // shifts subsequents events
}


void MIDITrack::ClearInterval(MIDIClockTime start, MIDIClockTime end) {
    if (end > GetEndTime())
        end = GetEndTime();                         // adjust end time
    if (end <= start) return;                       // nothing to do

    CloseOpenEvents(start);                         // truncate open events BEFORE start (so delete
                                                    // note off, etc in edit track)
    CloseOpenEvents(end);                           // truncate at end
    int ev_num;
    int ev_to_remove = 0;
    FindEventNumber(start, &ev_num);                // an event surely exists
    while (events[ev_num].GetTime() == start &&
           (events[ev_num].IsNoteOff() || events[ev_num].IsPedalOff() ||
           (events[ev_num].IsPitchBend() && events[ev_num].GetBenderValue() == 0)))
        ev_num++;                                   // skip these events at the beginning of the interval
    while (events[ev_num].GetTime() < end) {        // surely we won't reach the DataEnd (end previously adjusted)
        ev_num++;
        ev_to_remove++;
    }
    events.erase(events.begin() + ev_num, events.begin() + ev_num + ev_to_remove);
                                                    // deletes events between start and end-1
    while (events[ev_num].GetTime() == end && !events[ev_num].IsDataEnd())
        if (events[ev_num].IsNoteOff() || events[ev_num].IsPedalOff() ||
            (events[ev_num].IsPitchBend() && events[ev_num].GetBenderValue() == 0))
            events.erase(events.begin() + ev_num);  // deletes NOTE OFF,PEDAL OFF and unneded PITCH BEND at end
        else
            ev_num++;
}


void MIDITrack::ReplaceInterval(MIDIClockTime start, MIDIClockTime length, bool sysex, const MIDITrack* src) {
    if (length == 0) return;

    ClearInterval(start, start + length);           // deletes all in the interval
    for(unsigned int i = 0; i < src->GetNumEvents(); i++) {
        MIDITimedBigMessage msg(src->GetEvent(i));  // inserts events
        if (msg.GetTime() >= length) break;         // edit is too long
        msg.AddTime(start);
        InsertEvent(msg);
    }
    CloseOpenEvents(start + length);                // truncate at end
}


void MIDITrack::CloseOpenEvents(MIDIClockTime t) {
    if (t == 0 || t >= GetEndTime()) return;
                                            // there aren't open events at time 0 or after INT_END
    MIDITrackIterator iter(this);
    char ch  = iter.GetMIDIChannel();       // assumes all channel messages with the same channel!
    MIDITimedBigMessage* msgp;
    MIDITimedBigMessage msg;

    iter.GoToTime(t);
    msg.SetTime(t);                         // set right time for msg
    if (iter.NotesOn()) {                   // there are notes on at time t
        for (int i = 0; i < 0x7f; i++) {
            msg.SetNoteOn(ch, i, 100);
            if (iter.IsNoteOn(i) && !iter.EventIsNow(msg)) {
                if (iter.FindNoteOff(i, &msgp))
                    DeleteEvent(*msgp);     // delete original note OFF
                msg.SetNoteOff(ch, i, 0);
                InsertEvent(msg);           // insert a note OFF at time t
            }
        }
    }
    msg.SetControlChange(ch, C_DAMPER, 127);
    if (iter.IsPedalOn()&& !iter.EventIsNow(msg))   {               // pedal (CTRL 64) on at time t
        iter.GoToTime(t);               // TODO: ia this necessary? I don't remember
        if (iter.FindPedalOff(&msgp))
            DeleteEvent(*msgp);         // delete original pedal OFF
        msg.SetControlChange(ch, C_DAMPER, 0);
        InsertEvent(msg);               // insert a pedal OFF at time t
        msg.SetTime(t);
    }
    msg.SetPitchBend(ch, 0);
    if (iter.GetBender()) {                 // pitch bend not 0 at time t
        iter.GoToTime(t);                   // updates iter
        while (iter.GetCurEvent(&msgp)) {   // chase and delete all subsequent bender messages,
            if (msgp->IsPitchBend()) {      // until bender == 0
                short val = msgp->GetBenderValue(); // remember bender value
                DeleteEvent(*msgp);
                if (val == 0) break;
            }
        }
        InsertEvent(msg);                   // insert a new pitch bend = 0 at time t
    }
}



bool MIDITrack::FindEventNumber(const MIDITimedBigMessage& msg, int* event_num, int mode) const {
    if (msg.GetTime() > GetEndTime()) {
        *event_num = -1;                        // returns -1 in event_num
        return false;
    }

    int min = 0,                                // binary search with deferred detection algorithm
        max = events.size() - 1;
    while (min < max) {
        unsigned int mid = min + (max - min) / 2;
        if (GetEventAddress(mid)->GetTime() < msg.GetTime())
            min = mid + 1;
        else
            max = mid;
    }
                                                // the algorithm stops on 1st msg with time <= msg.GetTime()
    *event_num = min;                           // if event_num != -1 this is the last ev with time <= msg.GetTime()
    // if event_num != -1 this is the last ev with time <= t

    if (mode == COMPMODE_TIME)                  // find first message with same time
        return GetEventAddress(min)->GetTime() == msg.GetTime();
                                                // true if there is a message

    while (IsValidEventNum(min) && GetEventAddress(min)->GetTime() == msg.GetTime()) {
        if (mode == COMPMODE_EQUAL) {           // find equal events
            if (GetEvent(min) == msg) {         // element found
                *event_num = min;
                return true;
            }
        }
        else if (mode == COMPMODE_SAMEKIND) {   // find same kind events
            if (MIDITimedBigMessage::IsSameKind(GetEvent(min), msg)) {   // element found
                *event_num = min;
                return true;
            }
        }
        min++;
    }
    return false;                               // element not found
}


bool MIDITrack::FindEventNumber(MIDIClockTime time, int* event_num) const {
    if (time > GetEndTime()) {
        *event_num = -1;                        // returns -1 in event_num
        return false;
    }

    int min = 0,                                // binary search with deferred detection algorithm
        max = events.size() - 1;
    while (min < max ) {
        unsigned int mid = min + (max - min) / 2;
        if (GetEventAddress(mid)->GetTime() < time)
            min = mid + 1;
        else
            max = mid;
    }
                                                // the algorithm stops on 1st msg with time <= msg.GetTime()
    *event_num = min;                           // if event_num != -1 this is the last ev with time <= t
    if (GetEventAddress(min)->GetTime() == time)
        return true;                            // found event with time == t
    return false;                               // found event with time < t
}

/* OLD!!!!!
int MIDITrack::FindEventNumber(const MIDITimedBigMessage& m) const {
// returns -1 if not found
    if (num_events == 0) return -1;             // binsearch copied from binsearch.h
    int min = 0,
        max = num_events - 1,
        mid = (max + min) / 2;
    while (max - min > 1) {
        if (GetEvent(mid) < m) min = mid;
        else max = mid;
        mid = (max + min) / 2;
    }
    if (min < max && GetEvent(mid) < m)         // max-min = 1; mid could be one of these
        mid++;
    if (GetEvent(mid) == m)                     // element found
        return mid;
    return -1;
}


int MIDITrack::FindEventNumber(MIDIClockTime time) const {
    if (num_events == 0 || time > GetEndTime()) return -1;
    int min = 0,                                        // binary search
        max = num_events - 1,
        mid = (max + min) / 2;
    while (max - min > 1) {
        if (GetEvent(mid).GetTime() < time) min = mid;
        else max = mid;
        mid = (max + min) / 2;
    }
    if (GetEvent(mid).GetTime() < time)     // mid.GetTime() should be <= time
        return ++mid;
    while(mid && GetEvent(mid-1).GetTime() == time)
        mid--;                              // there are former events with same time
    return mid;
}
*/


/*


/* ************************************************************************************/
/* *                 C L A S S   M I D I T r a c k I t e r a t o r                    */
/* ************************************************************************************/

// this is interely added by me!!!! NC

MIDITrackIterator::MIDITrackIterator(MIDITrack* trk) : track(trk) {
    FindChannel();
    Reset();
}


void MIDITrackIterator::SetTrack(MIDITrack* trk) {
        track = trk;
        FindChannel();
        Reset();
}


void MIDITrackIterator::Reset() {   // use this if you edit the content of the track
    // go to time zero
    cur_time = 0;
    cur_ev_num = 0;

    // reset midi status
    program = 0xff;
    for (int i = 0; i < 128; i++) {
        controls[i] = 0xff;
        notes_on[i] = 0;
    }
    bender_value = 0;
    num_notes_on = 0;
    ScanEventsAtThisTime();                                 // process all messages at this time
}



bool MIDITrackIterator::FindNoteOff(uchar note, MIDITimedBigMessage** msg) {
    for (unsigned int i = cur_ev_num; i < track->GetNumEvents(); i ++) {
        *msg = track->GetEventAddress(i);
        if ((*msg)->IsNoteOff() && (*msg)->GetNote() == note)
            return true;                //  msg is the correct note off
    }
    return false;
}


bool MIDITrackIterator::FindPedalOff(MIDITimedBigMessage** msg) {
    for (unsigned int i = cur_ev_num; i < track->GetNumEvents(); i ++) {
        *msg = track->GetEventAddress(i);
        if ((*msg)->IsControlChange() && (*msg)->GetController() == 64 && (*msg)->GetControllerValue() < 64)
            return true;                //  msg is the correct pedal off
    }
    return false;
}


bool MIDITrackIterator::GetCurEvent(MIDITimedBigMessage** msg, MIDIClockTime end /* = 0xffffffff */) {
    if (!track->IsValidEventNum(cur_ev_num)) return false;
    if (end < cur_time) return false;                       // end lesser than cur_time
    *msg = track->GetEventAddress(cur_ev_num);
    MIDIClockTime new_time = (*msg)->GetTime();
    if (new_time > cur_time) {                              // is new time > cur_time?
        if (new_time > end)
            return false;
        cur_time = new_time;
        ScanEventsAtThisTime();                             // update status processing all messages at this time
    }
    cur_ev_num++;                                           // increment message pointer
    return true;
}


MIDIClockTime MIDITrackIterator::GetCurEventTime() const {
    if (!track->IsValidEventNum(cur_ev_num)) return 0xffffff;     // we are at the end of track
    return track->GetEventAddress(cur_ev_num)->GetTime();
}


bool MIDITrackIterator::EventIsNow(const MIDITimedBigMessage& msg) {
    MIDITimedBigMessage msg1;
    int ev_num;

    track->FindEventNumber(cur_time, &ev_num);
    while(track->IsValidEventNum(ev_num) && (msg1 = track->GetEvent(ev_num)).GetTime() == cur_time) {
        if (MIDITimedBigMessage::IsSameKind(msg1, msg)) return true;
        ev_num++;
    }
    return false;
}


bool MIDITrackIterator::GoToNextEvent() {
    if (!track->IsValidEventNum(cur_ev_num)) return false;
    cur_ev_num++;
    MIDITimedBigMessage* msg = track->GetEventAddress(cur_ev_num);
    if (msg->GetTime() > cur_time) {
        cur_time = msg->GetTime();
        ScanEventsAtThisTime();                             // process all messages at this time
    }
    return true;
}


bool MIDITrackIterator::GoToTime(MIDIClockTime time) {
    MIDITimedBigMessage* msg;

    if (time > track->GetEndTime()) return false;
    if (time <= cur_time)
        Reset();
    while (GetCurEventTime() < time)
        GetCurEvent(&msg);
    cur_time = time;
    ScanEventsAtThisTime();
    return true;
}


void MIDITrackIterator::FindChannel() {
    channel = 0xff;
    for (unsigned int i = 0; i < track->GetNumEvents(); i ++) {
        if (track->GetEvent(i).IsChannelMsg()) {
            channel = track->GetEvent(i).GetChannel();
            return;
        }
    }
}


bool MIDITrackIterator::Process(const MIDITimedBigMessage *msg) {

    // is it a normal MIDI channel message?
    if(msg->IsChannelMsg()) {
        if (msg->GetChannel() != channel) return false;
        switch (msg->GetType()) {
            case NOTE_OFF :
                if(notes_on[msg->GetNote()]) {
                    notes_on[msg->GetNote()] = 0;
                    num_notes_on--;
                }
                break;
            case NOTE_ON :
                if (msg->GetVelocity()) {
                    if (!notes_on[msg->GetNote()]) {
                        notes_on[msg->GetNote()] = msg->GetVelocity();
                        num_notes_on++;
                    }
                }
                else {
                    if (notes_on[msg->GetNote()]) {
                        notes_on[msg->GetNote()] = 0;
                        num_notes_on--;
                    }
                }
                break;
            case PITCH_BEND :
                bender_value = msg->GetBenderValue();
                break;
            case CONTROL_CHANGE :
                controls[msg->GetController()] = msg->GetControllerValue();
                break;
            case PROGRAM_CHANGE :
                program = msg->GetPGValue();
                break;
        }
        return true;
    }
    return false;
}


void MIDITrackIterator::ScanEventsAtThisTime() {
// process all messages up to and including this time only
// warning: this can be used only when we reach the first event at a new time!

    MIDIClockTime oldtime = cur_time;
    int oldnum = cur_ev_num;
    //cur_ev_num = track->FindEventNumber(cur_time);      // is it necessary? I think no
    MIDITimedBigMessage *msg;

    while(GetCurEvent(&msg, oldtime))
        Process(msg);
    cur_time = oldtime;
    cur_ev_num = oldnum;



////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////










/* NEW FUNCTIONS BY NC */







/* QUI NON E' USATA
MIDIClockTime MIDITrack::NoteLength( const MIDITimedBigMessage& msg ) const
{
    if ( !msg.IsNoteOn() )
    {
        return 0;
    }
    int event_num;
    const MIDITimedBigMessage* msgp;
    if ( !FindEventNumber( msg, &event_num, COMPMODE_EQUAL ) )
    {
        return 0;
    }
    for (int i = event_num; i < num_events; i++)
    {
        msgp = GetEvent( i );
        if ( msgp->IsNoteOff() &&
             msgp->GetChannel() == msg.GetChannel() &&
             msgp->GetNote() == msg.GetNote()
            )
        {
            return msgp->GetTime() - msg.GetTime();
        }
    }
    return GetLastEventTime() - msg.GetTime();
}
*/



/*  MEGLIO QUELLA CON L'ITERATOR ???
void MIDITrack::CloseOpenEvents(MIDIClockTime t) {
    if ( t == 0 ) return;                   // there aren't open events at time 0

    MIDIMatrix matrix;
    MIDITimedBigMessage msg;

    int ev_num = 0;
    while ( ev_num < num_events &&
            ( msg = *GetEvent( ev_num ) ).GetTime() < t )
    {
        matrix.Process( msg );
        ev_num++;
    }
    while ( ev_num < num_events &&
            ( msg = *GetEvent( ev_num ) ).GetTime() == t )
    {
        if ( msg.IsNoteOff() || msg.IsPedalOff() )
        {
            matrix.Process ( msg );
            ev_num++;
        }
    }                                           // ev_num is now the index of 1st event with timw > t

    for (int ch = 0; ch < 16; ch++)            // for every channel ...
    {
        int note_count = matrix.GetChannelCount( ch );      // get the number of notes on of the channel
        for ( int note = 0; note < 0x7f && note_count > 0; note++ )
        {                                                   // search which notes are on
            for (int i = matrix.GetNoteCount( ch, note ); i > 0; i-- )
            {                                               // if the note is on ... (i should normally be 1)
                msg.SetNoteOff( ch, note, 0 );
                msg.SetTime( t );
                InsertEvent( msg );                         // ... insert a note off at time t
                ev_num++;                                   // and adjust ev_num
                for (int j = ev_num; j < num_events; j++)
                {
                    msg = *GetEvent( j );                   // search the corresponding note off after t ...
                    if ( msg.IsNoteOff() &&
                         msg.GetChannel() == ch &&
                         msg.GetNote() == note )
                    {
                        DeleteEvent( msg );                 // ... and delete it
                        break;
                    }
                }
                note_count--;
            }
        }

        if ( matrix.GetHoldPedal( ch ) )
        {
            msg.SetControlChange(ch, C_DAMPER, 0);
            msg.SetTime( t );
            InsertEvent( msg );
            ev_num++;
            for (int i = ev_num; i < num_events; i++)
            {
                msg = *GetEvent( i );                       // search the corresponding note off after t ...
                if ( msg.IsPedalOff() &&
                     msg.GetChannel() == ch )
                {
                    DeleteEvent( msg );                     // ... and delete it
                    break;
                }
            }
        }
    }
}

*/






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
//


/*
#include "../include/world.h"
#include "../include/track.h"

//#include "jdksmidi/matrix.h"


namespace jdksmidi
{



bool MIDITrack::InsertEvent( const MIDITimedBigMessage& msg, int _ins_mode ) {
    bool ret = false;

    if (msg.IsDataEnd()) return false;                  // DATA_END only auto managed

    if ( !IsTrackEnded() )                              // add DATA_END if needed
    {
        SetEndTime( GetLastEventTime() );
    }

    if (GetLastEventTime() < msg.GetTime()) {           // insert as last event
        SetEndTime(msg.GetTime());                      // adjust DATA_END
        return PutEvent(msg, num_events-1);             // insert just before DATA_END
    }

    int event_num;
    FindEventNumber(msg.GetTime(), &event_num);         // must return true
    int old_event_num = event_num;

    if ( _ins_mode == INSMODE_DEFAULT )
    {
        _ins_mode = ins_mode;                           // set inert mode to default behaviour
    }

    switch ( _ins_mode )
    {
        case INSMODE_INSERT:                            // always insert the event
            while ( MIDITimedBigMessage::CompareEventsForInsert( msg, *GetEvent(event_num) ) == 1 )
                event_num++;
            ret = PutEvent( msg, event_num );
            break;

        case INSMODE_REPLACE:                           // replace a same kind event, or do nothing
            while ( event_num < num_events && GetEvent(event_num)->GetTime() == msg.GetTime() )
            {                                           // search for a same kind event
                if ( MIDITimedBigMessage::IsSameKind(*GetEvent(event_num), msg) )
                {
                    ret = SetEvent(event_num, msg);     // replace if found
                    break;
                }
                event_num++;
            }
            break;

        case INSMODE_INSERT_OR_REPLACE:                 // replace a same kind, or insert
        case INSMODE_INSERT_OR_REPLACE_BUT_NOTE:        // (always insert notes)
            while ( event_num < num_events && GetEvent(event_num)->GetTime() == msg.GetTime() )
            {
                if ( MIDITimedBigMessage::IsSameKind(*GetEvent(event_num), msg) &&
                     ( _ins_mode == INSMODE_INSERT_OR_REPLACE || !msg.IsNote() )
                    )
                {
                    ret = SetEvent(event_num, msg);     // replace if found
                    break;
                }
                event_num++;
            }
            if (event_num == num_events || msg.GetTime() != GetEvent(event_num)->GetTime() )
            {
                event_num = old_event_num;
                while ( MIDITimedBigMessage::CompareEventsForInsert( msg, *GetEvent(event_num) ) == 1 )
                    event_num++;
                ret =  PutEvent( msg, event_num );      // else insert
            }
            break;

        default:
            break;
    }
    return ret;
}

bool MIDITrack::InsertNote( const MIDITimedBigMessage& msg, MIDIClockTime len, int _ins_mode ) {
    bool ret = false;
    if ( !msg.IsNoteOn() )
        return false;

    MIDITimedBigMessage msgoff(msg);                    // set our NOTE_OFF message
    msgoff.SetType(NOTE_OFF);
    msgoff.SetTime( msg.GetTime() + len);

    int event_num;

    if ( _ins_mode == INSMODE_DEFAULT )
    {
        _ins_mode = ins_mode;                           // set insert mode to default behaviour
    }

    switch ( _ins_mode )
    {
        case INSMODE_INSERT:                            // always insert the event
        case INSMODE_INSERT_OR_REPLACE_BUT_NOTE:
            ret = InsertEvent(msg, _ins_mode);
            ret &= InsertEvent(msgoff, _ins_mode);
            break;

        case INSMODE_REPLACE:                           // replace a same kind event, or do nothing
            if ( FindEventNumber(msg, &event_num, COMPMODE_SAMEKIND) )
            {                                           // search for a note on event (with same note)
                RemoveEvent(event_num);                 // remove it
                while ( event_num < num_events )        // search for the note off
                {
                    MIDITimedBigMessage* msgp = GetEvent( event_num );
                    if ( msgp->IsNoteOff() && msgp->GetNote() == msg.GetNote() )
                    {
                        RemoveEvent( event_num );       // and remove
                        break;
                    }
                    event_num++;
                }
                ret = InsertEvent(msg, _ins_mode);      // insert note on
                ret &= InsertEvent(msgoff, _ins_mode);  // insert note off
            }
            break;

        case INSMODE_INSERT_OR_REPLACE:                 // replace a same kind, or insert
            if ( FindEventNumber(msg, &event_num, COMPMODE_SAMEKIND) )
            {                                           // search for a note on event (with same note)
                RemoveEvent(event_num);                 // remove it
                while ( event_num < num_events )        // search for the note off
                {
                    MIDITimedBigMessage* msgp = GetEvent( event_num );
                    if ( msgp->IsNoteOff() && msgp->GetNote() == msg.GetNote() )
                    {
                        RemoveEvent( event_num );       // and remove
                        break;
                    }
                    event_num++;
                }
            }
            ret = InsertEvent(msg, INSMODE_INSERT);     // insert note on
            ret &= InsertEvent(msgoff, INSMODE_INSERT); // insert note off
            break;

        default:
            break;
    }
// NOTE: this calls InsertEvent and RemoveEvent always with correct arguments, so they should return true
// In the case of a memory allocation error, this would leave the track in an inconsistent state (notes erased
// without corresponding note inserted). The solution would be a backup of the whole track at the beginning, but
// this would be very expensive (for example in a MIDI format 0 file with all events in a track)
    return ret;
}


bool MIDITrack::DeleteEvent( const MIDITimedBigMessage& msg )
{
    if ( msg.IsDataEnd() )
    {
        return false;
    }
    int event_num;
    if ( !FindEventNumber( msg, &event_num ) )
    {
        return false;
    }
    return RemoveEvent( event_num );
}

bool MIDITrack::DeleteNote( const MIDITimedBigMessage& msg ) {
    bool ret = false;
    if ( !msg.IsNoteOn() )
        return false;

    int event_num;
    if ( !FindEventNumber(msg, &event_num) )
    {
        return false;
    }
    ret = RemoveEvent( event_num );
    while ( event_num < num_events )
    {
        MIDITimedBigMessage* msgp = GetEvent( event_num );
        if ( msgp->IsNoteOff() && msgp->GetNote() == msg.GetNote() )
        {
            RemoveEvent( event_num );
            break;
        }
        event_num++;
    }
    return ret;
}

#if 0

bool  MIDITrack::Delete ( int start_event, int num )
{
    // TO DO: Delete
    return true;
}



#endif



bool MIDITrack::FindEventNumber(const MIDITimedBigMessage& msg, int* event_num, int _comp_mode) const {
    if (num_events == 0 || msg.GetTime() > GetLastEventTime())
    {
        *event_num = -1;                        // returns -1 in event_num
        return false;
    }

    int min = 0,                                // binary search with deferred detection algorithm
        max = num_events - 1;
    while ( min < max )
    {
        unsigned int mid = min + ( max - min ) / 2;
        if ( GetEvent(mid)->GetTime() < msg.GetTime() )
            min = mid + 1;
        else
            max = mid;
    }
                                                // the algorithm stops on 1st msg with time <= msg.GetTime()
    *event_num = min;                           // if event_num != -1 this is the last ev with time <= msg.GetTime()
    // if event_num != -1 this is the last ev with time <= t

    if ( _comp_mode == COMPMODE_TIME )          // find first message with same time
        return GetEvent(min)->GetTime() == msg.GetTime();   // true if there is a message

    while ( min < num_events && GetEvent(min)->GetTime() == msg.GetTime() )
    {
        if ( _comp_mode == COMPMODE_EQUAL )     // find equal events
        {
            if (*GetEvent(min) == msg)          // element found
            {
                *event_num = min;
                return true;
            }
        }
        else if ( _comp_mode == COMPMODE_SAMEKIND )     // find same kind events
        {
            if ( MIDITimedBigMessage::IsSameKind( *GetEvent(min), msg ) )    // element found
            {
                *event_num = min;
                return true;
            }
        }
        min++;
    }

    return false;                               // element not found
}


bool MIDITrack::FindEventNumber(MIDIClockTime time, int* event_num) const {
    if (num_events == 0 || time > GetLastEventTime())
    {
        *event_num = -1;                        // returns -1 in event_num
        return false;
    }

    int min = 0,                                // binary search with deferred detection algorithm
        max = num_events - 1;
    while (min < max )
    {
        unsigned int mid = min + ( max - min ) / 2;
        if ( GetEvent(mid)->GetTime() < time )
            min = mid + 1;
        else
            max = mid;
    }
                                                // the algorithm stops on 1st msg with time <= msg.GetTime()
    *event_num = min;                           // if event_num != -1 this is the last ev with time <= t
    if ( GetEvent(min)->GetTime() == time )
        return true;                            // found event with time == t

    return false;                               // found event with time < t
}

MIDIClockTime MIDITrack::NoteLength( const MIDITimedBigMessage& msg ) const
{
    if ( !msg.IsNoteOn() )
    {
        return 0;
    }
    int event_num;
    const MIDITimedBigMessage* msgp;
    if ( !FindEventNumber( msg, &event_num, COMPMODE_EQUAL ) )
    {
        return 0;
    }
    for (int i = event_num; i < num_events; i++)
    {
        msgp = GetEvent( i );
        if ( msgp->IsNoteOff() &&
             msgp->GetChannel() == msg.GetChannel() &&
             msgp->GetNote() == msg.GetNote()
            )
        {
            return msgp->GetTime() - msg.GetTime();
        }
    }
    return GetLastEventTime() - msg.GetTime();
}


void MIDITrack::CloseOpenEvents(MIDIClockTime t) {
    if ( t == 0 ) return;                   // there aren't open events at time 0

    MIDIMatrix matrix;
    MIDITimedBigMessage msg;

    int ev_num = 0;
    while ( ev_num < num_events &&
            ( msg = *GetEvent( ev_num ) ).GetTime() < t )
    {
        matrix.Process( msg );
        ev_num++;
    }
    while ( ev_num < num_events &&
            ( msg = *GetEvent( ev_num ) ).GetTime() == t )
    {
        if ( msg.IsNoteOff() || msg.IsPedalOff() )
        {
            matrix.Process ( msg );
            ev_num++;
        }
    }                                           // ev_num is now the index of 1st event with timw > t

    for (int ch = 0; ch < 16; ch++)            // for every channel ...
    {
        int note_count = matrix.GetChannelCount( ch );      // get the number of notes on of the channel
        for ( int note = 0; note < 0x7f && note_count > 0; note++ )
        {                                                   // search which notes are on
            for (int i = matrix.GetNoteCount( ch, note ); i > 0; i-- )
            {                                               // if the note is on ... (i should normally be 1)
                msg.SetNoteOff( ch, note, 0 );
                msg.SetTime( t );
                InsertEvent( msg );                         // ... insert a note off at time t
                ev_num++;                                   // and adjust ev_num
                for (int j = ev_num; j < num_events; j++)
                {
                    msg = *GetEvent( j );                   // search the corresponding note off after t ...
                    if ( msg.IsNoteOff() &&
                         msg.GetChannel() == ch &&
                         msg.GetNote() == note )
                    {
                        DeleteEvent( msg );                 // ... and delete it
                        break;
                    }
                }
                note_count--;
            }
        }

        if ( matrix.GetHoldPedal( ch ) )
        {
            msg.SetControlChange(ch, C_DAMPER, 0);
            msg.SetTime( t );
            InsertEvent( msg );
            ev_num++;
            for (int i = ev_num; i < num_events; i++)
            {
                msg = *GetEvent( i );                       // search the corresponding note off after t ...
                if ( msg.IsPedalOff() &&
                     msg.GetChannel() == ch )
                {
                    DeleteEvent( msg );                     // ... and delete it
                    break;
                }
            }
        }
    }
}


/*
    if (iter.NotesOn()) {                   // there are notes on at time t
        for (int i = 0; i < 0x7f; i++) {
            msg.SetNoteOn(ch, i, 100);
            if (iter.IsNoteOn(i) && !iter.EventIsNow(msg)) {
                if (iter.FindNoteOff(i, &msgp))
                    Delete(*msgp);          // delete original note OFF
                msg.SetNoteOff(ch, i, 0);
                msg.SetTime(t - 1);
                Insert(msg);            // insert a note OFF at time t-1
                msg.SetTime(t);
            }
        }
    }
    msg.SetControlChange(ch, C_DAMPER, 127);
    if (iter.IsPedalOn()&& !iter.EventIsNow(msg))   {               // pedal (CTRL 64) on at time t
        iter.GoToTime(t);               // TODO: ia this necessary? I don't remember
        if (iter.FindPedalOff(&msgp))
            Delete(*msgp);                  // delete original pedal OFF
        msg.SetControlChange(ch, C_DAMPER, 0);
        msg.SetTime(t - 1);
        Insert(msg);                    // insert a pedal OFF at time t-1
        msg.SetTime(t);
    }
    msg.SetPitchBend(ch, 0);
    if (iter.GetBender()) {                 // pitch bend not 0 at time t
        iter.GoToTime(t);                   // updates iter
        while (iter.GetCurEvent(&msgp)) {  // chase and delete all subsequent bender messages,
            if (msgp->IsPitchBend()) {      // until bender == 0
                short val = msgp->GetBenderValue(); // remember bender value
                Delete(*msgp);
                if (val == 0) break;
            }
        }
        Insert(msg);                        // insert a new pitch bend = 0 at time t
    }
}

*/



/* ***** FROM N. CASSETTA




void MIDITrack::ChangeChannel(int ch) {
    ch &= 0x0f;
    for (int i = 0; i < num_events; i++) {
        if (GetEventAddress(i)->IsChannelMsg())
            GetEventAddress(i)->SetChannel(ch);
    }
}


// WARNING! THESE ARE DIFFERENT FROM ANALOGUE IN int_track.cpp:
//  1) IN A MIDI TRACK WE MUST CLOSE OPEN EVENTS WHEN COPYING OR DELETING INTERVALS
//  2) DEALING WITH META EVENTS (timesigs, keysig, text, markers, etc) IS TOO DIFFICULT AT
//     THIS (low) LEVEL, SO ONLY sysex DATA IS COPIED (if requested). FOR ADJUSTING META DATA
//     IS MUCH EASIER RECOMPOSE TRACK 0 BY THE MasterTrackComposer



void MIDITrack::InsertInterval(MIDIClockTime start, MIDIClockTime length, bool sysex, const MIDITrack* src) {
    if (length == 0) return;

    CloseOpenEvents(start);                         // truncate notes, pedal, bender at start
    int start_n = FindEventNumber(start);
    if (start_n != -1) {                            // there are events after start time
        for (int i = start_n; i < num_events; i++)  // moves these events
            GetEventAddress(i)->AddTime(length);
    }
    if(!src) return;                                // we want only move events

    for(int i = 0; i < src->GetNumEvents(); i++) {  // else inserts other events
        MIDITimedBigMessage msg(src->GetEvent(i));
                // msg is created and destroyed every time so no memory leaks with sysex (hope!)
        if (msg.GetTime() >= length) break;         // edit is too long
        if (!msg.IsSysEx() || (msg.IsSysEx() && sysex)) {
            msg.AddTime(start);
            Insert(msg);
        }
    }
    CloseOpenEvents(start + length);                // truncate at end
}


MIDITrack* MIDITrack::MakeInterval(MIDIClockTime start, MIDIClockTime end, MIDITrack* interval) const {
    if (end <= start) return interval;
    MIDIClockTime length = end - start;
    MIDITrack edittrack(*this);      // copy original track to make edits on it;

    interval->Clear();               // clear
    interval->SetEndTime(length);
    if (start > 0)
        edittrack.CloseOpenEvents(start);
                                    // truncate open events BEFORE start (so delete note off, etc
                                    // in edit track. If start == 0, no need to truncate
    edittrack.CloseOpenEvents(end); // truncate at end
    int start_n = edittrack.FindEventNumber(start);
                                    // now copy edittrack events to interval
    if(start_n != -1) {             // there are events
        for (int i = start_n; edittrack.GetEventAddress(i)->GetTime() < end; i++) {
            MIDITimedBigMessage msg(edittrack.GetEvent(i));
                                    // msg is created and destroyed every time so no memory leaks with sysex (hope!)
            if (msg.IsDataEnd()) break;
            if (msg.IsChannelMsg()) {
                msg.SubTime(start); // only channel messages are copied
                interval->Insert(msg);
            }
        }
    }
    return interval;
}


void MIDITrack::DeleteInterval(MIDIClockTime start, MIDIClockTime end) {
    if (end <= start) return;
    CloseOpenEvents(start);
    CloseOpenEvents(end);
    int start_n = FindEventNumber(start);   // first event with time >= start
    if (start_n == -1) return;              // no events after start: nothing to do
    MIDIClockTime length = end - start;
    while (GetEvent(start_n).GetTime() < end && !GetEvent(start_n).IsDataEnd())
                // DataEnd is not deleted by DelEvent, so we could enter in an infinite loop
        DelEvent(start_n);                  // deletes events between start and end
    for (int i = start_n; i < num_events; i++)
        GetEvent(i).SubTime(length);        // shifts subsequents
    Shrink();
}


void MIDITrack::ClearInterval(MIDIClockTime start, MIDIClockTime end) {
    if (end <= start) return;
    CloseOpenEvents(start);
    CloseOpenEvents(end);
    int start_n = FindEventNumber(start);
    if (start_n == -1) return;
    while (GetEventAddress(start_n)->GetTime() < end && !GetEventAddress(start_n)->IsDataEnd())
                // DataEnd is not deleted by DelEvent, so we could enter in an infinite loop
        DelEvent(start_n);
    Shrink();
}


void MIDITrack::ReplaceInterval(MIDIClockTime start, MIDIClockTime length, bool sysex, const MIDITrack* src) {
    if (length == 0) return;

    ClearInterval(start, start + length);           // deletes all in the interval
    for(int i = 0; i < src->GetNumEvents(); i++) {  // inserts events
        MIDITimedBigMessage msg(src->GetEvent(i));
                // msg is created and destroyed every time so no memory leaks with sysex (hope!)
        if (msg.GetTime() >= length) break;         // edit is too long
        msg.AddTime(start);
        Insert(msg);
    }
    CloseOpenEvents(start + length);                // truncate at end
}


void MIDITrack::CloseOpenEvents(MIDIClockTime t) {
    if (t == 0 || t > GetEndTime()) return;
                                            // there aren't open events at time 0 or after INT_END
    MIDITrackIterator iter(this);
    char ch  = iter.GetMIDIChannel();       // assumes all channel messages with the same channel!
    MIDITimedBigMessage* msgp;
    MIDITimedBigMessage msg;

    iter.GoToTime(t);
    msg.SetTime(t);                         // set right time for msg
    if (iter.NotesOn()) {                   // there are notes on at time t
        for (int i = 0; i < 0x7f; i++) {
            msg.SetNoteOn(ch, i, 100);
            if (iter.IsNoteOn(i) && !iter.EventIsNow(msg)) {
                if (iter.FindNoteOff(i, &msgp))
                    Delete(*msgp);          // delete original note OFF
                msg.SetNoteOff(ch, i, 0);
                msg.SetTime(t - 1);
                Insert(msg);            // insert a note OFF at time t-1
                msg.SetTime(t);
            }
        }
    }
    msg.SetControlChange(ch, C_DAMPER, 127);
    if (iter.IsPedalOn()&& !iter.EventIsNow(msg))   {               // pedal (CTRL 64) on at time t
        iter.GoToTime(t);               // TODO: ia this necessary? I don't remember
        if (iter.FindPedalOff(&msgp))
            Delete(*msgp);                  // delete original pedal OFF
        msg.SetControlChange(ch, C_DAMPER, 0);
        msg.SetTime(t - 1);
        Insert(msg);                    // insert a pedal OFF at time t-1
        msg.SetTime(t);
    }
    msg.SetPitchBend(ch, 0);
    if (iter.GetBender()) {                 // pitch bend not 0 at time t
        iter.GoToTime(t);                   // updates iter
        while (iter.GetCurEvent(&msgp)) {  // chase and delete all subsequent bender messages,
            if (msgp->IsPitchBend()) {      // until bender == 0
                short val = msgp->GetBenderValue(); // remember bender value
                Delete(*msgp);
                if (val == 0) break;
            }
        }
        Insert(msg);                        // insert a new pitch bend = 0 at time t
    }
}

*********/




}

