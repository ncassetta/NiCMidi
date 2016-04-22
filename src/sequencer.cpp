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


bool MIDISequencerTrackProcessor::Process( MIDITimedBigMessage *msg ) {
    // are we muted?
    if( mute )
        // yes, ignore event.
        return false;

    // is the event a NoOp?
    if( msg->IsNoOp() )
        // yes, ignore event.
        return false;

    // pass the event to our extra_proc if we have one
    if( extra_proc && extra_proc->Process(msg)==false )
        // extra_proc wanted to ignore this event
        return false;

    // is it a normal MIDI channel message?
    if( msg->IsChannelMsg() ) {
        // yes, are we to re-channel it?
        if( rechannel!=-1 )
            msg->SetChannel( (unsigned char)rechannel );

        // is it a note on message?
        if( msg->IsNoteOn() && msg->GetVelocity() > 0 ) {
            // yes, scale the velocity value as required
            int vel = (int)msg->GetVelocity();
            vel = vel*velocity_scale / 100;

            // make sure velocity is never less than 0
            if( vel < 0 )
                vel = 0;

            // rewrite the velocity
            msg->SetVelocity( (unsigned char)vel );
        }

        // is it a type of event that needs to be transposed?
        if( msg->IsNoteOn() || msg->IsNoteOff() || msg->IsPolyPressure() ) {
            int new_note = ((int)msg->GetNote()) + transpose;
            if( new_note >= 0 && new_note <= 127 )

                // set new note number
                msg->SetNote( (unsigned char)new_note );
            else
                // otherwise delete this note - transposed value is out of range
                return false;
        }
    }
    return true;
}


////////////////////////////////////////////////////////////////////////////


MIDISequencerTrackState::MIDISequencerTrackState( MIDISequencer *seq_, int trk,
        MIDISequencerGUIEventNotifier *n) :

    MIDISequencerTrackNotifier( seq_, trk, n ),
    pg(-1), volume(100), pan(64), rev(0), chr(0),
    bender_value(0), got_good_track_name(false), notes_are_on(false), note_matrix() {
    *track_name = 0;
}


void MIDISequencerTrackState::GoToZero() {
    pg = -1;
    volume = 100;
    pan = 64;
    rev = 0;
    chr = 0;
    notes_are_on = false;
    bender_value = 0;
    note_matrix.Clear();
}


void MIDISequencerTrackState::Reset() {
    GoToZero();
    *track_name = 0;
    got_good_track_name = false;
}


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

////////////////////////////////////////////////////////////////////////////


MIDISequencerState::MIDISequencerState( MIDISequencer *s, MIDIMultiTrack *m,
    MIDISequencerGUIEventNotifier *n ) :
    MIDISequencerTrackNotifier(s, 0, n),
    notifier(n), multitrack(m), iterator( m ),
    cur_clock(0), cur_time_ms(0), cur_beat(0), cur_measure(0), next_beat_time(0),
    tempobpm(120.0), timesig_numerator(4), timesig_denominator(4),
    keysig_sharpflat(0), keysig_mode(0)
{
    *markertext = 0;
    markertext[49] = 0;     // string terminator
    for( int i=0; i<DEFAULT_MAX_NUM_TRACKS; ++i )
        track_state[i] = new MIDISequencerTrackState( s, i, notifier );
}


MIDISequencerState::MIDISequencerState( const MIDISequencerState &s ) :
    MIDISequencerTrackNotifier(s.seq, 0, s.notifier),
    notifier( s.notifier ), multitrack( s.multitrack ),
    iterator( s.iterator ), cur_clock( s.cur_clock ), cur_time_ms( s.cur_time_ms ),
    cur_beat( s.cur_beat ), cur_measure( s.cur_measure ), next_beat_time(s.next_beat_time ),
    tempobpm( s.tempobpm ), timesig_numerator( s.timesig_numerator ),
    timesig_denominator( s.timesig_denominator), keysig_sharpflat( s.keysig_sharpflat ),
    keysig_mode(s. keysig_mode)
{
    strncpy(markertext, s.markertext, 49);
    for( int i=0; i<DEFAULT_MAX_NUM_TRACKS; ++i )
        track_state[i] = new MIDISequencerTrackState( *s.track_state[i] );
}


MIDISequencerState::~MIDISequencerState() {
    for( int i=0; i<DEFAULT_MAX_NUM_TRACKS; ++i )
        delete track_state[i];
}


