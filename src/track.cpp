/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2010 V.R.Madgazin
 *   www.vmgames.com vrm@vmgames.com
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


#include "../include/track.h"
#include "../include/matrix.h"


////////////////////////////////////////////////////////////////////////////
//                                class MIDITrack                         //
////////////////////////////////////////////////////////////////////////////


tInsMode MIDITrack::ins_mode = INSMODE_INSERT_OR_REPLACE;


MIDITrack::MIDITrack(MIDIClockTime end_time) : status(INIT_STATUS), rec_chan(-1),
    time_shift(0), in_port(0), out_port() {
// a track always contains at least the MIDI_END event, so num_events > 0
    MIDITimedMessage msg;
    msg.SetDataEnd();
    msg.SetTime(end_time);
    events.push_back(msg);
}


MIDITrack::MIDITrack(const MIDITrack &trk) : events(trk.events), status(trk.status), rec_chan(trk.rec_chan),
                     time_shift(trk.time_shift), in_port(trk.in_port), out_port(trk.out_port)
{}


MIDITrack& MIDITrack::operator=(const MIDITrack &trk) {
    events = trk.events;
    rec_chan = trk.rec_chan;
    status = trk.status;
    time_shift = trk.time_shift;
    in_port = trk.in_port;
    out_port = trk.out_port;
    return *this;
}


void MIDITrack::Reset() {
    Clear();
    rec_chan = 0;
    time_shift = 0;
    in_port = 0;
    out_port = 0;
}

void MIDITrack::Clear(bool mantain_end) {
    MIDIClockTime end = mantain_end ? GetEndTime() : 0;
    events.resize(0);       // destroys messages clearing sysex
    MIDITimedMessage msg;
    msg.SetDataEnd();
    msg.SetTime(end);
    events.push_back(msg);
    status = INIT_STATUS;
}


int MIDITrack::GetChannel() {
    if (status & STATUS_DIRTY)
        Analyze();
    signed char chan = status & 0xff;
    return (int)chan;
}


int MIDITrack::GetStatus() {
    if (status & STATUS_DIRTY)
        Analyze();
    return status;
}


unsigned char MIDITrack::GetType() {
    if (IsEmpty())
        return TYPE_EMPTY;
    if (status & STATUS_DIRTY)
        Analyze();
    if ((status & HAS_MAIN_META) &&
        !((status & HAS_ONE_CHAN) || (status & HAS_MANY_CHAN)))
        return TYPE_MAIN;
    if ((status & HAS_TEXT_META) &&
        !((status & HAS_MAIN_META) || (status & HAS_ONE_CHAN) || (status & HAS_MANY_CHAN) || (status & HAS_RESET_SYSEX)))
        return TYPE_TEXT;
    if (status & HAS_ONE_CHAN) {
        if(!((status & HAS_MAIN_META) || (status & HAS_RESET_SYSEX)))
            return TYPE_CHAN;
        else
            return TYPE_IRREG_CHAN;
    }
    if (status & HAS_MANY_CHAN)
        return TYPE_MIXED_CHAN;
    return TYPE_UNKNOWN;
}


unsigned char MIDITrack::HasSysex() {
    if (status & STATUS_DIRTY)
        Analyze();
    if ((status & HAS_SYSEX) && (status & HAS_RESET_SYSEX))
        return TYPE_BOTH_SYSEX;
    if ((status & HAS_SYSEX) && !(status & HAS_RESET_SYSEX))
        return TYPE_SYSEX;
    if (!(status & HAS_SYSEX) && (status & HAS_RESET_SYSEX))
        return TYPE_RESET_SYSEX;
    return 0;
}

bool MIDITrack::SetEndTime(MIDIClockTime end_time) {
    if (end_time < GetEndTime() && events.size() > 1)
        if (events[events.size() - 2].GetTime() > end_time) return false;
            // we tried to insert an MIDI_END before last (musical) event
    events.back().SetTime(end_time);
    return true;
}


