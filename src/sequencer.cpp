/*
 * ADAPTED FROM
 *
 * libjdkmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  BY NICOLA CASSETTA
 *
 *  *** RELEASED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) April 27, 2004 ***
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// TODO: da rivedere! Adattarlo a jdksmidi porterebbe a modifiche nell'interfaccia (AddTrack, GetNumTracks e
// forse altre



#include "../include/world.h"
#include "../include/sequencer.h"

/* Now unused
  static void FixQuotes( char *s_ )
  {
    unsigned char *s = (unsigned char *)s_;
    while( *s )
    {
      if( *s==0xd2 || *s==0xd3 )
      {
        *s='"';
      }
      else if( *s==0xd5 )
      {
        *s='\'';
      }
      else if( *s>=0x80 )
      {
        *s = ' ';
      }
      s++;
    }
  }
*/


////////////////////////////////////////////////////////////////////////////
//              class MIDISequencerTrackProcessor                         //
////////////////////////////////////////////////////////////////////////////


MIDISequencerTrackProcessor::MIDISequencerTrackProcessor() :
    mute(false), solo(false), velocity_scale(100), rechannel(-1), transpose(0), extra_proc(0) {
}


void MIDISequencerTrackProcessor::Reset() {
    mute = false;
    solo = false;
    velocity_scale = 100;
    rechannel = -1;
    transpose = 0;
}


bool MIDISequencerTrackProcessor::Process( MIDITimedMessage *msg ) {
    // are we muted?
    if(mute)
        // yes, ignore event.
        return false;

    // is the event a NoOp?
    if(msg->IsNoOp())
        // yes, ignore event.
        return false;

    // pass the event to our extra_proc if we have one
    if(extra_proc && extra_proc->Process(msg) == false)
        // extra_proc wanted to ignore this event
        return false;

    // is it a normal MIDI channel message?
    if(msg->IsChannelMsg()) {
        // yes, are we to re-channel it?
        if(rechannel != -1)
            msg->SetChannel((unsigned char)rechannel);

        // is it a note on message?
        if(msg->IsNoteOn() && msg->GetVelocity() > 0) {
            // yes, scale the velocity value as required
            int vel = (int)msg->GetVelocity();
            vel = vel * velocity_scale / 100;

            // make sure velocity is never less than 0
            if( vel < 0 )
                vel = 0;

            // rewrite the velocity
            msg->SetVelocity((unsigned char)vel);
        }

        // is it a type of event that needs to be transposed?
        if(msg->IsNoteOn() || msg->IsNoteOff() || msg->IsPolyPressure()) {
            int new_note = ((int)msg->GetNote()) + transpose;
            if(new_note >= 0 && new_note <= 127)

                // set new note number
                msg->SetNote((unsigned char)new_note);
            else
                // otherwise delete this note - transposed value is out of range
                return false;
        }
    }
    return true;
}



////////////////////////////////////////////////////////////////////////////
//              class MIDISequencerTrackState                             //
////////////////////////////////////////////////////////////////////////////


MIDISequencerTrackState::MIDISequencerTrackState():
    program(-1), volume(100), pan(64), rev(0), chr(0),
    bender_value(0), notes_are_on(false), note_matrix(), got_good_track_name(false) {}


void MIDISequencerTrackState::Reset() {
    program = -1;
    volume = 100;
    pan = 64;
    rev = 0;
    chr = 0;
    track_name = "";
    notes_are_on = false;
    bender_value = 0;
    note_matrix.Clear();
    got_good_track_name = false;
}
// Note by NC: I eliminated GoToZero()



/*  MOVED TO MIDISequencerState
bool MIDISequencerTrackState::Process( MIDITimedBigMessage *msg ) {
    // is the event a NoOp?

    if( msg->IsNoOp() )
        // yes, ignore event.
        return false;

    // is it a normal MIDI channel message?
    if( msg->IsChannelMsg() ) {

        if( msg->GetType()==PITCH_BEND )    // is it a bender event?
            // yes, remember the bender wheel value
             bender_value = msg->GetBenderValue();
        else if( msg->IsControlChange() ) { // is it a control change event?
            // yes
            // is it a volume change event?

            if( msg->GetController()==C_MAIN_VOLUME ) {
                // yes, store the current volume level
                volume = msg->GetControllerValue();
                Notify( MIDISequencerGUIEvent::GROUP_TRACK_VOLUME );
            }
        }
        else if( msg->IsProgramChange() ) { // is it a program change event?
            // yes, update the current program change value
            pg = msg->GetPGValue();
            Notify( MIDISequencerGUIEvent::GROUP_TRACK_PG );
        }
        // pass the message to our note matrix to keep track of all notes on
        // on this track

        if( note_matrix.Process( *msg ) ) {
            // did the "any notes on" status change?
            if(    ( notes_are_on && note_matrix.GetTotalCount()==0)
                      || (!notes_are_on && note_matrix.GetTotalCount()>0 ) ) {
                // yes, toggle our notes_are_on flag
                notes_are_on = !notes_are_on;
                // and notify the gui about the activity on this track
                Notify( MIDISequencerGUIEvent::GROUP_TRACK_NOTE );
            }
        }
    }
    else if( ( msg->GetMetaType()==META_TRACK_NAME
                 || msg->GetMetaType()==META_INSTRUMENT_NAME
                 || (!got_good_track_name && msg->GetMetaType()==META_GENERIC_TEXT && msg->GetTime()==0) )
               && msg->GetSysEx() ) {
    // this is a META message (sent only to track_state[0]) is it a track name event?
            got_good_track_name = true;
            int len = msg->GetSysEx()->GetLength();
            if( len > (int)sizeof(track_name) - 1 )
                len = (int)sizeof(track_name) - 1;
            memcpy(track_name, msg->GetSysEx()->GetBuf(), len );
            track_name[len]='\0';
            FixQuotes( track_name );
            Notify( MIDISequencerGUIEvent::GROUP_TRACK_NAME );
    }
    return true;
}
*/


////////////////////////////////////////////////////////////////////////////
//                      class MIDISequencerState                          //
////////////////////////////////////////////////////////////////////////////


MIDISequencerState::MIDISequencerState(MIDIMultiTrack *m, MIDISequencerGUINotifier *n) :
    notifier(n), multitrack(m), iterator(m), cur_clock(0), cur_time_ms(0),
    cur_beat(0), cur_measure(0), next_beat_time(0),
    tempobpm(120.0), timesig_numerator(4), timesig_denominator(4),
    keysig_sharpflat(0), keysig_mode(0), last_event_track(-1)
{
    track_states.resize(m->GetNumTracks());
}


MIDISequencerState::MIDISequencerState(const MIDISequencerState& s) :
    notifier(s.notifier), multitrack(s.multitrack), iterator(s.iterator),
    cur_clock(s.cur_clock), cur_time_ms(s.cur_time_ms), cur_beat(s.cur_beat),
    cur_measure(s.cur_measure), next_beat_time(s.next_beat_time),
    tempobpm(s.tempobpm), timesig_numerator(s.timesig_numerator),
    timesig_denominator(s.timesig_denominator), keysig_sharpflat(s.keysig_sharpflat),
    keysig_mode(s. keysig_mode), marker_text(s.marker_text), track_states(s.track_states),
    last_event_track(s.last_event_track)
{}


