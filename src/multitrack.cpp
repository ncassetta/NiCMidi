/*
 *  libjdkmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  *** RELEASED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) April 27, 2004 ***
 *
 *  MODIFIED BY NICOLA CASSETTA
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
**	Copyright 1986 to 1998 By J.D. Koftinoff Software, Ltd.
**
**	All rights reserved.
**
**	No one may duplicate this source code in any form for any reason
**	without the written permission given by J.D. Koftinoff Software, Ltd.
**
*/



#include "../include/multitrack.h"

#include "../include/dump_tracks.h"    // DEBUG:
#include <iostream>


/////////////////////////////////////////
//      class MIDIMultiTrack           //
/////////////////////////////////////////


MIDIMultiTrack::MIDIMultiTrack(unsigned int num_tracks, unsigned int cl_p_b) :
          clks_per_beat(cl_p_b) {
    tracks.resize(num_tracks);
    for (unsigned int i = 0; i < num_tracks; i++)
        tracks[i] = new MIDITrack;
}


MIDIMultiTrack::~MIDIMultiTrack() {
    for(unsigned int i = 0; i < tracks.size(); ++i)
        delete tracks[i];
}


void MIDIMultiTrack::SetClksPerBeat(unsigned int cl_p_b) {
    MIDIClockTime ev_time;
        // calculate the ratio between new and old value
    float ratio = cl_p_b / clks_per_beat;
        // substitute new value
    clks_per_beat = cl_p_b;
        // update the times of all events, multiplying them by the ratio
    for (unsigned int i = 0; i < tracks.size(); i++) {
        for (unsigned int ev_num = 0; ev_num < tracks[i]->GetNumEvents(); ev_num++) {
            ev_time = tracks[i]->GetEvent(ev_num).GetTime();
            ev_time = (MIDIClockTime)(ev_time * ratio + 0.5);
            tracks[i]->GetEvent(ev_num).SetTime(ev_time);
        }
    }
}


unsigned int MIDIMultiTrack::GetNumTracksWithEvents() const {
    unsigned int num_tracks = 0;
    for (unsigned int i = 0; i < tracks.size(); i++ )
            if (!tracks[i]->IsTrackEmpty())
                num_tracks++;
    return num_tracks;
}


unsigned int MIDIMultiTrack::GetNumEvents() const {
    unsigned int num_events = 0;
    for (unsigned int i = 0; i < tracks.size(); i++ )
        num_events += tracks[i]->GetNumEvents();
    return num_events;
}


void MIDIMultiTrack::Clear() {
    for(unsigned int i = 0; i < tracks.size(); i++)
        delete tracks[i];
    tracks.resize(0);
}


void MIDIMultiTrack::ClearAndResize(unsigned int n) {
    Clear();
    for (unsigned int i = 0; i < n; i++)
        InsertTrack();
}

void MIDIMultiTrack::ClearTracks(bool mantain_end) {
    for(unsigned int i = 0; i < tracks.size(); i++)
        tracks[i]->Clear(mantain_end);
}


// TODO: Revise this
bool MIDIMultiTrack::AssignEventsToTracks ( const MIDITrack *src )
{
    MIDITrack tmp( *src ); // make copy of src track

    std::cout << "Multitrack before AssignEventsToTracks\n";
    DumpMIDIMultiTrack(this);

    // renew multitrack object with 17 tracks:
    // tracks 1-16 for channel events, and track 0 for other types of events
    ClearAndResize( 17 );

    // move events to tracks 0-16 according it's types/channels
    for (unsigned int i = 0; i < tmp.GetNumEvents(); ++i)
    {
        const MIDITimedBigMessage *msg;
        msg = tmp.GetEventAddress ( i );

        int track_num = 0;
        if ( msg->IsChannelMsg() )
            track_num = 1 + msg->GetChannel();

        if ( !GetTrack ( track_num )->InsertEvent(*msg ) )
            return false;
    }

    std::cout << "Multitrack before AssignEventsToTracks\n";
    DumpMIDIMultiTrack(this);


    return true;
}