bool MIDITrack::SetChannel(int chan) {
    if (chan < 0 || chan> 15)
        return false;
    for (unsigned int i = 0; i < events.size(); i++) {
        if (GetEvent(i).IsChannelMsg())
            GetEvent(i).SetChannel(chan);
    }
    status |= STATUS_DIRTY;
    return true;
}


bool MIDITrack::SetRecChannel(int chan) {
    if (chan < -1 || chan > 15)
        return false;
    rec_chan = chan;
    return true;
}


bool MIDITrack::SetInPort(unsigned int port) {
    if (!MIDIManager::IsValidInPortNumber(port))
        return false;
    in_port = port;
    return true;
}


bool MIDITrack::SetOutPort(unsigned int port) {
    if (!MIDIManager::IsValidOutPortNumber(port))
        return false;
    out_port = port;
    return true;
}


void MIDITrack::ShrinkEndTime() {
    if (events.size() == 1)         // empty track
        events.back().SetTime(0);   // set end time to 0
    else
        events.back().SetTime(events[events.size() - 2].GetTime());
            // set end time to the last (musical) event
}


MIDIClockTime MIDITrack::GetNoteLength(const MIDITimedMessage& msg) const {
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
    return TIME_INFINITE;
}


void MIDITrack::SetInsertMode(tInsMode mode) {
    if (mode != INSMODE_DEFAULT)
        ins_mode = mode;
}


bool MIDITrack::InsertEvent(const MIDITimedMessage& msg, tInsMode mode) {
    if (msg.IsDataEnd()) return false;                  // DATA_END only auto managed

    if (GetEndTime() < msg.GetTime()) {                 // insert as last event
        SetEndTime(msg.GetTime());                      // adjust DATA_END
        events.insert(events.end() - 1, msg);           // insert just before DATA_END
        status |= STATUS_DIRTY;
        return true;
    }

    int ev_num;
        // find the first event in the track with time <= msg time
    FindEventNumber(msg.GetTime(), &ev_num);            // must return true
    int old_ev_num = ev_num;

    if (mode == INSMODE_DEFAULT)
        mode = ins_mode;                                // set inert mode to default behaviour

    switch (mode) {
        case INSMODE_DEFAULT:                           // dummy, for avoiding a warning
        case INSMODE_INSERT:                            // always insert the event
                // find the right place among events with same time
            while (CompareEventsForInsert(msg, events[ev_num]) == 1)
                ev_num++;
            events.insert(events.begin() + ev_num, msg);
            status |= STATUS_DIRTY;
            return true;

        case INSMODE_REPLACE:                           // replace a same kind event, or do nothing
                // find a same kind event at same time
            while (IsValidEventNum(ev_num) && events[ev_num].GetTime() == msg.GetTime()) {
                if (IsSameKind(events[ev_num], msg)) {
                    events[ev_num] = msg;               // replace if found
                    status |= STATUS_DIRTY;
                    return true;
                }
                ev_num++;
            }
            return false;                               // return false if not found

        case INSMODE_INSERT_OR_REPLACE:                 // replace a same kind, or insert
        case INSMODE_INSERT_OR_REPLACE_BUT_NOTE:        // (always insert notes)
                // first search for a same kind event, as above
            while (IsValidEventNum(ev_num) && events[ev_num].GetTime() == msg.GetTime()) {
                if (IsSameKind(events[ev_num], msg) &&
                     (mode == INSMODE_INSERT_OR_REPLACE || !msg.IsNote())) {
                    events[ev_num] = msg;               // replace if found
                    status |= STATUS_DIRTY;
                    return true;
                }
                ev_num++;
            }
                // if not found a same kind insert the event
            ev_num = old_ev_num;
            while (CompareEventsForInsert(msg, events[ev_num]) == 1)
                ev_num++;
            events.insert(events.begin() + ev_num, msg);
            status |= STATUS_DIRTY;
            return true;                                // insert
    }

    return false;                                       // this should not happen
}