const MIDISequencerState& MIDISequencerState::operator= (const MIDISequencerState& s) {
    notifier = s.notifier;
    multitrack = s.multitrack;
    iterator.SetState(s.iterator.GetState());
    last_event_track = s.last_event_track;

    cur_clock = s.cur_clock;
    cur_time_ms = s.cur_time_ms;
    cur_beat = s.cur_beat;
    cur_measure = s.cur_measure;
    next_beat_time = s.next_beat_time;
    tempobpm = s.tempobpm;
    timesig_numerator = s.timesig_numerator;
    timesig_denominator = s.timesig_denominator;
    keysig_sharpflat = s.keysig_sharpflat;
    keysig_mode = s.keysig_mode;
    marker_text = s.marker_text;

    track_states = s.track_states;
    return *this;
}


void MIDISequencerState::Reset() {
    iterator.Reset();
    cur_clock = 0;
    cur_time_ms = 0.0;
    cur_beat = 0;
    cur_measure = 0;
    tempobpm = 120.0;
    timesig_numerator = 4;
    timesig_denominator = 4;
    keysig_sharpflat = 0;
    keysig_mode = 0;
    marker_text = "";
    next_beat_time = multitrack->GetClksPerBeat();
    if (multitrack->GetNumTracks() != track_states.size()) {
        track_states.clear();
        track_states.resize(multitrack->GetNumTracks());
    }
    else
        for (unsigned int i = 0; i < track_states.size(); i++)
            track_states[i].Reset();
}



bool MIDISequencerState::Process( MIDITimedMessage *msg ) {
    MIDISequencerTrackState& t_state = track_states[last_event_track];

    // is the event a NoOp?
    if( msg->IsNoOp() )
        return false;                   // ignore event.

    // is it a normal MIDI channel message?
    if(msg->IsChannelMsg()) {
        if( msg->GetType()==PITCH_BEND )    // is it a bender event?
            // yes, remember the bender wheel value
             t_state.bender_value = msg->GetBenderValue();
        else if( msg->IsControlChange() ) { // is it a control change event?
            // yes
            // is it a volume change event?
            if( msg->GetController() == C_MAIN_VOLUME ) {
                // yes, store the current level and notify the GUI
                t_state.volume = msg->GetControllerValue();
                NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_VOLUME);
            }
            // is it a pan change event?
            if( msg->GetController() == C_PAN ) {
                // idem
                t_state.pan = msg->GetControllerValue();
                NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_PAN);
            }
            // is it a chorus change event?
            if(msg->GetController() == C_CHORUS_DEPTH) {
                // idem
                t_state.chr = msg->GetControllerValue();
                NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_CHR);
            }
            // is it a reverb change event?
            if( msg->GetController() == C_EFFECT_DEPTH ) {
                // idem
                t_state.rev = msg->GetControllerValue();
                NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_REV);
            }
        }
        else if( msg->IsProgramChange() ) { // is it a program change event?
            // yes, update the current program change value
            t_state.program = msg->GetProgramValue();
            NotifyTrack( MIDISequencerGUIEvent::GROUP_TRACK_PROGRAM );
        }
        // pass the message to our note matrix to keep track of all notes on
        // on this track

        if(t_state.note_matrix.Process(*msg)) {
            // did the "any notes on" status change?
            if((t_state.notes_are_on && t_state.note_matrix.GetTotalCount() == 0) ||
               (!t_state.notes_are_on && t_state.note_matrix.GetTotalCount() > 0) ) {
                // yes, toggle our notes_are_on flag
                t_state.notes_are_on = !t_state.notes_are_on;
                // and notify the gui about the activity on this track
                NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_NOTE);
            }
        }
        return true;
    }

    if(msg->IsMetaEvent()) {            // is it a meta-event?

        if(msg->IsTempo()) {            // is it a tempo event?
            tempobpm = ((float)msg->GetTempo32())*(1.0f/32.0f);
            if(tempobpm < 1 )
            tempobpm=120.0;
            Notify(MIDISequencerGUIEvent::GROUP_CONDUCTOR,
                   MIDISequencerGUIEvent::GROUP_CONDUCTOR_TEMPO);
        }
        else if(msg->IsTimeSig()) {     // is it a time signature event?
            timesig_numerator = msg->GetTimeSigNumerator();
            timesig_denominator = msg->GetTimeSigDenominator();
            Notify(MIDISequencerGUIEvent::GROUP_CONDUCTOR,
                   MIDISequencerGUIEvent::GROUP_CONDUCTOR_TIMESIG);
        }
        else if(msg->IsKeySig()) {      // is it a key signature event?
            keysig_sharpflat = msg->GetKeySigSharpFlats();
            keysig_mode = msg->GetKeySigMajorMinor();
            Notify(MIDISequencerGUIEvent::GROUP_CONDUCTOR,
                   MIDISequencerGUIEvent::GROUP_CONDUCTOR_KEYSIG);
        }
        else if ( msg->IsMarkerText()) {
            marker_text = std::string((const char *)msg->GetSysEx()->GetBuf(), msg->GetSysEx()->GetLength());
            Notify(MIDISequencerGUIEvent::GROUP_CONDUCTOR,
                   MIDISequencerGUIEvent::GROUP_CONDUCTOR_MARKER);
        }
        else if ( msg->IsBeatMarker()) {
            // update our beat count
            int new_beat = cur_beat + 1;
            int new_measure = cur_measure;

            // do we need to update the measure number?
            if( new_beat >= timesig_numerator ) {
                // yup
                new_beat=0;
                ++new_measure;
            }

            // update our next beat time
            // denom=4  (16) ---> 4/16 midi file beats per symbolic beat
            // denom=3  (8)  ---> 4/8 midi file beats per symbolic beat
            // denom=2  (4)  ---> 4/4 midi file beat per symbolic beat
            // denom=1  (2)  ---> 4/2 midi file beats per symbolic beat
            // denom=0  (1)  ---> 4/1 midi file beats per symbolic beat

            next_beat_time +=
                multitrack->GetClksPerBeat() * 4 / timesig_denominator;
            cur_beat = new_beat;
            cur_measure = new_measure;

            // now notify the GUI that the beat number changed
            Notify(MIDISequencerGUIEvent::GROUP_TRANSPORT,
                   MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT);

            // if the new beat number is 0 then the measure changed too
            if( cur_beat == 0 )
                Notify(MIDISequencerGUIEvent::GROUP_TRANSPORT,
                       MIDISequencerGUIEvent::GROUP_TRANSPORT_MEASURE);
        }
        else if( ( msg->GetMetaType()==META_TRACK_NAME
                 || msg->GetMetaType()==META_INSTRUMENT_NAME
                 || (!t_state.got_good_track_name && msg->GetMetaType()==META_GENERIC_TEXT && msg->GetTime()==0) )
               && msg->GetSysEx() ) {
            // is it a track name event?
            t_state.got_good_track_name = true;
            int len = msg->GetSysEx()->GetLength();
            t_state.track_name = std::string((const char *)msg->GetSysEx()->GetBuf(), len);
            //FixQuotes( track_name );
            NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_NAME);
        }
        return true;
    }
    return false;
}