bool MIDIMultiTrack::InsertTrack(int trk) {
    if (trk == -1) trk = tracks.size();                 // if trk = -1 (default) append track
    if (trk < 0 || (unsigned)trk > tracks.size()) return false;
    tracks.insert(tracks.begin() + trk, new MIDITrack);
    return true;
}


bool MIDIMultiTrack::DeleteTrack(int trk) {
    if (!IsValidTrackNumber(trk)) return false;
    delete tracks[trk];
    tracks.erase(tracks.begin() + trk);
    return true;
}


bool MIDIMultiTrack::MoveTrack(int from, int to) {
    if (from == to) return true;                    // nothing to do
    if (!IsValidTrackNumber(from) || !IsValidTrackNumber(to)) return false;
    MIDITrack* temp = tracks[from];
    tracks.erase(tracks.begin() + from);
    if (from < to)
        to--;
    tracks.insert(tracks.begin() + to, temp);
    return true;
}


bool MIDIMultiTrack::InsertEvent(int trk, const MIDITimedBigMessage& msg, int _ins_mode) {
    if (IsValidTrackNumber(trk))
        return tracks[trk]->InsertEvent(msg, _ins_mode);
    return false;
}


bool MIDIMultiTrack::InsertNote(int trk, const MIDITimedBigMessage& msg, MIDIClockTime len, int _ins_mode) {
    if (IsValidTrackNumber(trk))
        return tracks[trk]->InsertNote(msg, len, _ins_mode);
    return false;
}


bool MIDIMultiTrack::DeleteEvent(int trk, const MIDITimedBigMessage& msg) {
    if (IsValidTrackNumber(trk))
        return tracks[trk]->DeleteEvent(msg);
    return false;
}


bool MIDIMultiTrack::DeleteNote(int trk, const MIDITimedBigMessage& msg) {
    if (IsValidTrackNumber(trk))
        return tracks[trk]->DeleteNote(msg);
    return false;
}


//TODO: these must be revised
void MIDIMultiTrack::EditCopy(MIDIClockTime start, MIDIClockTime end,
                                int tr_start, int tr_end, MIDIEditMultiTrack* edit) {
    if (tr_start < 0 || (unsigned int)tr_end >= GetNumTracks()) return;

    edit->Clear();
    for (int i = 0; i <= tr_end; i++)
        edit->InsertTrack();
    edit->SetStartTrack(tr_start);
    edit->SetEndTrack(tr_end);

    edit->tracks[0]->SetEndTime(end - start);
    for (int i = tr_start; i <= tr_end; i++)
        tracks[i]->MakeInterval(start, end, edit->tracks[i]);

    DumpMIDIMultiTrack(this);
    DumpMIDIMultiTrack(edit);
}


void MIDIMultiTrack::EditCut(MIDIClockTime start, MIDIClockTime end, MIDIEditMultiTrack* edit) {
    DumpMIDIMultiTrack(this);

    EditCopy(start, end, 0, GetNumTracks()-1, edit);

    for (unsigned int i = 0; i < tracks.size(); i++)
        GetTrack(i)->DeleteInterval(start, end);

    DumpMIDIMultiTrack(this);
}


void MIDIMultiTrack::EditClear(MIDIClockTime start, MIDIClockTime end, int tr_start, int tr_end) {
    DumpMIDIMultiTrack(this);

    for (int i = tr_start; i <= tr_end; i++)
        GetTrack(i)->ClearInterval(start, end);

    DumpMIDIMultiTrack(this);
}