const MIDISequencerState& MIDISequencerState::operator= ( const MIDISequencerState & s ) {
/* revised by me, so in CustomSequences::ExtractWarpPositions() we can copy sequencer states
   instead of deleting and creating continuosly
*/

    notifier = s.notifier;
    multitrack = s.multitrack;
    last_event_track = s.last_event_track;

    iterator.SetState(s.iterator.GetState());
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
    strncpy(markertext, s.markertext, 49);

    for( int i = 0; i < GetNumTracks(); ++i ) { // copies track states
        track_state[i]->SetNotifier(s.seq, i, notifier);
        track_state[i]->pg = s.track_state[i]->pg;
        track_state[i]->volume = s.track_state[i]->volume;
        track_state[i]->pan = s.track_state[i]->pan;
        track_state[i]->rev = s.track_state[i]->rev;
        track_state[i]->chr = s.track_state[i]->chr;
        track_state[i]->bender_value = s.track_state[i]->bender_value;
        strncpy(track_state[i]->track_name, s.track_state[i]->track_name, 256);
        track_state[i]->got_good_track_name = s.track_state[i]->got_good_track_name;
        track_state[i]->notes_are_on = s.track_state[i]->notes_are_on;
        memmove((void *)&(track_state[i]->note_matrix), (void *)&(s.track_state[i]->note_matrix),
                 sizeof(MIDIMatrix));
    }
    return *this;
}


void MIDISequencerState::Reset() {      // new : added by me
    iterator.GoToTime(0);
    iterator.GetState().SetNumTracks(multitrack->GetNumTracks());
    cur_clock = 0;
    cur_time_ms = 0.0;
    cur_beat = 0;
    cur_measure = 0;
    tempobpm = 120.0;
    timesig_numerator = 4;
    timesig_denominator = 4;
    keysig_sharpflat = 0;
    keysig_mode = 0;
    *markertext = 0;
    next_beat_time = multitrack->GetClksPerBeat();  // * 4 / timesig_denominator;   Redundant!
    for( int i=0; i<GetNumTracks(); ++i )
        track_state[i]->Reset();
}



bool MIDISequencerState::Process( MIDITimedBigMessage *msg ) {
    // is the event a NoOp?

    if( msg->IsNoOp() )
        return false;                   // ignore event.

    if( msg->IsChannelMsg() )           // is it a normal MIDI channel message?
        // give it to its MIDISequencerTrackState
        return track_state[last_event_track]->Process(msg);

    if( msg->IsMetaEvent() ) {     // is it a meta-event?

        if( msg->IsTempo() ) {          // is it a tempo event?
            tempobpm = ((float)msg->GetTempo32())*(1.0f/32.0f);
            if(tempobpm < 1 )
            tempobpm=120.0;
            NotifyConductor( MIDISequencerGUIEvent::GROUP_CONDUCTOR_TEMPO );
        }
        else if( msg->IsTimeSig() ) {   // is it a time signature event?
            timesig_numerator = msg->GetTimeSigNumerator();
            timesig_denominator = msg->GetTimeSigDenominator();
            NotifyConductor( MIDISequencerGUIEvent::GROUP_CONDUCTOR_TIMESIG );
        }
        else if( msg->IsKeySig() ) {    // is it a key signature event?
            keysig_sharpflat = msg->GetKeySigSharpFlats();
            keysig_mode = msg->GetKeySigMajorMinor();
            NotifyConductor( MIDISequencerGUIEvent::GROUP_CONDUCTOR_KEYSIG );
        }
        else if ( msg->IsTextMarker()) {
            strncpy(markertext, (const char *)msg->GetSysEx()->GetBuf(), 49);
            NotifyConductor( MIDISequencerGUIEvent::GROUP_CONDUCTOR_MARKER );
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
            Notify( MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT );

            // if the new beat number is 0 then the measure changed too
            if( cur_beat == 0 )
                Notify( MIDISequencerGUIEvent::GROUP_TRANSPORT_MEASURE );
        }
        else                               // could be a track name meta event
            return track_state[last_event_track]->Process(msg);
    }
    return true;
}



////////////////////////////////////////////////////////////////////////////


MIDISequencer::MIDISequencer( MIDIMultiTrack *m, MIDISequencerGUIEventNotifier *n ) :
    solo_mode(false),
    tempo_scale(100),
    state( this, m,n ) {    // TODO: fix this hack
    for( int i = 0; i < DEFAULT_MAX_NUM_TRACKS; ++i )
        track_processors[i] = new MIDISequencerTrackProcessor;
}


MIDISequencer::~MIDISequencer() {
    for(int i = 0; i < DEFAULT_MAX_NUM_TRACKS; ++i )
        delete track_processors[i];
}

void MIDISequencer::ResetTrack( int trk ) {
    state.track_state[trk]->Reset();
    track_processors[trk]->Reset();
}


void MIDISequencer::ResetAllTracks() {
    for( int i=0; i<GetNumTracks(); ++i ) {
        state.track_state[i]->Reset();
        track_processors[i]->Reset();
    }
}