void MIDISequencerState::Notify(int group, int item) const {
    if (notifier) {
        MIDISequencerGUIEvent e(group, 0, item);
        notifier->Notify(e);
    }
}


void MIDISequencerState::NotifyTrack(int item) const {
    if (notifier) {
        MIDISequencerGUIEvent e(MIDISequencerGUIEvent::GROUP_TRACK,
                                last_event_track,
                                item);
        notifier->Notify(e);
    }
}



////////////////////////////////////////////////////////////////////////////
//                        class MIDISequencer                             //
////////////////////////////////////////////////////////////////////////////


MIDISequencer::MIDISequencer (MIDIMultiTrack *m, MIDISequencerGUINotifier *n) :
    solo_mode (false), tempo_scale (100), state (m, n) {
    if (n)
        n->SetSequencer(this);
    track_processors.resize(GetNumTracks());
    for (unsigned  int i = 0; i < GetNumTracks(); ++i)
        track_processors[i] = new MIDISequencerTrackProcessor;
}


MIDISequencer::~MIDISequencer() {
    for (unsigned int i = 0; i < GetNumTracks(); ++i)
        delete track_processors[i];
}

// OK WITH JDKSMIDI
/* This should be unneeded
void MIDISequencer::ResetTrack (int trk)
{
    state.track_states[trk].Reset();    // TODO: OBJECTS OR POINTERS?
    track_processors[trk]->Reset();
}
*/


void MIDISequencer::Reset() {
    state.Reset();
    if (track_processors.size() != GetNumTracks()) {
        for (unsigned int i = 0; i < track_processors.size(); i++)
            delete track_processors[i];
        track_processors.resize(GetNumTracks());
        for (unsigned int i = 0; i < track_processors.size(); i++)
            track_processors[i] = new MIDISequencerTrackProcessor;
    }
    else
        for (unsigned int i = 0; i < GetNumTracks(); ++i)
            track_processors[i]->Reset();
}


// OK WITH JDKSMIDI
void MIDISequencer::SetSoloMode(bool m, int trk)  {
    solo_mode = m;

    for(unsigned int i = 0; i < GetNumTracks(); ++i ) {
        if(i == (unsigned)trk)
            track_processors[i]->solo = true;
        else
            track_processors[i]->solo = false;
    }
}

// These are new
bool MIDISequencer::InsertTrack(int trk) {
    if (trk == -1) trk = GetNumTracks();                // if trk = -1 (default) append track
    if (state.multitrack->InsertTrack(trk)) {
        track_processors.insert(track_processors.begin() + trk, new MIDISequencerTrackProcessor);
        MIDIClockTime now = state.cur_clock;            // remember current time
        state.Reset();                                  // reset the state (syncs the iterator)
        GoToTime(now);                                  // returns to current time
        return true;
    }
    return false;
}


bool MIDISequencer::DeleteTrack(int trk) {
    if (state.multitrack->DeleteTrack(trk)) {
        track_processors.erase(track_processors.begin() + trk);
        MIDIClockTime now = state.cur_clock;            // remember current time
        state.Reset();                                  // reset the state (syncs the iterator)
        GoToTime(now);                                  // returns to current time
        return true;
    }
    return false;
}


bool MIDISequencer::MoveTrack(int from, int to) {
    if (from == to) return true;                    // nothing to do
    if (state.multitrack->MoveTrack(from, to)) {
        MIDISequencerTrackProcessor* temp = track_processors[from];
        track_processors.erase(track_processors.begin() + from);
        if (from < to)
            to--;
        track_processors.insert(track_processors.begin() + to, temp);
        MIDIClockTime now = state.cur_clock;            // remember current time
        state.Reset();                                  // reset the state (syncs the iterator)
        GoToTime(now);                                  // returns to current time
        return true;
    }
    return false;
}

// OK WITH JDKSMIDI
void MIDISequencer::GoToZero() {
    // go to time zero
    state.Reset();

    // examine all the events at this specific time
    // and update the track states to reflect this time
    ScanEventsAtThisTime();
    // cause a full gui refresh now
    state.Notify(MIDISequencerGUIEvent::GROUP_ALL);
}

// REVISED
bool MIDISequencer::GoToTime(MIDIClockTime time_clk) {
    bool ret = true;

        // temporarily disable the gui notifier
    bool notifier_mode = false;
    if(state.notifier) {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable(false);
    }

    // OLD VERSION if(time_clk < state.cur_clock || time_clk == 0)
    if(time_clk <= state.cur_clock)     // we must restart also if cur_clock is equal to cur_clock, as we could
                                        // already have got some event. Moreover this is good if we have edited the
                                        // multitrack
        // start from zero if desired time is before where we are
        state.Reset();

    MIDIClockTime t;
    int trk;
    MIDITimedMessage ev;

    while (state.cur_clock < time_clk) {
        if (!GetNextEventTime(&t)) {    // no other events: we can't reach time_clk and return false
            ret = false;
            break;
        }
        if (t < time_clk)               // next event is before time_clk
            GetNextEvent ( &trk, &ev ); // get it and continue
        else if (t == time_clk) {       // next event is at the right time
            MIDIMultiTrackIteratorState istate(state.iterator.GetState());  // save the state of the iterator
            GetNextEvent( &trk, &ev );          // get the event (cur_time becomes time_clk)
            state.iterator.SetState( istate );  // restore the iterator state, so ev is the next event
        }
        else {                          // next event is after time_clk : set cur_time to time_clk
            state.cur_clock = time_clk;
            state.cur_time_ms = MIDItoMs(time_clk);
        }
    }

        // examine all the events at this specific time
        // and update the track states to reflect this time
    if (ret) ScanEventsAtThisTime();

        // re-enable the gui notifier if it was enabled previously
    if( state.notifier ) {
        state.notifier->SetEnable(notifier_mode);
        // cause a full gui refresh now
        state.notifier->Notify(MIDISequencerGUIEvent::GROUP_ALL);
    }
    return ret;
}

// REVISED
bool MIDISequencer::GoToTimeMs(double time_ms) {
    bool ret = true;

        // temporarily disable the gui notifier
    bool notifier_mode = false;
    if(state.notifier) {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable(false);
    }

    // OLD VERSION if(time_ms < state.cur_time_ms || time_ms == 0.0)
    if(time_ms <= state.cur_time_ms)
                                        // we must restart also if cur_clock is equal to cur_clock, as we could
                                        // already have got some event. Moreover this is good if we have edited the
                                        // multitrack
        // start from zero if desired time is before where we are
        state.Reset();

    double t;
    int trk;
    MIDITimedMessage ev;

    while (state.cur_time_ms < time_ms) {
        if (!GetNextEventTimeMs(&t)) {  // no other events: we can't reach time_clk and return false
            ret = false;
            break;
        }
        if (t < time_ms)                        // next event is before time_ms
            GetNextEvent (&trk, &ev);       // get it and continue
        else if (abs (t - time_ms) < 0.001){    // next event is at right time
            MIDIMultiTrackIteratorState istate(state.iterator.GetState());  // save the state of the iterator
            GetNextEvent(&trk, &ev);            // get the event
            state.iterator.SetState(istate);    // restore the iterator state, so ev is the next event
        }
        else {                          // next event is after time_clk : set cur_time to time_clk
            double delta_t_ms = t - time_ms;
            double ms_per_clock = (double)6000000.0 / (state.tempobpm *    // see MIDIToMs()
                    (double)tempo_scale * state.multitrack->GetClksPerBeat());
            state.cur_clock += (MIDIClockTime)(delta_t_ms / ms_per_clock);
            state.cur_time_ms = time_ms;
        }
    }

        // examine all the events at this specific time
        // and update the track states to reflect this time
	if (ret) ScanEventsAtThisTime();

        // re-enable the gui notifier if it was enabled previously
    if( state.notifier ) {
        state.notifier->SetEnable( notifier_mode );
        // cause a full gui refresh now
        state.notifier->Notify(MIDISequencerGUIEvent::GROUP_ALL);
    }
    return ret;
}