bool MIDITrack::InsertNote(const MIDITimedMessage& msg, MIDIClockTime len, tInsMode mode) {
    if (!msg.IsNoteOn()) return false;

    MIDITimedMessage msgoff(msg);                       // set our NOTE_OFF message
    msgoff.SetType(NOTE_OFF);
    msgoff.SetTime(msg.GetTime() + len);

    int ev_num;

    if (mode == INSMODE_DEFAULT)
        mode = ins_mode;                                // set insert mode to default behavior

    switch (mode) {
        case INSMODE_DEFAULT:                           // dummy, for avoiding a warning
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


bool MIDITrack::DeleteEvent( const MIDITimedMessage& msg ) {
    if (msg.IsDataEnd())
        return false;
    int ev_num;
    if (!FindEventNumber(msg, &ev_num))
        return false;
    events.erase(events.begin() + ev_num);
    status |= STATUS_DIRTY;
    return true;
}


bool MIDITrack::DeleteNote( const MIDITimedMessage& msg ) {
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

// NEW
void MIDITrack::PushEvent(const MIDITimedMessage& msg) {
    if (msg.IsDataEnd()) return;                        // DATA_END only auto managed

    if (GetEndTime() < msg.GetTime())
        SetEndTime(msg.GetTime());                      // adjust DATA_END
    events.insert(events.end() - 1, msg);               // insert just before DATA_END
    status |= STATUS_DIRTY;
}


void MIDITrack::InsertInterval(MIDIClockTime start, MIDIClockTime length, const MIDITrack* src) {
    if (length == 0) return;

    CloseOpenEvents(0, start);                          // truncate notes, pedal, bender at start
    int start_n;
    if (FindEventNumber(start, &start_n)) {             // there are events after start time
        for (unsigned int i = start_n; i < events.size(); i++)
            events[i].AddTime(length);                  // moves these events
    }
    if(!src) return;                                    // we want only move events

    for(unsigned int i = 0; i < src->GetNumEvents(); i++) {
                                                        // else inserts events in src
        MIDITimedMessage msg = src->events[i];
        if (msg.GetTime() >= length) break;             // edit is too long
        msg.AddTime(start);                             // TODO: don't insert time, key and tempo messages if
                                                        // already present
        InsertEvent(msg);
    }
    CloseOpenEvents(start, start + length);             // truncate at end
}


void MIDITrack::MakeInterval(MIDIClockTime start, MIDIClockTime end, MIDITrack* interval) const {
    interval->Clear();
    if (end > GetEndTime())                         // adjust end time
        end = GetEndTime();
    if (end <= start) return;                       // nothing to do
    interval->SetEndTime(end - start);

    MIDITrack edittrack(*this);                     // copy original track to make edits on it;
    edittrack.CloseOpenEvents(0, start);            // truncate open events BEFORE start (so delete
                                                    // note off, etc in edit track)
    edittrack.CloseOpenEvents(start, end);          // truncate at end

    int ev_num;                                     // now copy edittrack events to interval
    edittrack.FindEventNumber(start, &ev_num);      // an event surely exists
    MIDITimedMessage msg = edittrack.events[ev_num];
    for (; msg.GetTime() <= end && !msg.IsDataEnd(); ev_num++, msg = edittrack.events[ev_num]) {
        if (msg.GetTime() == start &&
            (msg.IsNoteOff() || msg.IsPedalOff() ||
            (msg.IsPitchBend() && msg.GetBenderValue() == 0)))
            continue;                               // skip these events at the beginning of the interval
       if (msg.GetTime() == end &&
            (msg.IsNoteOn() || msg.IsPedalOn() ||
            (events[ev_num].IsPitchBend() && events[ev_num].GetBenderValue() != 0)))
            continue;                               // skip these events at the end of the interval

        msg.SubTime(start);                         // adjust message time
        interval->InsertEvent(msg);                 // insert it in interval
    }
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

    CloseOpenEvents(0, start);                      // truncate open events BEFORE start (so delete
                                                    // note off, etc in edit track)
    CloseOpenEvents(start, end);                    // truncate at end
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


void MIDITrack::ReplaceInterval(MIDIClockTime start, MIDIClockTime end, const MIDITrack* src) {
    if (end <= start || src == 0) return;

    ClearInterval(start, end);                      // deletes all events in the interval
    for(unsigned int i = 0; i < src->GetNumEvents(); i++) {
        MIDITimedMessage msg(src->GetEvent(i));     // inserts events
        if (msg.GetTime() >= end - start) break;    // edit is too long
        msg.AddTime(start);
        InsertEvent(msg);
    }
    CloseOpenEvents(start, end);                    // truncate at end
}


/* This is the old function, more complicated, but working even on tracks with mixed
   MIDI channels
*/
void MIDITrack::CloseOpenEvents(MIDIClockTime from, MIDIClockTime to) {
    if (to == 0 || to >= GetEndTime()) return;      // there aren't open events at beginning or end

    MIDIMatrix matrix;
    int pitchbend[16];
    MIDITimedMessage msg;
    unsigned int ev_num = 0;
    for (int i = 0; i < 16; i++)
        pitchbend[i] = 0;
    // ignore events before from
    while (ev_num < GetNumEvents() - 1 && GetEvent(ev_num + 1).GetTime() < from)
        ev_num++;
    // scan events before current time, remembering notes, pedal and pitch bend
    while (ev_num < GetNumEvents() && (msg = GetEvent(ev_num)).GetTime() < to) {
        matrix.Process(&msg);
        if (msg.IsPitchBend())
            pitchbend[msg.GetChannel()] = msg.GetBenderValue();
        ev_num++;
    }
    // now scan events at current time, closing open notes, pedal and pitch bend
    while (ev_num < GetNumEvents() && (msg = GetEvent(ev_num)).GetTime() == to) {
        if (msg.IsNoteOff() || msg.IsPedalOff())
            matrix.Process (&msg);
        if (msg.IsPitchBend() && msg.GetBenderValue() == 0)
            pitchbend[msg.GetChannel()] = msg.GetBenderValue();
        ev_num++;
    }                                           // ev_num is now the index of 1st event with timw > t

    for (int ch = 0; ch < 16; ch++) {           // for every channel ...
        // get the number of notes on of the channel
        int note_count = matrix.GetChannelCount(ch);
        // search which notes are on
        for (int note = matrix.GetMinNoteOn(ch); note_count > 0 && note <= matrix.GetMaxNoteOn(ch); note++) {
            // if the note is on ... (i should normally be 1)
            for (int i = matrix.GetNoteCount(ch, note); i > 0; i--) {
                msg.SetNoteOff(ch, note, 0);
                msg.SetTime(to);
                InsertEvent(msg);               // ... insert a note off at time to
                ev_num++;                       // ... adjust ev_num ...
                note_count--;                   // ... and decrease note_count
                // now search the corresponding note off after t ...
                for (unsigned int j = ev_num; j < GetNumEvents(); j++) {
                    msg = GetEvent(j);
                    if (msg.IsNoteOff() && msg.GetChannel() == ch && msg.GetNote() == note ) {
                        DeleteEvent( msg );     // ... and delete it
                        break;
                    }
                }

            }
        }

        if (matrix.GetHoldPedal(ch)) {
            msg.SetControlChange(ch, C_DAMPER, 0);
            msg.SetTime(to);
            InsertEvent(msg);
            ev_num++;
            for (unsigned int j = ev_num; j < GetNumEvents(); j++) {
                // search the corresponding pedal off after to ...
                msg = GetEvent(j);
                if (msg.IsPedalOff() && msg.GetChannel() == ch) {
                    DeleteEvent(msg);           // ... and delete it
                    break;
                }
            }
        }

        if (pitchbend[ch]) {
            msg.SetPitchBend(ch, 0);
            msg.SetTime(to);
            InsertEvent(msg);
            ev_num++;
            for (unsigned int j = ev_num; j < GetNumEvents(); j++) {
                // search other pitch bend messages after to until we find a 0 value ...
                msg = GetEvent(j);
                if (msg.IsPitchBend() && msg.GetChannel() == ch) {
                    if (msg.GetBenderValue() != 0)
                        DeleteEvent(msg);       // ... and delete it
                    else
                        break;
                }
            }
        }
    }
}


bool MIDITrack::FindEventNumber(const MIDITimedMessage& msg, int* event_num, int mode) const {
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
            if (IsSameKind(GetEvent(min), msg)) {   // element found
                *event_num = min;           // TODO: Eliminate scope selectors for IsSameKind, CompareEventsForInsert
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


void MIDITrack::Analyze() {
    signed char channel = -1;
    const MIDITimedMessage* msg;
    for (unsigned int i = 0; i < GetNumEvents(); i++) {
        msg = GetEventAddress(i);
        if (msg->IsMetaEvent() && !msg->IsDataEnd()) {
            if (msg->IsTextEvent())
                status |= HAS_TEXT_META;
            else
                status |= HAS_MAIN_META;
        }
        else if (msg->IsChannelMsg()) {
            if (channel == -1)
                channel = msg->GetChannel();
            else if (channel != msg->GetChannel())
                status |= HAS_MANY_CHAN;
        }
        else if (msg->IsSysEx()) {
            if (msg->GetSysEx()->IsGMReset() || msg->GetSysEx()->IsGSReset() || msg->GetSysEx()->IsXGReset())
                status |= HAS_RESET_SYSEX;
            else
                status |= HAS_SYSEX;
        }
    }
    if (channel != -1 && !(status & HAS_MANY_CHAN)) {
        status |= HAS_ONE_CHAN;
        status &= 0xffffff00;
        status |= channel;
    }
    status &= (~STATUS_DIRTY);
}



////////////////////////////////////////////////////////////////////////////
//                       class MIDITrackIterator                          //
////////////////////////////////////////////////////////////////////////////


// this is interely added by me!!!! NC

MIDITrackIterator::MIDITrackIterator(MIDITrack* trk) : track(trk) {
    Reset();
}


void MIDITrackIterator::Reset() {
    // go to time zero
    cur_time = 0;
    cur_ev_num = 0;

    // reset midi status
    program = -1;
    for (int i = 0; i < 128; i++) {
        controls[i] = -1;
        notes_on[i] = 0;
    }
    bender_value = 0;
    num_notes_on = 0;
    ScanEventsAtThisTime();         // process all messages at this time
}


void MIDITrackIterator::SetTrack(MIDITrack* trk) {
        track = trk;
        Reset();
}


bool MIDITrackIterator::FindNoteOff(unsigned char note, MIDITimedMessage** msg) {
    for (unsigned int i = cur_ev_num; i < track->GetNumEvents(); i ++) {
        *msg = track->GetEventAddress(i);
        if ((*msg)->IsNoteOff() && (*msg)->GetNote() == note)
            return true;                //  msg is the correct note off
    }
    return false;
}


bool MIDITrackIterator::FindPedalOff(MIDITimedMessage** msg) {
    for (unsigned int i = cur_ev_num; i < track->GetNumEvents(); i ++) {
        *msg = track->GetEventAddress(i);
        if ((*msg)->IsControlChange() && (*msg)->GetController() == C_DAMPER  &&
            (*msg)->GetControllerValue() < 64)
            return true;                //  msg is the correct pedal off
    }
    return false;
}


bool MIDITrackIterator::GoToTime(MIDIClockTime time) {
    MIDIClockTime t;
    MIDITimedMessage* msg;

    if (time > track->GetEndTime()) return false;
    if (time <= cur_time)
        Reset();
    while (GetNextEventTime(&t) && t < time)
        GetNextEvent(&msg);
    cur_time = time;
    ScanEventsAtThisTime();
    return true;
}


bool MIDITrackIterator::GetNextEvent(MIDITimedMessage** msg) {
    if (!track->IsValidEventNum(cur_ev_num))
        return false;
    *msg = track->GetEventAddress(cur_ev_num);
    MIDIClockTime old_time = cur_time;
    cur_time = (*msg)->GetTime();
    if (cur_time > old_time)
        ScanEventsAtThisTime();                             // update status processing all messages at this time
    cur_ev_num++;                                           // increment message pointer
    return true;
}


bool MIDITrackIterator::GetNextEventTime(MIDIClockTime* t) const {
    if (!track->IsValidEventNum(cur_ev_num))
        return false;  // we are at the end of track
    *t = track->GetEventAddress(cur_ev_num)->GetTime();
    return true;
}


bool MIDITrackIterator::EventIsNow(const MIDITimedMessage& msg) {
    MIDITimedMessage msg1;
    int ev_num;

    track->FindEventNumber(cur_time, &ev_num);
    while(track->IsValidEventNum(ev_num) && (msg1 = track->GetEvent(ev_num)).GetTime() == cur_time) {
        if (IsSameKind(msg1, msg)) return true;
        ev_num++;
    }
    return false;
}


/* This was eliminated
bool MIDITrackIterator::GoToNextEvent() {
    if (!track->IsValidEventNum(cur_ev_num)) return false;
    cur_ev_num++;
    MIDITimedMessage* msg = track->GetEventAddress(cur_ev_num);
    if (msg->GetTime() > cur_time) {
        cur_time = msg->GetTime();
        ScanEventsAtThisTime();                             // process all messages at this time
    }
    return true;
}
*/


bool MIDITrackIterator::Process(const MIDITimedMessage *msg) {

    // is it a normal MIDI channel message?
    if(msg->IsChannelMsg()) {
        if (msg->IsNoteOff() && notes_on[msg->GetNote()]) {
            notes_on[msg->GetNote()]--;
            num_notes_on--;
        }
        else if (msg->IsNoteOn() && !notes_on[msg->GetNote()]) {
            notes_on[msg->GetNote()]++;
            num_notes_on++;
        }
        else if (msg->IsPitchBend())
            bender_value = msg->GetBenderValue();
        else if (msg->IsControlChange())
            controls[msg->GetController()] = msg->GetControllerValue();
        else if (msg->IsProgramChange())
            program = msg->GetProgramValue();
        return true;
    }
    return false;
}


void MIDITrackIterator::ScanEventsAtThisTime() {
// process all messages up to and including this time only

    unsigned int ev_num = cur_ev_num;
        // move to first event at this time
    while (ev_num > 0 && track->GetEventAddress(ev_num - 1)->GetTime() == cur_time)
        ev_num--;
        // now examine all events at this time upgrading the iterator status
    while (ev_num < track->GetNumEvents() && track->GetEventAddress(ev_num)->GetTime() == cur_time) {
        MIDITimedMessage* msg = track->GetEventAddress(ev_num);

        // is msg a normal MIDI channel message?
        if(msg->IsChannelMsg()) {
            if (msg->IsNoteOff() && notes_on[msg->GetNote()]) {
                notes_on[msg->GetNote()]--;
                num_notes_on--;
            }
            else if (msg->IsNoteOn() && !notes_on[msg->GetNote()]) {
                notes_on[msg->GetNote()]++;
                num_notes_on++;
            }
            else if (msg->IsPitchBend())
                bender_value = msg->GetBenderValue();
            else if (msg->IsControlChange())
                controls[msg->GetController()] = msg->GetControllerValue();
            else if (msg->IsProgramChange())
                program = msg->GetProgramValue();
        }
        ev_num++;
    }
}