void MIDISequencer::SetSoloMode( bool m, int trk ) {
    int i;
    solo_mode = m;

    for( i=0; i<GetNumTracks(); ++i ) {
        if( i==trk )
            track_processors[i]->solo = true;
        else
            track_processors[i]->solo = false;
    }
}


void MIDISequencer::GoToZero() {
    // go to time zero
    state.Reset();

    // examine all the events at this specific time
    // and update the track states to reflect this time
    ScanEventsAtThisTime();
}


bool MIDISequencer::GoToTime( MIDIClockTime time_clk ) {
    bool ret = true;

        // temporarily disable the gui notifier
    bool notifier_mode=false;
    if( state.notifier ) {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable(false);
    }

    if( time_clk < state.cur_clock || time_clk == 0 )
        // start from zero if desired time is before where we are
        state.Reset();

    MIDIClockTime t;
    int trk;
    MIDITimedBigMessage ev;

    while(1) {
        if (GetCurrentMIDIClockTime() == time_clk)
            break;                      // we are already at right time
        if (!GetNextEventTime( &t )) {
            ret = false;
            break;
        }
        if (t <= time_clk) {
            GetNextEvent(&trk, &ev);    // next event is before or at right time
            ev.Clear();                 // added by me: we always must delete eventual sysex pointers
            continue;                   // before reassigning to av
        }
        else {
            // next event is after time_clk : set cur_time to time_clk
            state.cur_clock = time_clk;
            state.cur_time_ms = MIDItoMs(time_clk);
            break;
        }
    }

        // examine all the events at this specific time
        // and update the track states to reflect this time

    if (ret) ScanEventsAtThisTime();

        // re-enable the gui notifier if it was enabled previously
    if( state.notifier ) {
        state.notifier->SetEnable( notifier_mode );
        // cause a full gui refresh now
        state.notifier->Notify( this, MIDISequencerGUIEvent::GROUP_ALL );
    }
    return ret;
}


bool MIDISequencer::GoToTimeMs( float time_ms ) {
    bool ret = true;

        // temporarily disable the gui notifier
    bool notifier_mode = false;
    if( state.notifier ) {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable(false);
    }

    if( time_ms < state.cur_time_ms || time_ms==0.0 )
        // start from zero if desired time is before where we are
        state.Reset();

    MIDIClockTime ct;
    int trk;
    MIDITimedBigMessage ev;

    while(1) {
        if (GetCurrentTimeInMs() == time_ms)
            break;                      // we are already at right time
        if (!GetNextEventTime( &ct )) {
            ret = false;
            break;
        }
        if (MIDItoMs( ct ) <= time_ms) {
            GetNextEvent(&trk, &ev);    // next event is before or at right time
            ev.Clear();                 // added by me: we always must delete eventual sysex pointers
            continue;                   // before reassigning to ev
        }
        else {
            // next event is after time_clk : set cur_time to time_clk
            double ms_per_clock = (double)6000000.0 / (state.cur_clock *    // see MIDItoMs()
                    (double)tempo_scale * state.multitrack->GetClksPerBeat());
            MIDIClockTime clocks_needed = (MIDIClockTime)((ct - state.cur_clock) *
                    ms_per_clock);
            state.cur_clock += clocks_needed;
            state.cur_time_ms = time_ms;
            break;
        }
    }

        // examine all the events at this specific time
        // and update the track states to reflect this time

	if (ret) ScanEventsAtThisTime();

        // re-enable the gui notifier if it was enabled previously
    if( state.notifier ) {
        state.notifier->SetEnable( notifier_mode );
        // cause a full gui refresh now
        state.notifier->Notify( this, MIDISequencerGUIEvent::GROUP_ALL );
    }
    return ret;
}


bool MIDISequencer::GoToMeasure( int measure, int beat ) {
    bool ret = true;

        // temporarily disable the gui notifier
    bool notifier_mode = false;
    if( state.notifier ) {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable(false);
    }

    if( measure < state.cur_measure || measure == 0 )
        // start from zero if desired time is before where we are
        state.Reset();


    int trk;
    MIDITimedBigMessage ev;

        // iterate thru all the events until cur-measure and cur_beat are
        // where we want them.
    if (measure > 0 || beat > 0) {          // if meas == 0 && beat == 0 nothing to do
        while( 1 ) {
            if (!GetNextEvent(&trk, &ev)) {
                ret = false;
                break;
            }
            if (ev.IsBeatMarker()) {        // there must be a beat marker at right time
                if( state.cur_measure == measure && state.cur_beat >= beat )
                    break;
            }
            ev.Clear();                 // added by me: we always must delete eventual sysex pointers
                                        // before reassigning to av
        }
    }

        // examine all the events at this specific time
        // and update the track states to reflect this time
    if (ret) ScanEventsAtThisTime();

        // re-enable the gui notifier if it was enabled previously
    if( state.notifier ) {
        state.notifier->SetEnable( notifier_mode );
        // cause a full gui refresh now
        state.notifier->Notify( this, MIDISequencerGUIEvent::GROUP_ALL );
    }

        // return true if we actually found the measure requested
    return ret;
}