void MIDIMultiTrack::EditInsert(MIDIClockTime start, int tr_start, int times, bool sysex,
                                MIDIEditMultiTrack* edit) {

    MIDIClockTime length = edit->tracks[0]->GetEndTime();
    int tr_end = tr_start + edit->GetEndTrack() - edit->GetStartTrack();

    for (unsigned int i = 0; i < tracks.size(); i++)
        tracks[i]->InsertInterval(start, length * times, 0);    // inserts a blank interval

    DumpMIDIMultiTrack(this);
    DumpMIDIMultiTrack(edit);

    if(edit) {
        if (tr_start == 0)
            tr_start = 1;
            // skip track 0: it will be set from the corresponding INTTrack with a Recompose
        for (int i = tr_start; i <= tr_end; i++) {
            for(int j = 0; j < times; j++)
                tracks[i]->ReplaceInterval(start + j * length, length, sysex, edit->tracks[i]);
        }
    }

    DumpMIDIMultiTrack(this);
    DumpMIDIMultiTrack(edit);
}


void MIDIMultiTrack::EditReplace(MIDIClockTime start, int tr_start, int times, bool sysex,
                                 MIDIEditMultiTrack* edit) {
    MIDIClockTime length = edit->tracks[0]->GetEndTime() * times;
    int tr_end = tr_start + edit->GetEndTrack() - edit->GetStartTrack();

    DumpMIDIMultiTrack(this);
    DumpMIDIMultiTrack(edit);

    for (int i = tr_start; i <= tr_end; i++)
        tracks[i]->ClearInterval(start, length);        // deletes previous events

    if (tr_start == 0)
        tr_start = 1;
        // skip track 0: it will be set from the corresponding INTTrack with a Recompose
    if (start + times * edit->tracks[0]->GetEndTime() > tracks[0]->GetEndTime())
        tracks[0]->SetEndTime(start + times * edit->tracks[0]->GetEndTime());
    for (int i = tr_start; i <= tr_end; i++) {
        for(int j = 0; j < times; j++)
            tracks[i]->ReplaceInterval(start + j * edit->tracks[0]->GetEndTime(),
                                       edit->tracks[0]->GetEndTime(), sysex, edit->tracks[i]);
    }

    DumpMIDIMultiTrack(this);
}



//////////////////////////////////////////////////////
//      class MIDIMultiTrackIteratorState           //
//////////////////////////////////////////////////////



MIDIMultiTrackIteratorState::MIDIMultiTrackIteratorState(int n_tracks) :
        num_tracks(n_tracks) {
    next_event_number = new int [n_tracks];
    next_event_time = new MIDIClockTime [n_tracks];
    Reset();
}


MIDIMultiTrackIteratorState::MIDIMultiTrackIteratorState(const MIDIMultiTrackIteratorState &m) :
        num_tracks(m.num_tracks), cur_time(m.cur_time),
        cur_event_track(m.cur_event_track) {

    next_event_number = new int [m.num_tracks];
    next_event_time = new MIDIClockTime [m.num_tracks];
    for(int i = 0; i < num_tracks; i++) {
        next_event_number[i] = m.next_event_number[i];
        next_event_time[i] = m.next_event_time[i];
    }
}


MIDIMultiTrackIteratorState::~MIDIMultiTrackIteratorState() {
    delete[] next_event_number;
    delete[] next_event_time;
}


const MIDIMultiTrackIteratorState& MIDIMultiTrackIteratorState::operator= (const MIDIMultiTrackIteratorState &m) {
    SetNumTracks(m.num_tracks);
    cur_time = m.cur_time;
    cur_event_track = m.cur_event_track;
    for(int i = 0; i < num_tracks; ++i) {
        next_event_number[i] = m.next_event_number[i];
        next_event_time[i] = m.next_event_time[i];
    }
    return *this;
}


void MIDIMultiTrackIteratorState::SetNumTracks(int n) {
    if (n != num_tracks) {
        num_tracks = n;
        delete[] next_event_number;
        delete[] next_event_time;
        next_event_number = new int [n];
        next_event_time = new MIDIClockTime [n];
    }
    Reset();
}


void MIDIMultiTrackIteratorState::Reset() {
    cur_time = 0;
    cur_event_track = 0;
    for(int i = 0; i < num_tracks; ++i) {
      next_event_number[i] = 0;
      next_event_time[i] = TIME_INFINITE;
    }
}