// This is simpler because every measure has as first event a beat event!
// REVISED (not tested)
bool MIDISequencer::GoToMeasure (int measure, int beat) {
    bool ret = true;

        // temporarily disable the gui notifier
    bool notifier_mode = false;
    if (state.notifier) {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable (false);
    }

//    if (measure < state.cur_measure ||
//         // ADDED FOLLOWING LINE:  this failed in this case!!!
//        (measure == state.cur_measure && beat < state.cur_beat) ||
//        (measure == 0 && beat == 0))
    if (measure < state.cur_measure ||
            // ANOTHER CORRECTION!!!
        (measure == state.cur_measure && beat <= state.cur_beat))
        state.Reset();

    int trk;
    MIDITimedMessage ev;

        // iterate thru all the events until cur-measure and cur_beat are
        // where we want them.
    if (measure > 0 || beat > 0) {          // if meas == 0 && beat == 0 nothing to do
        while(1) {
            if (!GetNextEvent(&trk, &ev)) {
                ret = false;
                break;
            }
            if (ev.IsBeatMarker()) {        // there must be a beat marker at right time
                if(state.cur_measure == measure && state.cur_beat >= beat)
                    break;
            }
        }
    }
        // examine all the events at this specific time
        // and update the track states to reflect this time
    if (ret) ScanEventsAtThisTime();    // TODO: is this necessary after GetNextEvent() ?

        // re-enable the gui notifier if it was enabled previously
    if (state.notifier) {
        state.notifier->SetEnable (notifier_mode);
        // cause a full gui refresh now
        state.Notify (MIDISequencerGUIEvent::GROUP_ALL);
    }
        // return true if we actually found the measure requested
    return ret;
}

// OK WITH JDKSMIDI
bool MIDISequencer::GetNextEventTime(MIDIClockTime *time_clk) {
    // ask the iterator for the current event time
    bool f = state.iterator.GetCurEventTime(time_clk);

    if(f) {
        // if we have an event in the future, check to see if it is
        // further in time than the next beat marker

        if((*time_clk) >= state.next_beat_time)
            // ok, the next event is a beat - return the next beat time
            *time_clk = state.next_beat_time;
    }
    return f;
}

// REVISED (uses MIDItoMs)
bool MIDISequencer::GetNextEventTimeMs(double *time_ms) {
    MIDIClockTime t;
    bool f = GetNextEventTime(&t);

    if(f)
        *time_ms = MIDItoMs(t);
    return f;
}

// REVISED
bool MIDISequencer::GetNextEvent(int *trk, MIDITimedMessage *msg) {
    MIDIClockTime t;

    // ask the iterator for the current event time
    if( state.iterator.GetCurEventTime(&t) ) {
        // move current time forward one event
        MIDIClockTime new_clock;
        double new_time_ms;
        GetNextEventTime(&new_clock);
        GetNextEventTimeMs(&new_time_ms);

        // must set cur_clock AFTER GetNextEventTimeMs() is called
        // since GetNextEventTimeMs() uses cur_clock to calculate

        state.cur_clock = new_clock;
        state.cur_time_ms = new_time_ms;

        // is the next beat marker before this event?
        if(state.next_beat_time <= t) {
            // yes, this is a beat event now.
            // say this event came on track 0, the conductor track
            state.last_event_track = *trk = 0;

            // put current info into beat marker message
            beat_marker_msg.SetBeatMarker();
            beat_marker_msg.SetTime( state.next_beat_time );
            *msg = beat_marker_msg;

            state.Process(msg);
        }
        else    {   // this event comes before the next beat
            MIDITimedMessage *msg_ptr;

            if(state.iterator.GetCurEvent(trk, &msg_ptr)) {
                state.last_event_track = *trk;

                // copy the event so Process can modify it
                *msg = *msg_ptr;
                 bool allow_msg = true;

                // are we in solo mode?
                if( solo_mode ) {
                    // yes, only allow this message thru if
                    // the track is either track 0
                    // or it is explicitly solod.
                    allow_msg = (*trk == 0 || track_processors[*trk]->solo) ? true : false;
                }
                if(!(allow_msg
                   && track_processors[*trk]->Process(msg)
                   && state.Process(msg)))
                    // the message is not allowed to come out!
                    // erase it
                    msg->Clear();

                // go to the next event on the multitrack
                state.iterator.GoToNextEvent();
            }
        }
        return true;
    }
    return false;
}

// This is new
double MIDISequencer::MIDItoMs(MIDIClockTime t) {
    MIDITimedMessage* msg;
    MIDITrackIterator tr_iter(state.multitrack->GetTrack(0));
    MIDIClockTime base_t = 0, delta_t = 0, now_t = 0;
    double ms_time = 0.0, old_tempo = 120.0, ms_per_clock;

    while (now_t < t) {
        if (!tr_iter.GetCurEvent(&msg, t))      // next message is after t or doesn't exists
            now_t = t;
        else
            now_t = msg->GetTime();
        if (msg->IsTempo() || now_t == t) {
            // delta time in MIDI clocks
            delta_t = now_t - base_t;

            // calculate delta time in milliseconds: this comes from
            //  -true_bpm = old_tempo * tempo_scale / 100
            //  -clocks_per_sec = true_bpm * clks_per_beat / 60
            //  -clocks_per_ms = clocks_per_sec / 1000
            //  -ms_per_clock = 1 / clocks_per_ms
            ms_per_clock = (double)6000000.0 / (old_tempo *
                                (double)tempo_scale * state.multitrack->GetClksPerBeat());

            // and add it to ms_time
            ms_time += (delta_t * ms_per_clock);

            // update variables
            base_t = msg->GetTime();
            if (msg->IsTempo())
                old_tempo = msg->GetTempo32() / 32.0;
            if (now_t == t) break;
        }
    }
    return ms_time;
}