bool MIDISequencer::GetNextEventTimeMs( float *t ) {
    MIDIClockTime ct;
    bool f = GetNextEventTime( &ct );

    if( f )
        *t = MIDItoMs( ct );
    return f;
}


bool MIDISequencer::GetNextEventTime( MIDIClockTime *t ) {
    // ask the iterator for the current event time
    bool f = state.iterator.GetCurEventTime(t);

    if( f ) {
        // if we have an event in the future, check to see if it is
        // further in time than the next beat marker

        if( (*t) >= state.next_beat_time )
            // ok, the next event is a beat - return the next beat time
            *t = state.next_beat_time;
    }
    return f;
}


bool MIDISequencer::GetNextEvent( int *tracknum, MIDITimedBigMessage *msg ) {
    MIDIClockTime t;

    // ask the iterator for the current event time
    if( state.iterator.GetCurEventTime(&t) ) {
        // move current time forward one event
        MIDIClockTime new_clock;
        float new_time_ms;
        GetNextEventTime( &new_clock );
        GetNextEventTimeMs( &new_time_ms );

        // must set cur_clock AFTER GetnextEventTimeMs() is called
        // since GetNextEventTimeMs() uses cur_clock to calculate

        state.cur_clock = new_clock;
        state.cur_time_ms = new_time_ms;

        // is the next beat marker before this event?
        if( state.next_beat_time <= t ) {
            // yes, this is a beat event now.
            // say this event came on track 0, the conductor track
            state.last_event_track = *tracknum = 0;

            // put current info into beat marker message
            beat_marker_msg.SetBeatMarker();
            beat_marker_msg.SetTime( state.next_beat_time );
            *msg = beat_marker_msg;

            state.Process(msg);
        }
        else    {   // this event comes before the next beat
            MIDITimedBigMessage *msg_ptr;

            if( state.iterator.GetCurEvent( tracknum, &msg_ptr ) ) {
                int trk = state.last_event_track = *tracknum;

                // copy the event so Process can modify it
                *msg = *msg_ptr;
                 bool allow_msg = true;

                // are we in solo mode?
                if( solo_mode ) {
                    // yes, only allow this message thru if
                    // the track is either track 0
                    // or it is explicitly solod.
                    allow_msg = ( trk == 0 || track_processors[trk]->solo ) ? true : false;
                }
                if( !(allow_msg
                    && track_processors[trk]->Process(msg)
                    && state.Process(msg))) {
                    // the message is not allowed to come out!
                    // erase it
                    msg->Clear();
                    // delete eventual sysex
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
    MIDITimedBigMessage ev;

    while( GetNextEventTime( &t ) && t==orig_clock && GetNextEvent(&trk,&ev) ) {
        ev.Clear();  // added by me: we always must delete eventual sysex pointers before reassigning to av
    }

    // restore the iterator state
    state.iterator.SetState( istate );

    // and current time
    state.cur_clock = orig_clock;
    state.cur_time_ms = float(orig_time_ms);

    state.cur_measure=prev_measure;
    state.cur_beat = prev_beat;

}


float MIDISequencer::MIDItoMs(MIDIClockTime t) {
    MIDITimedBigMessage* msg;
    MIDITrackIterator tr_iter(state.multitrack->GetTrack(0));
    MIDIClockTime base_t = 0, delta_t = 0, now_t = 0;
    double ms_time = 0.0, oldtempo = 120.0, ms_per_clock;

    while (now_t < t) {
        if (!tr_iter.GetCurEvent(&msg, t))      // next message is after t or doesn't exists
            now_t = t;
        else
            now_t = msg->GetTime();
        if (msg->IsTempo() || now_t == t) {
            // delta time in MIDI clocks
            delta_t = now_t - base_t;

            // calculate delta time in milliseconds: this comes from
            //  -true_bpm = oldtempo * tempo_scale / 100
            //  -clocks_per_sec = true_bpm * clks_per_beat / 60
            //  -clocks_per_ms = clocks_per_sec / 1000
            //  -ms_per_clock = 1 / clocks_per_ms
            ms_per_clock = (double)6000000.0 / (oldtempo *
                                (double)tempo_scale * state.multitrack->GetClksPerBeat());

            // and add it to ms_time
            ms_time += (delta_t * ms_per_clock);

            // update variables
            base_t = msg->GetTime();
            if (msg->IsTempo())
                oldtempo = msg->GetTempo32() / 32.0;
            if (now_t == t) break;
        }
    }
    return (float)ms_time;
}