int MIDIMultiTrackIteratorState::FindTrackOfFirstEvent() {
    MIDIClockTime minimum_time = TIME_INFINITE;
    int minimum_time_track = -1;

    // go through all tracks and find the track with the smallest event time.
    for(int j = 0; j < num_tracks; ++j) {
        int i = (j + cur_event_track + 1) % num_tracks;

    // skip any tracks that have a current event number less than 0 - these are finished already
        if(next_event_number[i] >= 0 && next_event_time[i] < minimum_time) {
            minimum_time = next_event_time[i];
            minimum_time_track = i;
        }
    }

    // set cur_event_track to -1 if there are no more events left
    cur_event_track = minimum_time_track;
    cur_time = minimum_time;
    return cur_event_track;
}



/////////////////////////////////////////////////
//      class MIDIMultiTrackIterator           //
/////////////////////////////////////////////////



MIDIMultiTrackIterator::MIDIMultiTrackIterator(MIDIMultiTrack *mlt) :
    multitrack(mlt), state(mlt->GetNumTracks()) {
    Reset();
}


MIDIMultiTrackIterator::~MIDIMultiTrackIterator() {}


void MIDIMultiTrackIterator::Reset() {
    state.SetNumTracks(multitrack->GetNumTracks());
    GoToTime(0);
}


void MIDIMultiTrackIterator::GoToTime(MIDIClockTime time) {
    // start at time 0
    state.Reset();

    // transfer info from the first events in each track in the
    // multitrack object to our current state.
    for(unsigned int i = 0; i < multitrack->GetNumTracks(); ++i) {
        MIDITrack *track = multitrack->GetTrack(i);

    // extract the time of the first event of the track (already one exists)
        MIDITimedBigMessage *msg = track->GetEventAddress(0);

    // Keep track of the event number and the event time.
        state.next_event_number[i] = 0;
        state.next_event_time[i]=msg->GetTime();
    }

    // are there any events at all? find the track with the earliest event
    if(state.FindTrackOfFirstEvent() != -1) {
    // yes iterate through all the events until we find a time >= the requested time
        while(state.GetCurTime() < time) {

    // did not get to the requested time yet.
    // go to the next chronological event on all tracks
            if(!GoToNextEvent()) break;
    // there is no more events to go to

        }
    }
}



bool MIDIMultiTrackIterator::GetCurEventTime(MIDIClockTime *t) const {
    // if there is a next event, then set *t to the time of the event and return true
    if(state.GetCurEventTrack() != -1) {
        *t = state.GetCurTime();
        return true;
    }
    else
      return false;
}


bool MIDIMultiTrackIterator::GetCurEvent(int *track, MIDITimedBigMessage **msg) {
    int t = state.GetCurEventTrack();

    if(t != -1) {
        if(track)
            *track=t;
        if(msg) {
            int num = state.next_event_number[t];
            if(num >= 0)
                *msg = multitrack->GetTrack(t)->GetEventAddress(state.next_event_number[t]);
            else
                *msg = 0;

        // do we really have a message?
            if(! *msg)
        // no, return false then
                return false;
        }
        return true;
    }
    else return false;

}


bool MIDIMultiTrackIterator::GoToNextEvent() {
    // find the next event in the multitrack and return it
    // if there is no event left, return false

    if(state.cur_event_track == -1)
    // no tracks left - all tracks are at end
        return false;

    // update the current event for the current track to the
    // next event on the same track.

    GoToNextEventOnTrack(state.cur_event_track);

    // now find out which track now has the earliest event

    if(state.FindTrackOfFirstEvent() == -1)
    // No tracks do. all tracks are at the end. return false.
        return false;

    // ok, now state.cur_event_track has a valid track # of the next event

    return true;
}