void MIDISequencer::ScanEventsAtThisTime() {
    // save the current iterator state
    MIDIMultiTrackIteratorState istate( state.iterator.GetState() );
    int prev_measure = state.cur_measure;
    int prev_beat = state.cur_beat;

    // process all messages up to and including this time only
    MIDIClockTime orig_clock = state.cur_clock;
    double orig_time_ms = state.cur_time_ms;

    MIDIClockTime t = 0;
    int trk;
    MIDITimedMessage ev;

    while( GetNextEventTime(&t) && t == orig_clock && GetNextEvent(&trk, &ev)) {
        ;  // cycle through all events at this time
    }

    // restore the iterator state
    state.iterator.SetState( istate );

    // and current time
    state.cur_clock = orig_clock;
    state.cur_time_ms = float(orig_time_ms);

    state.cur_measure = prev_measure;
    state.cur_beat = prev_beat;
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
//
// Copyright (C) 2010 V.R.Madgazin
// www.vmgames.com vrm@vmgames.com
//

//
// MODIFIED by N. Cassetta ncassetta@tiscali.it
// search /* NC */ for modifies
//

/*
#include "jdksmidi/world.h"
#include "jdksmidi/sequencer.h"

namespace jdksmidi
{





MIDISequencerTrackState::MIDISequencerTrackState (
    const MIDISequencer *seq_,
    int trk,
    MIDISequencerGUIEventNotifier *n
)
    :
    pg (-1),
    volume ( 100 ),
    bender_value ( 0 ),
    got_good_track_name ( false ),
    notes_are_on ( false ),
    note_matrix(),
    seq ( seq_ ),   /* NEW
    track ( trk ),
    notifier ( n )
{
    *track_name = 0;
}


void MIDISequencerTrackState::GoToZero()
{
    pg = -1;
    volume = 100;
    notes_are_on = false;       // BUG! I forgot this!
    bender_value = 0;
    note_matrix.Clear();
}

void MIDISequencerTrackState::Reset()
{
    pg = -1;
    volume = 100;
    notes_are_on = false;
    bender_value = 0;
    *track_name = 0;
    note_matrix.Clear();
    got_good_track_name = false;
}


bool MIDISequencerTrackState::Process ( MIDITimedBigMessage *msg )
/* NC            // this has been splitted into this and MIDISequencerState::Process()
{
    // is the event a NoOp?
    if ( msg->IsNoOp() )
    {
        // yes, ignore event.
        return false;
    }

    // is it a normal MIDI channel message?
    if ( msg->IsChannelMsg() )
    {
        if ( msg->GetType() == PITCH_BEND ) // is it a bender event?
        {
            // yes
            // remember the bender wheel value
            bender_value = msg->GetBenderValue();
        }

        else if ( msg->IsControlChange() ) // is it a control change event?
        {
            // yes
            // is it a volume change event?
            if ( msg->GetController() == C_MAIN_VOLUME )
            {
                // yes, store the current volume level
                volume = msg->GetControllerValue();
                Notify ( MIDISequencerGUIEvent::GROUP_TRACK_VOLUME );  // and notify the GUI
            }
        }

        else if ( msg->IsProgramChange() ) // is it a program change event?
        {
            // yes
            // update the current program change value
            pg = msg->GetPGValue();
            Notify ( MIDISequencerGUIEvent::GROUP_TRACK_PG );   // and notify the GUI
        }

        // pass the message to our note matrix to keep track of all notes on this track
        if ( note_matrix.Process ( *msg ) )
        {
            // did the "any notes on" status change?
            if ( ( notes_are_on && note_matrix.GetTotalCount() == 0 )
                || ( !notes_are_on && note_matrix.GetTotalCount() > 0 ) )
            {
                // yes, toggle our notes_are_on flag
                notes_are_on = !notes_are_on;
                // and notify the gui about the activity on this track
                Notify ( MIDISequencerGUIEvent::GROUP_TRACK_NOTE );
            }
        }
    }

    else
    {
        // event is not a channel message. is it a meta-event?
        if ( msg->IsMetaEvent() )
        {
            // is it a track name event?
            if ( ( msg->GetMetaType() == META_TRACK_NAME
                    || msg->GetMetaType() == META_INSTRUMENT_NAME
                    || ( !got_good_track_name && msg->GetMetaType() == META_GENERIC_TEXT && msg->GetTime() == 0 )
                 )
                    &&
                 msg->GetSysEx() )
                // this is a META message (sent only to track_state[0]) is it a track name event?
            {
                got_good_track_name = true;
                // yes, copy the track name
                int len = msg->GetSysEx()->GetLengthSE();

                if ( len > ( int ) sizeof ( track_name ) - 1 )
                    len = ( int ) sizeof ( track_name ) - 1;

                memcpy ( track_name, msg->GetSysEx()->GetBuf(), len );
                track_name[len] = '\0';
                FixQuotes ( track_name );
                Notify ( MIDISequencerGUIEvent::GROUP_TRACK_NAME );
            }
        }
    }

    return true;
}


*/
////////////////////////////////////////////////////////////////////////////
/*
MIDISequencerState::MIDISequencerState (
    const MIDISequencer *s,
    const MIDIMultiTrack * m,
    MIDISequencerGUIEventNotifier *n
)
    :
    notifier ( n ),
    seq ( s ),                                  /* NC
    multitrack ( m ),
    num_tracks ( m->GetNumTracks() ),
    iterator ( m ),
    cur_clock ( 0 ),
    cur_time_ms ( 0 ),
    cur_beat ( 0 ),
    cur_measure ( 0 ),
    last_beat_time ( 0 ),                       /* NC
    next_beat_time ( 0 ),
    tempobpm( 120.0 ),                          /* NC
    timesig_numerator( 4 ),                     /* NC
    timesig_denominator( 4 ),                   /* NC
    keysig_sharpflat( 0 ),                      /* NC
    keysig_mode( 0 )                            /* NC
{
    for ( int i = 0; i < num_tracks; ++i )
    {
        track_state[i] = new MIDISequencerTrackState ( s, i, notifier );
    }
    *marker_name = 0;                           /* NC
}

MIDISequencerState::MIDISequencerState ( const MIDISequencerState &s )
    :
    notifier ( s.notifier ),
    seq (s.seq ),                               /* NC
    multitrack ( s.multitrack ),
    num_tracks ( s.num_tracks ),
    iterator ( s.iterator ),
    cur_clock ( s.cur_clock ),
    cur_time_ms ( s.cur_time_ms ),
    cur_beat ( s.cur_beat ),
    cur_measure ( s.cur_measure ),
    last_beat_time ( s.last_beat_time ),        /* NC
    next_beat_time ( s.next_beat_time ),
    tempobpm( s.tempobpm ),                     /* NC
    timesig_numerator( s.timesig_numerator ),   /* NC
    timesig_denominator( s.timesig_denominator),/* NC
    keysig_sharpflat( s.keysig_sharpflat ),     /* NC
    keysig_mode( s. keysig_mode )               /* NC
{
    for ( int i = 0; i < num_tracks; ++i )
    {
        track_state[i] = new MIDISequencerTrackState ( *s.track_state[i] );
    }
    memmove( marker_name, s.marker_name, sizeof( marker_name ) );    /* NC
}


MIDISequencerState::~MIDISequencerState()
{
    for ( int i = 0; i < num_tracks; ++i )
    {
        jdks_safe_delete_object( track_state[i] );
    }
}


/* NC NEW corrected
const MIDISequencerState & MIDISequencerState::operator = ( const MIDISequencerState & s )
{
    notifier = s.notifier;
    seq = s.seq;
    multitrack = s.multitrack;

    if ( num_tracks != s.num_tracks )
    {
        {
            for ( int i = 0; i < num_tracks; ++i )
            {
                jdks_safe_delete_object( track_state[i] );
            }
        }
        num_tracks = s.num_tracks;
        {
            for ( int i = 0; i < num_tracks; ++i )
            {
                track_state[i] = new MIDISequencerTrackState ( *s.track_state[i] );
            }
        }
    }
    else
    {
        for( int i = 0; i < num_tracks; ++i )
        {   // copies track states
            track_state[i]->pg = s.track_state[i]->pg;
            track_state[i]->volume = s.track_state[i]->volume;
            track_state[i]->bender_value = s.track_state[i]->bender_value;
            memmove( track_state[i]->track_name, s.track_state[i]->track_name, sizeof ( track_state[i] ) );
            track_state[i]->got_good_track_name = s.track_state[i]->got_good_track_name;
            track_state[i]->notes_are_on = s.track_state[i]->notes_are_on;
            memmove(&(track_state[i]->note_matrix), &(s.track_state[i]->note_matrix), sizeof(MIDIMatrix));
            track_state[i]->seq = s.seq;
            track_state[i]->track = s.track_state[i]->track;
            track_state[i]->notifier = s.notifier;
        }
    }

    iterator.SetState(s.iterator.GetState());
    cur_clock = s.cur_clock;
    cur_time_ms = s.cur_time_ms;
    cur_beat = s.cur_beat;
    cur_measure = s.cur_measure;
    last_beat_time = s.last_beat_time;  /* NC
    next_beat_time = s.next_beat_time;
    tempobpm = s.tempobpm;
    timesig_numerator = s.timesig_numerator;
    timesig_denominator = s.timesig_denominator;
    keysig_sharpflat = s.keysig_sharpflat;
    keysig_mode = s.keysig_mode;
    memmove( marker_name, s.marker_name, sizeof ( marker_name ) );

    return *this;
}


void MIDISequencerState::Reset() {              /* NC
    iterator.GoToTime(0);
    cur_clock = 0;
    cur_time_ms = 0.0;
    cur_beat = 0;
    cur_measure = 0;
    tempobpm = 120.0;
    timesig_numerator = 4;
    timesig_denominator = 4;
    keysig_sharpflat = 0;
    keysig_mode = 0;
    last_beat_time = 0;
    next_beat_time = multitrack->GetClksPerBeat() * 4 / timesig_denominator;
    for( int i=0; i<num_tracks; ++i )
    {
        track_state[i]->GoToZero();
    }
    *marker_name = 0;
}



bool MIDISequencerState::Process( MIDITimedBigMessage *msg ) /* NC
{
// new: this come from MIDISequencerTrackState::Process

    // is the event a NoOp?
    if ( msg->IsNoOp() )
    {
        // yes, ignore event.
        return false;
    }

    else // is it a normal MIDI channel message?
    if( msg->IsChannelMsg() )
    {
        // give it to its MIDISequencerTrackState
        return track_state[ last_event_track ]->Process(msg);
    }

    else // is it a meta-event?
    if ( msg->IsMetaEvent() )
    {
        // yes, is it a tempo event?
        if ( msg->IsTempo() )
        {
            // yes get the current tempo
            tempobpm = ( float ) ( msg->GetTempo32() / 32. );

            if ( tempobpm < 1. )
                tempobpm = 120.0;

            Notify (
                MIDISequencerGUIEvent::GROUP_CONDUCTOR,
                MIDISequencerGUIEvent::GROUP_CONDUCTOR_TEMPO
            );
        }

        else // is it a time signature event?
        if ( msg->IsTimeSig() )
        {
            // yes, extract the current numerator and denominator
            timesig_numerator = msg->GetTimeSigNumerator();
            timesig_denominator = msg->GetTimeSigDenominator();
            Notify (
                MIDISequencerGUIEvent::GROUP_CONDUCTOR,
                MIDISequencerGUIEvent::GROUP_CONDUCTOR_TIMESIG
            );
        }


        else // is it a key signature event?
        if( msg->IsKeySig() )
        {
            // yes, extract keysig accidents and mode
            keysig_sharpflat = msg->GetKeySigSharpFlats();
            keysig_mode = msg->GetKeySigMajorMinor();
            Notify (
                MIDISequencerGUIEvent::GROUP_CONDUCTOR,
                MIDISequencerGUIEvent::GROUP_CONDUCTOR_KEYSIG
            );
        }


        else // is it a marker name event?
        if ( msg->IsMarkerText() )
        {
            // yes, copy its name
            int len = msg->GetSysEx()->GetLengthSE();
            if ( len > ( int ) sizeof ( marker_name ) - 1 )
                len = ( int ) sizeof ( marker_name ) - 1;
            memcpy ( marker_name, msg->GetSysEx()->GetBuf(), len );
            marker_name[len] = 0;   /* NC: there was a bug!
            Notify (
                MIDISequencerGUIEvent::GROUP_CONDUCTOR,
                MIDISequencerGUIEvent::GROUP_CONDUCTOR_MARKER
            );
        }
        else { // could be a track name meta event
            return track_state[ last_event_track ]->Process(msg);
        }
    }
    else  // is the message a beat marker?
    if ( msg->IsBeatMarker())
    {
        // NOTE BY NC: newly revised, now sends the beat marker msg even at start
        int new_beat = cur_beat;
        int new_measure = cur_measure;
        if ( last_beat_time != next_beat_time )
        {   // if at start, wmust not upgrade beat count
            // update our beat count
            ++new_beat;

            // do we need to update the measure number?
            if( new_beat >= timesig_numerator )
            {
                // yup
                new_beat=0;
                ++new_measure;
            }
        }


        // update our next beat timetrack_state[ 0 ]->Notify
        // denom=4  (16) ---> 4/16 midi file beats per symbolic beat
        // denom=3  (8)  ---> 4/8 midi file beats per symbolic beat
        // denom=2  (4)  ---> 4/4 midi file beat per symbolic beat
        // denom=1  (2)  ---> 4/2 midi file beats per symbolic beat
        // denom=0  (1)  ---> 4/1 midi file beats per symbolic beat

        last_beat_time = cur_clock; /* NC
        next_beat_time +=
            multitrack->GetClksPerBeat() * 4 / timesig_denominator;
        cur_beat = new_beat;
        cur_measure = new_measure;

        // now notify the GUI that the beat number changed
        Notify (
            MIDISequencerGUIEvent::GROUP_TRANSPORT,
            MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT
        );

        // if the new beat number is 0 then the measure changed too
        if( cur_beat == 0 ) {
            Notify (
                MIDISequencerGUIEvent::GROUP_TRANSPORT,
                MIDISequencerGUIEvent::GROUP_TRANSPORT_MEASURE
            );
        }
    }
    return true;
}


////////////////////////////////////////////////////////////////////////////


MIDISequencer::MIDISequencer (
    const MIDIMultiTrack *m,
    MIDISequencerGUIEventNotifier *n
)
    :
    solo_mode ( false ),
    tempo_scale ( 100 ),
    num_tracks ( m->GetNumTracks() ),
    state ( this, m, n ) // TO DO: fix this hack
{
    for ( int i = 0; i < num_tracks; ++i )
    {
        track_processors[i] = new MIDISequencerTrackProcessor;
    }
}


MIDISequencer::~MIDISequencer()
{
    for ( int i = 0; i < num_tracks; ++i )
    {
        jdks_safe_delete_object( track_processors[i] );
    }
}



/* NC: THESE WERE BUGGY (they stopped AFTER the first event at right time)
bool MIDISequencer::GoToTime( MIDIClockTime time_clk ) {
    bool ret = true;

    // temporarily disable the gui notifier
    bool notifier_mode = false;

    if ( state.notifier )
    {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable ( false );
    }

    if ( time_clk < state.cur_clock || time_clk == 0 )
    {
        // start from zero if desired time is before where we are
        state.Reset();
    }

    MIDIClockTime t;
    int trk;
    MIDITimedBigMessage ev;

    while ( 1 )
    {
        if ( GetCurrentMIDIClockTime() == time_clk )
            break;                      // we are already at right time
        if ( !GetNextEventTime( &t  ))  // no other events: we can't reach time_clk and return false
        {
            ret = false;
            break;
        }
        if ( t <= time_clk )            // next event is before or at right time
        {
            GetNextEvent ( &trk, &ev ); // get it and continue
        }
        else                            // next event is after time_clk : set cur_time to time_clk
        {                               // and update cur_time_ms
            MIDIClockTime delta_time = time_clk - state.cur_clock;
            // calculate delta_time in milliseconds: this comes from
            //  -true_bpm = tempobpm * tempo_scale / 100
            //  -clocks_per_sec = true_bpm * clks_per_beat / 60
            //  -clocks_per_ms = clocks_per_sec / 1000
            //  -ms_per_clock = 1 / clocks_per_ms
            float ms_per_clock = ( double )6000000.0 / (state.tempobpm *
                                ( double )tempo_scale * state.multitrack->GetClksPerBeat());
            state.cur_clock = time_clk;
            state.cur_time_ms += ( ms_per_clock * delta_time );
            break;
        }
    }

        // examine all the events at this specific time
        // and update the track states to reflect this time

    ScanEventsAtThisTime();

        // re-enable the gui notifier if it was enabled previously
    if( state.notifier )
    {
        state.notifier->SetEnable( notifier_mode );
        // cause a full gui refresh now
        state.Notify( MIDISequencerGUIEvent::GROUP_ALL );
    }
    return ret;
}


bool MIDISequencer::GoToTimeMs( float time_ms ) {
    bool ret = true;

    // temporarily disable the gui notifier
    bool notifier_mode = false;

    if ( state.notifier )
    {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable( false );
    }

    if ( time_ms < state.cur_time_ms || time_ms==0.0 ) {
        // start from zero if desired time is before where we are
        state.Reset();
    }

    double t;
    int trk;
    MIDITimedBigMessage ev;

    while ( 1 )
    {
        if ( GetCurrentTimeInMs() == time_ms )
            break;                          // we are already at right time
        if ( !GetNextEventTimeMs( &t ) )    // no other events: we can't reach time_clk and return false
        {
            ret = false;
            break;
        }
        if ( t  <= time_ms ) // next event is before or at right time
        {
            GetNextEvent ( &trk, &ev );
        }
        else                            // next event is after time_clk : set correct cur_time to t
        {

            double delta_t_ms = t - time_ms;
            double ms_per_clock = ( double )6000000.0 / ( state.cur_clock *    // see GoToTime()
                    ( double )tempo_scale * state.multitrack->GetClksPerBeat());
            state.cur_clock += ( MIDIClockTime )(delta_t_ms / ms_per_clock);
            state.cur_time_ms = time_ms;
            break;
        }
    }

        // examine all the events at this specific time
        // and update the track states to reflect this time
    ScanEventsAtThisTime();

        // re-enable the gui notifier if it was enabled previously
    if( state.notifier )
    {
        state.notifier->SetEnable( notifier_mode );
        // cause a full gui refresh now
        state.Notify( MIDISequencerGUIEvent::GROUP_ALL );
    }
    return ret;
}



/* NEW BY NC
bool MIDISequencer::GoToTime( MIDIClockTime time_clk ) {
    bool ret = true;

    // temporarily disable the gui notifier
    bool notifier_mode = false;

    if ( state.notifier )
    {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable ( false );
    }

    if ( time_clk < state.cur_clock || time_clk == 0 )
    {
        // start from zero if desired time is before where we are
        state.Reset();
    }

    MIDIClockTime t;
    int trk;
    MIDITimedBigMessage ev;

    while ( state.cur_clock < time_clk )
    {
        if ( !GetNextEventTime( &t  ))  // no other events: we can't reach time_clk and return false
        {
            ret = false;
            break;
        }
        if ( t < time_clk )             // next event is before time_clk
        {
            GetNextEvent ( &trk, &ev ); // get it and continue
        }
        else if (t == time_clk)         // next event is at right time
        {
            MIDIMultiTrackIteratorState istate(state.iterator.GetState());  // save the state of the iterator
            GetNextEvent( &trk, &ev );          // get the event
            state.iterator.SetState( istate );  // restore the iterator state, so ev is the next event
        }
        else                            // next event is after time_clk : set cur_time to time_clk
        {                               // and update cur_time_ms
            MIDIClockTime delta_time = time_clk - state.cur_clock;
            // calculate delta_time in milliseconds: this comes from
            //  -true_bpm = tempobpm * tempo_scale / 100
            //  -clocks_per_sec = true_bpm * clks_per_beat / 60
            //  -clocks_per_ms = clocks_per_sec / 1000
            //  -ms_per_clock = 1 / clocks_per_ms
            float ms_per_clock = ( double )6000000.0 / (state.tempobpm *
                                ( double )tempo_scale * state.multitrack->GetClksPerBeat());
            state.cur_clock = time_clk;
            state.cur_time_ms += ( ms_per_clock * delta_time );
        }
    }

        // examine all the events at this specific time
        // and update the track states to reflect this time

    ScanEventsAtThisTime();

        // re-enable the gui notifier if it was enabled previously
    if( state.notifier )
    {
        state.notifier->SetEnable( notifier_mode );
        // cause a full gui refresh now
        state.Notify( MIDISequencerGUIEvent::GROUP_ALL );
    }
    return ret;
}


bool MIDISequencer::GoToTimeMs( float time_ms ) {
    bool ret = true;

    // temporarily disable the gui notifier
    bool notifier_mode = false;

    if ( state.notifier )
    {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable( false );
    }

    if ( time_ms < state.cur_time_ms || time_ms==0.0 ) {
        // start from zero if desired time is before where we are
        state.Reset();
    }

    double t;
    int trk;
    MIDITimedBigMessage ev;

    while ( state.cur_time_ms < time_ms )
    {
        if ( !GetNextEventTimeMs( &t ) )    // no other events: we can't reach time_clk and return false
        {
            ret = false;
            break;
        }
        if ( t  < time_ms ) // next event is before or at right time
        {
            GetNextEvent ( &trk, &ev ); // get it and continue
        }
        else if ( abs (t-time_ms) < 0.001 )     // next event is at right time
        {
            MIDIMultiTrackIteratorState istate(state.iterator.GetState());  // save the state of the iterator
            GetNextEvent( &trk, &ev );          // get the event
            state.iterator.SetState( istate );  // restore the iterator state, so ev is the next event
        }
        else                            // next event is after time_ms : set correct cur_time to t
        {

            double delta_t_ms = t - time_ms;
            double ms_per_clock = ( double )6000000.0 / ( state.cur_clock *    // see GoToTime()
                    ( double )tempo_scale * state.multitrack->GetClksPerBeat());
            state.cur_clock += ( MIDIClockTime )(delta_t_ms / ms_per_clock);
            state.cur_time_ms = time_ms;
            break;
        }
    }

        // examine all the events at this specific time
        // and update the track states to reflect this time
    ScanEventsAtThisTime();

        // re-enable the gui notifier if it was enabled previously
    if( state.notifier )
    {
        state.notifier->SetEnable( notifier_mode );
        // cause a full gui refresh now
        state.Notify( MIDISequencerGUIEvent::GROUP_ALL );
    }
    return ret;
}




/* NOTE BY NC: this is simpler because every measure has as first event a beat event!

bool MIDISequencer::GoToMeasure ( int measure, int beat )
{
    // temporarily disable the gui notifier
    bool notifier_mode = false;

    if ( state.notifier )
    {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable ( false );
    }

    if ( measure < state.cur_measure ||
        /* NC  // ADDED FOLLOWING LINE:  this failed in this case!!!
         ( measure == state.cur_measure && beat < state.cur_beat ) ||
         ( measure == 0 && beat == 0 ) )
    {

        state.Reset();                      /* NC
    }

    MIDIClockTime t = 0;
    int trk;
    MIDITimedBigMessage ev;
    // iterate thru all the events until cur-measure and cur_beat are
    // where we want them.

    if ( !(measure == 0 && beat == 0) )
    {
        while (
            GetNextEventTime ( &t )
            && GetNextEvent ( &trk, &ev )
            && state.cur_measure <= measure
        )
        {
            if ( state.cur_measure == measure && state.cur_beat >= beat )
            {
                break;
            }
        }
    }
    // examine all the events at this specific time
    // and update the track states to reflect this time
    ScanEventsAtThisTime();

    // re-enable the gui notifier if it was enabled previously
    if ( state.notifier )
    {
        state.notifier->SetEnable ( notifier_mode );
        // cause a full gui refresh now
        state.Notify ( MIDISequencerGUIEvent::GROUP_ALL );
    }

    // return true if we actually found the measure requested
    return state.cur_measure == measure && state.cur_beat == beat;
}


bool MIDISequencer::GetNextEventTimeMs ( double *t )
{
    MIDIClockTime ct;
    bool f = GetNextEventTime ( &ct );

    if ( f )
    {
        // calculate delta time from last event time
        double delta_clocks = ( double ) ( ct - state.cur_clock );
        // calculate tempo in milliseconds per clock
        //double clocks_per_sec = ( ( state.track_state[0]->tempobpm *    /* OLD
		double clocks_per_sec = ( ( state.tempobpm *            /* NC
                                    ( ( ( double ) tempo_scale ) * 0.01 )
                                    * ( 1. / 60. ) ) * state.multitrack->GetClksPerBeat() );

        if ( clocks_per_sec > 0. )
        {
            double ms_per_clock = 1000. / clocks_per_sec;
            // calculate delta time in milliseconds
            double delta_ms = delta_clocks * ms_per_clock;
            // return it added with the current time in ms.
            *t = delta_ms + state.cur_time_ms;
        }

        else
        {
            f = false;
        }
    }

    return f;
}


bool MIDISequencer::GetNextEventTimeMs ( float *t )
{
    double t2;
    bool res = GetNextEventTimeMs ( &t2 );
    *t = ( float ) t2;
    return res;
}





/* NEW by NC
bool MIDISequencer::GetNextEvent ( int *tracknum, MIDITimedBigMessage *msg )
{
    MIDIClockTime t;

    // ask the iterator for the current event time
    if ( state.iterator.GetCurEventTime ( &t ) )
    {
        // move current time forward one event
        MIDIClockTime new_clock;
        float new_time_ms = 0.0f;
        GetNextEventTime ( &new_clock );
        GetNextEventTimeMs ( &new_time_ms );
        // must set cur_clock AFTER GetnextEventTimeMs() is called
        // since GetNextEventTimeMs() uses cur_clock to calculate
        state.cur_clock = new_clock;
        state.cur_time_ms = new_time_ms;

        // is the next beat marker before this event?
        if ( state.next_beat_time <= t )
        {
            // yes, this is a beat event now.
            // say this event came on track 0, the conductor track
            *tracknum = state.last_event_track = 0;
            // put current info into beat marker message
            beat_marker_msg.SetBeatMarker();
            beat_marker_msg.SetTime ( state.next_beat_time );
            *msg = beat_marker_msg;
            state.Process( msg );
        }

        else // this event comes before the next beat
        {
            const MIDITimedBigMessage *msg_ptr;

            if ( state.iterator.GetCurEvent ( tracknum, &msg_ptr ) )
            {
                int trk = state.last_event_track = *tracknum;
                // copy the event so Process can modify it
                *msg = *msg_ptr;
                bool allow_msg = true;
                // are we in solo mode?

                if ( solo_mode )
                {
                    // yes, only allow this message thru if
                    // the track is either track 0
                    // or it is explicitly solod.
                    if ( trk == 0 || track_processors[trk]->solo )
                    {
                        allow_msg = true;
                    }

                    else
                    {
                        allow_msg = false;
                    }
                }

                if ( ! ( allow_msg
                         && track_processors[trk]->Process ( msg )
                         && state.Process ( msg ) )
                   )
                {
                    // the message is not allowed to come out!
                    // erase it
                    msg->SetNoOp();
                }

                // go to the next event on the multitrack
                state.iterator.GoToNextEvent();
            }
        }
        return true;
    }

    return false;
}


double MIDISequencer::GetMisicDurationInSeconds()
{
    double event_time = 0.; // in milliseconds

    MIDITimedBigMessage ev;
    int ev_track;

    GoToZero();

    while ( GetNextEvent( &ev_track, &ev ) )
    {
        // std::cout << EventAsText( ev ) << std::endl;

        // skip these events
        if ( ev.IsEndOfTrack() || ev.IsBeatMarker() )
            continue;

        // end of music is the time of last not end of track midi event!
        event_time = GetCurrentTimeInMs();
    }

    return ( 0.001 * event_time );
}


void MIDISequencer::ScanEventsAtThisTime()
{
    // save the current iterator state
    MIDIMultiTrackIteratorState istate ( state.iterator.GetState() );
    int prev_measure = state.cur_measure;
    int prev_beat = state.cur_beat;
    // process all messages up to and including this time only
    MIDIClockTime orig_clock = state.cur_clock;
    double orig_time_ms = state.cur_time_ms;
    MIDIClockTime t = 0;
    int trk;
    MIDITimedBigMessage ev;

    while (
        GetNextEventTime ( &t )
        && t == orig_clock
        && GetNextEvent ( &trk, &ev )
    )
    {
        ;
    }

    // restore the iterator state
    state.iterator.SetState ( istate );
    // and current time
    state.cur_clock = orig_clock;
    state.cur_time_ms = float ( orig_time_ms );
    state.cur_measure = prev_measure;
    state.cur_beat = prev_beat;

    if ( state.cur_clock == state.last_beat_time )
    {
        state.next_beat_time = state.cur_clock;
    }
}


}
*/