bool MIDIMultiTrackIterator::GoToNextEventOnTrack(int track_num) {
    // Get the track that we are dealing with
    MIDITrack *track = multitrack->GetTrack(track_num);

    // Get ptr to the current event number for this track
    int *event_num = &state.next_event_number[track_num];

    // skip this track if this event number is <0 - This track has hit end already.
    if(*event_num < 0)
        return false; // at end of track

    // increment *event_num to next event on track
    (*event_num) += 1;

    // are we at end of track?
    if(*event_num >= track->GetNumEvents()) {
        // yes, set *event_num to -1
        *event_num=-1;
        return false; // at end of track
    }
    else {
    // not at end of track yet - get the time of the event
        MIDITimedBigMessage *msg;
        msg = track->GetEventAddress( *event_num );
        state.next_event_time[track_num] = msg->GetTime();
    }

    return true;
}






// Don't implement Cut, Edit, Paste... (see header)
void MIDIEditMultiTrack::CopyAll(MIDIMultiTrack* m) {
    Clear();
    for (unsigned int i = 0; i < m->GetNumTracks(); i++) {
        InsertTrack(i);
        *GetTrack(i) = *(m->GetTrack(i));
    }
}





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

/***********************************OLD FILE From NC




bool MIDIMultiTrack::InsertTrack(int trk) {
    if (trk == -1) trk = num_tracks;                // if trk = -1 (default) append track
    if (num_tracks == max_num_tracks || trk > num_tracks) return false;

            // move pointers to tracks...
    memmove((void *)(tracks + trk + 1), (void *)(tracks + trk), (num_tracks - trk) * sizeof(MIDITrack *));
            // and allocate a new track
    tracks[trk] = new MIDITrack;
    num_tracks++;
    return true;
}


bool MIDIMultiTrack::DeleteTrack(int trk) {
    if (trk >= num_tracks) return false;
    delete tracks[trk];
    memmove((void *)(tracks + trk), (void *)(tracks + trk + 1), (num_tracks - trk -1) * sizeof(MIDITrack *));
    tracks[--num_tracks] = 0;
    return true;
}





void MIDIMultiTrack::EditCopy(MIDIClockTime start, MIDIClockTime end,
                                int tr_start, int tr_end, MIDIEditMultiTrack* edit) {
    if (tr_start < 0 || tr_end >= GetNumTracks()) return;

    edit->Clear();
    for (int i = 0; i <= tr_end; i++)
        edit->InsertTrack();
    edit->SetStartTrack(tr_start);
    edit->SetEndTrack(tr_end);

    edit->tracks[0]->SetEndTime(end - start);
    for (int i = tr_start; i <= tr_end; i++)
        tracks[i]->MakeInterval(start, end, edit->tracks[i]);

    DumpMIDIMultiTrack(this);
    DumpMIDIMultiTrack(edit);
}


void MIDIMultiTrack::EditCut(MIDIClockTime start, MIDIClockTime end, MIDIEditMultiTrack* edit) {
    DumpMIDIMultiTrack(this);

    EditCopy(start, end, 0, GetNumTracks()-1, edit);

    for (int i = 0; i < num_tracks; i++)
        GetTrack(i)->DeleteInterval(start, end);

    DumpMIDIMultiTrack(this);
}


void MIDIMultiTrack::EditClear(MIDIClockTime start, MIDIClockTime end, int tr_start, int tr_end) {
    DumpMIDIMultiTrack(this);

    for (int i = tr_start; i <= tr_end; i++)
        GetTrack(i)->ClearInterval(start, end);

    DumpMIDIMultiTrack(this);
}


void MIDIMultiTrack::EditInsert(MIDIClockTime start, int tr_start, int times, bool sysex,
                                MIDIEditMultiTrack* edit) {

    MIDIClockTime length = edit->tracks[0]->GetEndTime();
    int tr_end = tr_start + edit->GetEndTrack() - edit->GetStartTrack();

    for (int i = 0; i < num_tracks; i++)
        tracks[i]->InsertInterval(start, length * times, 0);    // inserts a blank interval

    DumpMIDIMultiTrack(this);
    DumpMIDIMultiTrack(edit);

    if(edit) {
        if (tr_start == 0)
            tr_start = 1;
            // skip track 0: it will be set from the corresponding INTTrack with a Recompose
        for (int i = tr_start; i <= tr_end; i++) {
            for(int j = 0; j < times; j++)
                tracks[i]->ReplaceInterval(start + j * length, length, sysex, edit->tracks[i]);
        }
    }

    DumpMIDIMultiTrack(this);
    DumpMIDIMultiTrack(edit);
}


void MIDIMultiTrack::EditReplace(MIDIClockTime start, int tr_start, int times, bool sysex,
                                 MIDIEditMultiTrack* edit) {
    MIDIClockTime length = edit->tracks[0]->GetEndTime() * times;
    int tr_end = tr_start + edit->GetEndTrack() - edit->GetStartTrack();

    DumpMIDIMultiTrack(this);
    DumpMIDIMultiTrack(edit);

    for (int i = tr_start; i <= tr_end; i++)
        tracks[i]->ClearInterval(start, length);        // deletes previous events

    if (tr_start == 0)
        tr_start = 1;
        // skip track 0: it will be set from the corresponding INTTrack with a Recompose
    if (start + times * edit->tracks[0]->GetEndTime() > tracks[0]->GetEndTime())
        tracks[0]->SetEndTime(start + times * edit->tracks[0]->GetEndTime());
    for (int i = tr_start; i <= tr_end; i++) {
        for(int j = 0; j < times; j++)
            tracks[i]->ReplaceInterval(start + j * edit->tracks[0]->GetEndTime(),
                                       edit->tracks[0]->GetEndTime(), sysex, edit->tracks[i]);
    }

    DumpMIDIMultiTrack(this);
}








MIDIMultiTrackIteratorState::MIDIMultiTrackIteratorState(int n_tracks) :
        num_tracks(n_tracks), cur_event_track(0)  {
    next_event_number = new int [DEFAULT_MAX_NUM_TRACKS];
    next_event_time = new MIDIClockTime [DEFAULT_MAX_NUM_TRACKS];

    Reset();
}


MIDIMultiTrackIteratorState::MIDIMultiTrackIteratorState(const MIDIMultiTrackIteratorState &m) :
        num_tracks(m.num_tracks), cur_time(m.cur_time),
        cur_event_track(m.cur_event_track) {

    next_event_number = new int [DEFAULT_MAX_NUM_TRACKS];
    next_event_time = new MIDIClockTime [DEFAULT_MAX_NUM_TRACKS];
    for(int i = 0; i < num_tracks; ++i) {
        next_event_number[i] = m.next_event_number[i];
        next_event_time[i] = m.next_event_time[i];
    }
}


MIDIMultiTrackIteratorState::~MIDIMultiTrackIteratorState() {
    delete[] next_event_number;
    delete[] next_event_time;
}


const MIDIMultiTrackIteratorState& MIDIMultiTrackIteratorState::operator= (const MIDIMultiTrackIteratorState &m) {
    num_tracks = m.num_tracks;
    cur_time = m.cur_time;
    cur_event_track = m.cur_event_track;
    for(int i = 0; i < num_tracks; ++i) {
        next_event_number[i] = m.next_event_number[i];
        next_event_time[i] = m.next_event_time[i];
    }
    return *this;
}


void MIDIMultiTrackIteratorState::Reset() {
    cur_time = 0;
    cur_event_track = 0;
    for(int i = 0; i < num_tracks; ++i) {
      next_event_number[i] = 0;
      next_event_time[i] = 0xffffffff;
    }
}


int MIDIMultiTrackIteratorState::FindTrackOfFirstEvent() {
    MIDIClockTime minimum_time = 0xffffffff;
    int minimum_time_track=-1;

    // go through all tracks and find the track with the smallest event time.

    for(int j = 0; j < num_tracks; ++j) {
        int i = (j + cur_event_track+1) % num_tracks;

    // skip any tracks that have a current event number less than 0 - these are finished already

        if(next_event_number[i] >= 0 && next_event_time[i] < minimum_time) {
            minimum_time = next_event_time[i];
            minimum_time_track = i;
        }
    }

    // set cur_event_track to -1 if there are no more events left
    cur_event_track = minimum_time_track;
    cur_time = minimum_time;
    return cur_event_track;
}






MIDIMultiTrackIterator::MIDIMultiTrackIterator(MIDIMultiTrack *mlt) :
    multitrack(mlt), state(mlt->GetNumTracks()) {}


MIDIMultiTrackIterator::~MIDIMultiTrackIterator() {}


void MIDIMultiTrackIterator::Reset() {
    state.SetNumTracks(multitrack->GetNumTracks());
    GoToTime(0);
}


void MIDIMultiTrackIterator::GoToTime(MIDIClockTime time) {
    // start at time 0

    state.Reset();

    // transfer info from the first events in each track in the
    // multitrack object to our current state.

    for(int i = 0; i < multitrack->GetNumTracks(); ++i) {
        MIDITrack *track = multitrack->GetTrack(i);

    // default: set the next_event_number for this track to -1 to signify end of track

        state.next_event_number[ i ] = -1;

    // are there any events in this track?
        if(track && track->GetNumEvents() > 0) {
    // yes, extract the time of the first event

            MIDITimedBigMessage *msg = track->GetEventAddress(0);
            if(msg) {
    // found the first message of the track. Keep track of the event number and the event time.

                state.next_event_number[i] = 0;
                state.next_event_time[i]=msg->GetTime();
            }
        }
    }

    // are there any events at all? find the track with the earliest event

    if(state.FindTrackOfFirstEvent() != -1) {
    // yes iterate through all the events until we find a time >= the requested time

        while(state.GetCurTime() < time) {
    // did not get to the requested time yet.
    // go to the next chronological event on all tracks

            if(!GoToNextEvent()) break;
    // there is no more events to go to

        }
    }

}


bool MIDIMultiTrackIterator::GetCurEventTime(MIDIClockTime *t) {
    // if there is a next event, then set *t to the time of the event and return true

    if(state.GetCurEventTrack() != -1) {
        *t = state.GetCurTime();
        return true;
    }
    else
      return false;
}


bool MIDIMultiTrackIterator::GetCurEvent(int *track, MIDITimedBigMessage **msg) {
    int t = state.GetCurEventTrack();

    if(t != -1) {
        if(track)
            *track=t;
        if(msg) {
            int num = state.next_event_number[t];
            if(num >= 0)
                *msg = multitrack->GetTrack(t)->GetEventAddress( state.next_event_number[t] );
            else
                *msg = 0;

        // do we really have a message?
            if(! *msg)
          // no, return false then
                return false;
        }
        return true;
    }
    else return false;

}


bool MIDIMultiTrackIterator::GoToNextEvent() {
    // find the next event in the multitrack list and return it
    // if there is no event left, return false

    if(state.cur_event_track == -1)
      // no tracks left - all tracks are at end
        return false;

    // update the current event for the current track to the
    // next event on the same track.

    GoToNextEventOnTrack(state.cur_event_track);

    // now find out which track now has the earliest event

    if(state.FindTrackOfFirstEvent() == -1)
      // No tracks do. all tracks are at the end. return false.
        return false;

    // ok, now state.cur_event_track has a valid track # of the next event

    return true;
}


bool MIDIMultiTrackIterator::GoToNextEventOnTrack(int track_num) {
    // Get the track that we are dealing with
    MIDITrack *track = multitrack->GetTrack(track_num);

    // Get ptr to the current event number for this track
    int *event_num = &state.next_event_number[track_num];

    // skip this track if this event number is <0 - This track has hit end already.

    if(*event_num < 0)
        return false; // at end of track

    // increment *event_num to next event on track

    (*event_num) += 1;

    // are we at end of track?
    if(*event_num >= track->GetNumEvents()) {
        // yes, set *event_num to -1
        *event_num=-1;
        return false; // at end of track
    }
    else {
      // not at end of track yet - get the time of the event
        MIDITimedBigMessage *msg;
        msg = track->GetEventAddress( *event_num );
        state.next_event_time[track_num] = msg->GetTime();
    }

    return true;
}

*********************/


