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


#include "../include/sequencer.h"
#include "../include/manager.h"     // goes here, for SetPort()





////////////////////////////////////////////////////////////////////////////
//              class MIDISequencerTrackState                             //
////////////////////////////////////////////////////////////////////////////


MIDISequencerTrackState::MIDISequencerTrackState() {
    Reset();
}


void MIDISequencerTrackState::Reset() {
    program = -1;
    for (unsigned int i = 0; i < C_ALL_NOTES_OFF; i++)
        control_values[i] = -1;
    track_name = "";
    notes_are_on = false;
    bender_value = 0;
    note_matrix.Clear();
    got_good_track_name = false;
}


////////////////////////////////////////////////////////////////////////////
//                      class MIDISequencerState                          //
////////////////////////////////////////////////////////////////////////////


int MIDISequencerState::metronome_mode = MIDISequencer::FOLLOW_MIDI_TIMESIG_MESSAGE;

MIDISequencerState::MIDISequencerState(MIDIMultiTrack *m, MIDISequencerGUINotifier *n) :
    notifier(n), multitrack(m), iterator(m)
{
    Reset();
}


MIDISequencerState::~MIDISequencerState() {
    for (unsigned int i = 0; i < track_states.size(); i++)
        delete track_states[i];
}


MIDISequencerState::MIDISequencerState(const MIDISequencerState& s) :
    notifier(s.notifier), multitrack(s.multitrack), iterator(s.iterator),
    cur_clock(s.cur_clock), cur_time_ms(s.cur_time_ms), cur_beat(s.cur_beat),
    cur_measure(s.cur_measure), beat_length(s.beat_length), next_beat_time(s.next_beat_time),
    tempobpm(s.tempobpm), timesig_numerator(s.timesig_numerator),
    timesig_denominator(s.timesig_denominator), keysig_sharpflat(s.keysig_sharpflat),
    keysig_mode(s. keysig_mode), marker_text(s.marker_text), last_event_track(s.last_event_track),
    last_beat_time(s.last_beat_time)
{
    track_states.resize(multitrack->GetNumTracks());
    for (unsigned int i = 0; i < track_states.size(); i++)
        track_states[i] = new MIDISequencerTrackState(*s.track_states[i]);
}


const MIDISequencerState& MIDISequencerState::operator= (const MIDISequencerState& s) {
    notifier = s.notifier;
    multitrack = s.multitrack;
    iterator.SetState(s.iterator.GetState());
    cur_clock = s.cur_clock;
    cur_time_ms = s.cur_time_ms;
    cur_beat = s.cur_beat;
    cur_measure = s.cur_measure;
    beat_length = s.beat_length;
    next_beat_time = s.next_beat_time;
    tempobpm = s.tempobpm;
    timesig_numerator = s.timesig_numerator;
    timesig_denominator = s.timesig_denominator;
    keysig_sharpflat = s.keysig_sharpflat;
    keysig_mode = s.keysig_mode;
    marker_text = s.marker_text;

    for (unsigned int i = 0; i < track_states.size(); i++)
        delete track_states[i];
    track_states.resize(multitrack->GetNumTracks());
    for (unsigned int i = 0; i < track_states.size(); i++)
        track_states[i] = new MIDISequencerTrackState(*s.track_states[i]);
    last_event_track = s.last_event_track;
    last_beat_time = s.last_beat_time;

    return *this;
}


void MIDISequencerState::Reset() {
    iterator.Reset();
    cur_clock = 0;
    cur_time_ms = 0.0;
    cur_beat = 0;
    cur_measure = 0;
    beat_length = next_beat_time = multitrack->GetClksPerBeat();
    tempobpm = MIDI_DEFAULT_TEMPO;
    timesig_numerator = MIDI_DEFAULT_TIMESIG_NUMERATOR;
    timesig_denominator = MIDI_DEFAULT_TIMESIG_DENOMINATOR;
    keysig_sharpflat = MIDI_DEFAULT_KEYSIG_KEY;
    keysig_mode = MIDI_DEFAULT_KEYSIG_MODE;
    marker_text = "";
    if (multitrack->GetNumTracks() != track_states.size()) {
        for (unsigned int i = 0; i < track_states.size(); i++)
            delete track_states[i];
        track_states.resize(multitrack->GetNumTracks());
        for (unsigned int i = 0; i < track_states.size(); i++)
            track_states[i] = new MIDISequencerTrackState;
    }
    else
        for (unsigned int i = 0; i < track_states.size(); i++)
            track_states[i]->Reset();
    last_event_track = -1;
    last_beat_time = 0;
}


bool MIDISequencerState::Process( MIDITimedMessage *msg ) {
    MIDISequencerTrackState* const t_state = track_states[last_event_track];

    // is the event a NoOp?
    if(msg->IsNoOp())
        return false;                           // ignore event.

    // is the event a beat marker?
    else if (msg->IsBeatMarker()) {
        // update our beat count
        int new_beat = cur_beat;
        int new_measure = cur_measure;
        if ( last_beat_time != next_beat_time ) {               // if at start, we must not upgrade beat count
            // upgrade beat number
            new_beat++;
            int number_of_beats = multitrack->GetClksPerBeat() *// quarter length in MIDI ticks
                                  timesig_numerator *           // number of symbolic beats
                                  4 / timesig_denominator /     // length of a symbolic beat in quarters
                                  beat_length;                  // beat length in MIDI ticks
            if(new_beat >= number_of_beats) {
                // upgrade measure number
                new_beat = 0;
                ++new_measure;
            }
        }

        // set the status parameters
        last_beat_time = cur_clock;
        next_beat_time += beat_length;
        cur_beat = new_beat;
        cur_measure = new_measure;

        // now notify the GUI that the beat number (and eventually the measure) is changed
        if(cur_beat == 0)
            Notify(MIDISequencerGUIEvent::GROUP_TRANSPORT,
                   MIDISequencerGUIEvent::GROUP_TRANSPORT_MEASURE);
            Notify(MIDISequencerGUIEvent::GROUP_TRANSPORT,
               MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT);
    }

    // is the event a MIDI channel message?
    else if(msg->IsChannelMsg()) {
        if( msg->GetType()==PITCH_BEND )        // is it a bender event?
            // yes, remember the bender wheel value
             t_state->bender_value = msg->GetBenderValue();
        else if( msg->IsControlChange() ) {     // is it a control change event?
            // don't monitor system channel messages
            if (msg->GetController() < C_ALL_NOTES_OFF) {
                t_state->control_values[msg->GetController()] = msg->GetControllerValue();
                // notify event
                if(msg->GetController() == C_MAIN_VOLUME)
                    NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_VOLUME);
                else if(msg->GetController() == C_PAN)
                    NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_PAN);
                else if(msg->GetController() == C_CHORUS_DEPTH)
                    NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_CHR);
                else if( msg->GetController() == C_EFFECT_DEPTH )
                    NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_REV);
            }
        }
        else if(msg->IsProgramChange()) {       // is it a program change event?
            // yes, update the current program change value
            t_state->program = msg->GetProgramValue();
            NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_PROGRAM);
        }
        // pass the message to our note matrix to keep track of all notes on
        // on this track
        if(t_state->note_matrix.Process(*msg)) {
            // did the "any notes on" status change?
            if((t_state->notes_are_on && t_state->note_matrix.GetTotalCount() == 0) ||
               (!t_state->notes_are_on && t_state->note_matrix.GetTotalCount() > 0) ) {
                // yes, toggle our notes_are_on flag
                t_state->notes_are_on = !(t_state->notes_are_on);
                // and notify the gui about the activity on this track
                NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_NOTE);
            }
        }
        return true;
    }

    // is the event a meta-event?
    else if(msg->IsMetaEvent()) {
        if(msg->IsTempo()) {                    // is it a tempo event?
            tempobpm = msg->GetTempo();
            //if(tempobpm < 1 )
            //tempobpm=MIDI_DEFAULT_TEMPO;
            Notify(MIDISequencerGUIEvent::GROUP_CONDUCTOR,
                   MIDISequencerGUIEvent::GROUP_CONDUCTOR_TEMPO);
        }
        else if(msg->IsTimeSig()) {             // is it a time signature event?
            // set internal prameters for the timesig
            timesig_numerator = msg->GetTimeSigNumerator();
            timesig_denominator = msg->GetTimeSigDenominator();
            MIDIClockTime old_beat_length = beat_length;
            // now set the metronome beat length
            if (metronome_mode == MIDISequencer::FOLLOW_MIDI_TIMESIG_MESSAGE)
                // in this mode the metronome length follows the MIDI message setting
                beat_length = msg->GetSysEx()->GetData(2) * multitrack->GetClksPerBeat() / 24;
            else {
                // in this mode the metronome beat follows the timesig denominator
                beat_length = multitrack->GetClksPerBeat() * 4 / timesig_denominator;
                if (metronome_mode == MIDISequencer::FOLLOW_THEORETIC_VALUE && !(timesig_numerator % 3) &&
                    timesig_numerator != 3)
                    // in this mode the metronome is multiplied by 3 if this i s a compound time
                    beat_length *= 3;
            }
            // adjust the next beat time (this was set according to the old value)
            next_beat_time = next_beat_time + beat_length - old_beat_length;

            // notify the GUI of the timesig change
            Notify(MIDISequencerGUIEvent::GROUP_CONDUCTOR,
                   MIDISequencerGUIEvent::GROUP_CONDUCTOR_TIMESIG);
        }
        else if(msg->IsKeySig()) {              // is it a key signature event?
            keysig_sharpflat = msg->GetKeySigSharpsFlats();
            keysig_mode = msg->GetKeySigMode();
            Notify(MIDISequencerGUIEvent::GROUP_CONDUCTOR,
                   MIDISequencerGUIEvent::GROUP_CONDUCTOR_KEYSIG);
        }
        else if ( msg->IsMarkerText()) {        // is it a marker event?
            marker_text = std::string((const char *)msg->GetSysEx()->GetBuffer(), msg->GetSysEx()->GetLength());
            Notify(MIDISequencerGUIEvent::GROUP_CONDUCTOR,
                   MIDISequencerGUIEvent::GROUP_CONDUCTOR_MARKER);
        }
        else if( ( msg->GetMetaType()==META_TRACK_NAME
                 || msg->GetMetaType()==META_INSTRUMENT_NAME
                 || (!t_state->got_good_track_name && msg->GetMetaType()==META_GENERIC_TEXT && msg->GetTime()==0) )
                 && msg->GetSysEx() ) {           // is it a track name event?
            t_state->got_good_track_name = true;
            int len = msg->GetSysEx()->GetLength();
            t_state->track_name = std::string((const char *)msg->GetSysEx()->GetBuffer(), len);
            NotifyTrack(MIDISequencerGUIEvent::GROUP_TRACK_NAME);
        }
        return true;
    }
    return false;
}


void MIDISequencerState::Notify(int group, int item) const {
    if (notifier) {
        MIDISequencerGUIEvent ev(group, 0, item);
        notifier->Notify(ev);
    }
}


void MIDISequencerState::NotifyTrack(int item) const {
    if (notifier) {
        MIDISequencerGUIEvent ev(MIDISequencerGUIEvent::GROUP_TRACK,
                                last_event_track,
                                item);
        notifier->Notify(ev);
    }
}



////////////////////////////////////////////////////////////////////////////
//                        class MIDISequencer                             //
////////////////////////////////////////////////////////////////////////////


MIDISequencer::MIDISequencer (MIDIMultiTrack *m, MIDISequencerGUINotifier *n) :
    MIDITickComponent(PR_SEQ, StaticTickProc),
    tempo_scale (100), repeat_play_mode(false),
    repeat_start_meas(0), repeat_end_meas(0),
    track_processors(m->GetNumTracks(), 0),
    time_shifts(m->GetNumTracks(), 0),
    track_ports(m->GetNumTracks(), 0),
    state (m, n) {
    if (n)
        n->SetSequencer(this);
    beat_marker_msg.SetBeatMarker();
    state.iterator.GetState().SetTimeShiftMode(false, &time_shifts);  // surely returns true
}


MIDISequencer::~MIDISequencer() {
    Stop();
    for (unsigned int i = 0; i < track_processors.size(); ++i)
        if (track_processors[i])
            delete track_processors[i];
}


void MIDISequencer::Reset() {
    Stop();
    state.Reset();              // syncronize the multitrack with the state
    for (unsigned int i = 0; i < track_processors.size(); ++i)
        if (track_processors[i])
            delete track_processors[i];
    if (track_processors.size() != GetNumTracks()) {
        track_processors.resize(GetNumTracks());
        time_shifts.resize(GetNumTracks());
        track_ports.resize(GetNumTracks());
    }
    for (unsigned int i = 0; i < GetNumTracks(); ++i) {
        track_processors[i] = 0;
        time_shifts[i] = 0;
        track_ports[i] = 0;
    }
    tempo_scale = 100;
    SetTimeShiftMode(false);
    bool notifier_mode = false;
    if(state.notifier) {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable(false);
    }
    // examine all the events at this specific time
    // and update the track states to reflect this time
    ScanEventsAtThisTime();     // we can't call this before we have syncronized the track processors

    // re-enable the gui notifier if it was enabled previously
    if(state.notifier) {
        state.notifier->SetEnable(notifier_mode);
        // cause a full gui refresh now
        state.notifier->Notify(MIDISequencerGUIEvent::GROUP_ALL);
    }
}


MIDIClockTime MIDISequencer::GetCurrentMIDIClockTime() const {
    MIDIClockTime time = state.cur_clock;
    if (IsPlaying()) {
        float ms_offset = GetCurrentTimeMs() - state.cur_time_ms;
        float ms_per_clock = 60000.0 / (GetTempoWithScale() * state.multitrack->GetClksPerBeat());
        time += (MIDIClockTime)(ms_offset / ms_per_clock);
    }
    return time;
}


float MIDISequencer::GetCurrentTimeMs() const {
    return IsPlaying() ?
        MIDITimer::GetSysTimeMs() - sys_time_offset + dev_time_offset :
        state.cur_time_ms;
}

/* OLD WITHOUT MUTEX
void MIDISequencer::SetRepeatPlay(int on_off, int start_meas, int end_meas) {
    // shut off repeat play while we deal with values
    bool old_repeat = repeat_play_mode;
    repeat_play_mode = false;
    // change start and end measures if needed
    if (start_meas != -1)
        repeat_start_meas = (unsigned)start_meas;
    if (end_meas != -1)
        repeat_end_meas = (unsigned)end_meas;
    if (repeat_start_meas == repeat_end_meas)
        repeat_play_mode = false;
    else if (on_off == -1)
        repeat_play_mode = old_repeat;
    else
        repeat_play_mode = (bool)on_off;
}
*/

void MIDISequencer::SetRepeatPlay(int on_off, int start_meas, int end_meas) {
    proc_lock.lock();
    // change start and end measures if needed
    if (start_meas != -1)
        repeat_start_meas = (unsigned)start_meas;
    if (end_meas != -1)
        repeat_end_meas = (unsigned)end_meas;
    if (repeat_start_meas == repeat_end_meas)
        repeat_play_mode = false;
    else if (on_off != -1)
        repeat_play_mode = (bool)on_off;
    proc_lock.unlock();
}


void MIDISequencer::SetTempoScale(unsigned int scale) {
    proc_lock.lock();
    tempo_scale = scale;
    if (IsPlaying()) {
        dev_time_offset = state.cur_time_ms = MIDItoMs(state.cur_clock);
        sys_time_offset = MIDITimer::GetSysTimeMs();
    }
    proc_lock.unlock();
}



void MIDISequencer::SetTrackTimeShift(unsigned int trk, int offset) {
    if (!state.multitrack->IsValidTrackNumber(trk))
        return;
    char channel = state.multitrack->GetTrack(trk)->GetChannel();
    proc_lock.lock();

    if (IsPlaying() && channel != -1) {
        MIDIManager::GetOutDriver(GetTrackPort(trk))->AllNotesOff(channel);
        GetTrackState(trk)->note_matrix.Clear();
    }
    time_shifts[trk] = offset;
    //GoToTime(GetCurrentMIDIClockTime());      // TODO: this should sync the iterator, but provokes a AllNotesOff
                                                // on all channels, so I eliminated it. Is this correct?
    proc_lock.unlock();
}


void MIDISequencer::SetTrackOutPort(unsigned int trk, unsigned int port) {
    if (!state.multitrack->IsValidTrackNumber(trk))
        return;
    char channel = state.multitrack->GetTrack(trk)->GetChannel();
    proc_lock.lock();
    if (IsPlaying() && port != GetTrackPort(trk) && channel != -1) {
        MIDIManager::GetOutDriver(GetTrackPort(trk))->AllNotesOff(channel);
        GetTrackState(trk)->note_matrix.Clear();
    }
    port %= MIDIManager::GetNumMIDIOuts();
    track_ports[trk] = port;
    proc_lock.unlock();
}


void MIDISequencer::SetState(MIDISequencerState* s) {
    proc_lock.lock();
    if (IsPlaying())
        MIDIManager::AllNotesOff(); // no need to reset note matrix, thy are overwritten by the new state
    state = *s;
    if (state.notifier)                             // cause a complete GUI refresh
        state.notifier->Notify(MIDISequencerGUIEvent::GROUP_ALL);
    proc_lock.unlock();
}


void MIDISequencer::SetProcessor(unsigned int trk, MIDIProcessor* p) {
    if (!state.multitrack->IsValidTrackNumber(trk))
        return;
    Stop();
    track_processors[trk] = p;
}


bool MIDISequencer::InsertTrack(int trk) {
    if (trk == -1) trk = GetNumTracks();                // if trk = -1 (default) append track
    bool ret = false;
    proc_lock.lock();
    if (state.multitrack->InsertTrack(trk)) {
        track_processors.insert(track_processors.begin() + trk, 0);
        time_shifts.insert(time_shifts.begin() + trk, 0);
        track_ports.insert(track_ports.begin() + trk, 0);
        MIDIClockTime now = state.cur_clock;            // remember current time
        state.Reset();                                  // reset the state (syncs the iterator and the track states)
        GoToTime(now);                                  // returns to current time
        if (state.notifier)                             // cause a complete GUI refresh
            state.notifier->Notify(MIDISequencerGUIEvent::GROUP_ALL);
        ret = true;
    }
    proc_lock.unlock();
    return ret;
}


bool MIDISequencer::DeleteTrack(int trk) {
    bool ret = false;
    proc_lock.lock();
    char channel = state.multitrack->GetTrack(trk)->GetChannel();
    if (IsPlaying() && channel != -1) {
        MIDIManager::GetOutDriver(GetTrackPort(trk))->AllNotesOff(channel);
        GetTrackState(trk)->note_matrix.Clear();
    }
    if (state.multitrack->DeleteTrack(trk)) {
        if (track_processors[trk])
            delete track_processors[trk];
        track_processors.erase(track_processors.begin() + trk);
        time_shifts.erase(time_shifts.begin() + trk);
        track_ports.erase(track_ports.begin() + trk);
        MIDIClockTime now = state.cur_clock;            // remember current time
        state.Reset();                                  // reset the state (syncs the iterator and the track states)
        GoToTime(now);                                  // returns to current time
        if (state.notifier)                             // cause a complete GUI refresh
            state.notifier->Notify(MIDISequencerGUIEvent::GROUP_ALL);
        ret = true;
    }
    proc_lock.unlock();
    return ret;
}


bool MIDISequencer::MoveTrack(int from, int to) {
    if (from == to) return true;                        // nothing to do
    bool ret = false;
    proc_lock.lock();
    if (state.multitrack->MoveTrack(from, to)) {        // checks if from and to are valid
        MIDIProcessor* temp_processor = track_processors[from];
        int temp_offset = time_shifts[from];
        int temp_port = track_ports[from];
        track_processors.erase(track_processors.begin() + from);
        time_shifts.erase(time_shifts.begin() + from);
        track_ports.erase(track_ports.begin() + from);
        if (from < to)
            to--;
        track_processors.insert(track_processors.begin() + to, temp_processor);
        time_shifts.insert(time_shifts.begin() + to, temp_offset);
        track_ports.insert(track_ports.begin() + to, temp_port);
        MIDIClockTime now = state.cur_clock;            // remember current time
        state.Reset();                                  // reset the state (syncs the iterator)
        GoToTime(now);                                  // returns to current time
        if (state.notifier)                             // cause a complete GUI refresh
            state.notifier->Notify(MIDISequencerGUIEvent::GROUP_ALL);
        ret = true;
    }
    proc_lock.unlock();
    return ret;
}


void MIDISequencer::GoToZero() {
    proc_lock.lock();
    // temporarily disable the gui notifier
    bool notifier_mode = false;
    if(state.notifier) {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable(false);
    }
    // go to time zero
    state.Reset();

    // examine all the events at this specific time
    // and update the track states to reflect this time
    ScanEventsAtThisTime();

    // re-enable the gui notifier if it was enabled previously
    if(state.notifier) {
        state.notifier->SetEnable(notifier_mode);
        // cause a full gui refresh now
        state.notifier->Notify(MIDISequencerGUIEvent::GROUP_ALL);
    }
    if (IsPlaying()) {
        dev_time_offset = state.cur_time_ms = MIDItoMs(state.cur_clock);
        sys_time_offset = MIDITimer::GetSysTimeMs();
        MIDIManager::AllNotesOff();
    }
    proc_lock.unlock();
}


bool MIDISequencer::GoToTime(MIDIClockTime time_clk) {
    bool ret = true;

    proc_lock.lock();
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
    MIDITimedMessage msg;

    while (state.cur_clock < time_clk) {
        if (!GetNextEventTime(&t)) {    // no other events: we can't reach time_clk and return false
            ret = false;
            break;
        }
        if (t < time_clk)               // next event is before time_clk
            GetNextEvent (&trk, &msg);  // get it and continue
        else if (t == time_clk) {       // next event is at the right time
            MIDIMultiTrackIteratorState istate(state.iterator.GetState());  // save the state of the iterator
            GetNextEvent(&trk, &msg);           // get the event (cur_time becomes time_clk)
            state.iterator.SetState(istate);    // restore the iterator state, so ev is the next event
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
    if (IsPlaying()) {
        dev_time_offset = state.cur_time_ms = MIDItoMs(state.cur_clock);
        sys_time_offset = MIDITimer::GetSysTimeMs();
        MIDIManager::AllNotesOff();
    }
    proc_lock.unlock();
    return ret;
}


bool MIDISequencer::GoToTimeMs(float time_ms) {
    bool ret = true;

    proc_lock.lock();
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

    float t;
    int trk;
    MIDITimedMessage msg;
    float tscale = tempo_scale * 0.01;

    while (state.cur_time_ms < time_ms) {
        if (!GetNextEventTimeMs(&t)) {  // no other events: we can't reach time_clk and return false
            ret = false;
            break;
        }
        if (t < time_ms)                        // next event is before time_ms
            GetNextEvent (&trk, &msg);          // get it and continue
        else if (abs (t - time_ms) < 0.001){    // next event is at right time
            MIDIMultiTrackIteratorState istate(state.iterator.GetState());  // save the state of the iterator
            GetNextEvent(&trk, &msg);           // get the event
            state.iterator.SetState(istate);    // restore the iterator state, so ev is the next event
        }
        else {                          // next event is after time_clk : set cur_time to time_clk
            float delta_t_ms = t - time_ms;
            float ms_per_clock = 6000000.0 / (state.tempobpm *    // see MIDIToMs()
                    tscale * state.multitrack->GetClksPerBeat());
            state.cur_clock += (MIDIClockTime)(delta_t_ms / ms_per_clock);
            state.cur_time_ms = time_ms;
        }
    }

        // examine all the events at this specific time
        // and update the track states to reflect this time
	if (ret) ScanEventsAtThisTime();

        // re-enable the gui notifier if it was enabled previously
    if(state.notifier) {
        state.notifier->SetEnable( notifier_mode );
        // cause a full gui refresh now
        state.notifier->Notify(MIDISequencerGUIEvent::GROUP_ALL);
    }
    if (IsPlaying()) {
        dev_time_offset = state.cur_time_ms = MIDItoMs(state.cur_clock);
        sys_time_offset = MIDITimer::GetSysTimeMs();
        MIDIManager::AllNotesOff();
    }
    proc_lock.unlock();
    return ret;
}


// This is simpler because every measure has as first event a beat event!
bool MIDISequencer::GoToMeasure (unsigned int measure, unsigned int beat) {
    bool ret = true;

    proc_lock.lock();
        // temporarily disable the gui notifier
    bool notifier_mode = false;
    if (state.notifier) {
        notifier_mode = state.notifier->GetEnable();
        state.notifier->SetEnable (false);
    }

//    OLD VERSION if (measure < state.cur_measure ||
//         // ADDED FOLLOWING LINE:  this failed in this case!!!
//        (measure == state.cur_measure && beat < state.cur_beat) ||
//        (measure == 0 && beat == 0))
    if (measure < state.cur_measure ||
        (measure == state.cur_measure && beat <= state.cur_beat))
        state.Reset();

    int trk;
    MIDITimedMessage msg;

        // iterate thru all the events until cur-measure and cur_beat are
        // where we want them.
    if (measure > 0 || beat > 0) {          // if meas == 0 && beat == 0 nothing to do
        while(1) {
            if (!GetNextEvent(&trk, &msg)) {
                ret = false;
                break;
            }
            if (msg.IsBeatMarker()) {       // there must be a beat marker at right time
                if(state.cur_measure == measure && state.cur_beat >= beat)
                    break;
            }
        }
    }
        // examine all the events at this specific time
        // and update the track states to reflect this time
    if (ret) ScanEventsAtThisTime();

        // re-enable the gui notifier if it was enabled previously
    if (state.notifier) {
        state.notifier->SetEnable (notifier_mode);
        // cause a full gui refresh now
        state.Notify (MIDISequencerGUIEvent::GROUP_ALL);
    }
    if (IsPlaying()) {
        dev_time_offset = state.cur_time_ms = MIDItoMs(state.cur_clock);
        sys_time_offset = MIDITimer::GetSysTimeMs();
        MIDIManager::AllNotesOff();
    }
    proc_lock.unlock();
        // return true if we actually found the measure requested
    return ret;
}


bool MIDISequencer::GetNextEvent(int *trk, MIDITimedMessage *msg) {
    MIDIClockTime t;
    bool ret = false;

    proc_lock.lock();
    // ask the iterator for the current event time
    if( state.iterator.GetNextEventTime(&t) ) {
        // move current time forward one event
        MIDIClockTime new_clock;
        GetNextEventTime(&new_clock);
        double new_time_ms = MIDItoMs(new_clock);

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
            beat_marker_msg.SetTime(state.next_beat_time);
            *msg = beat_marker_msg;

            state.Process(msg);
        }
        else    {   // this event comes before the next beat
            MIDITimedMessage *msg_ptr;

            if(state.iterator.GetNextEvent(trk, &msg_ptr)) {
                state.last_event_track = *trk;

                // copy the event so Process can modify it
                *msg = *msg_ptr;

                /* OLD WITH MIDISequencerTrackProcessor
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
                */
                if ((track_processors[*trk] && !track_processors[*trk]->Process(msg)) ||
                     !state.Process(msg))
                    msg->Clear();

                // go to the next event on the multitrack
                // state.iterator.GoToNextEvent(); no more used
            }
        }
        ret = true;
    }
    proc_lock.unlock();
    return ret;
}


bool MIDISequencer::GetNextEventTime(MIDIClockTime *time_clk) {
    proc_lock.lock();
    // ask the iterator for the current event time
    bool ret = state.iterator.GetNextEventTime(time_clk);

    if(ret) {
        // if we have an event in the future, check to see if it is
        // further in time than the next beat marker
        if((*time_clk) >= state.next_beat_time)
            // ok, the next event is a beat - return the next beat time
            *time_clk = state.next_beat_time;
    }
    proc_lock.unlock();
    return ret;
}


bool MIDISequencer::GetNextEventTimeMs(float *time_ms) {
    MIDIClockTime t;
    proc_lock.lock();
    bool ret = GetNextEventTime(&t);

    if(ret)
        *time_ms = MIDItoMs(t);
    proc_lock.unlock();
    return ret;
}


float MIDISequencer::MIDItoMs(MIDIClockTime t) {
    proc_lock.lock();
    MIDITrack* track = state.multitrack->GetTrack(0);
    int ev_num = 0;
    MIDIClockTime base_t = 0, delta_t = 0, now_t = 0;
    double ms_time = 0.0;
    MIDITimedMessage* msg;
    // see below
    float ms_per_clock = 6000000.0 / (MIDI_DEFAULT_TEMPO * (float)tempo_scale * //TODO: is this right????
                                       state.multitrack->GetClksPerBeat());

    while (now_t < t) {
        if (!track->IsValidEventNum(ev_num) || track->GetEventAddress(ev_num)->GetTime() >= t) {
            // next message doesn't exists or is at t or after t
            now_t = t;
            // calculate delta_time in MIDI clocks
            delta_t = now_t - base_t;
            // and add it in msecs to ms_time
            ms_time += (delta_t * ms_per_clock);
        }
        else {
            msg = track->GetEventAddress(ev_num);
            now_t = msg->GetTime();
            // if we have a tempo change must calculate the time
            // in msecs between the last tempo change and now
            if (msg->IsTempo()) {
                // calculate delta time in MIDI clocks
                delta_t = now_t - base_t;
                // and add it in msecs to ms_time
                ms_time += (delta_t * ms_per_clock);

                // calculate new milliseconds per clock: this comes from
                //  -true_bpm = old_tempo * tempo_scale / 100
                //  -clocks_per_sec = true_bpm * clks_per_beat / 60
                //  -clocks_per_ms = clocks_per_sec / 1000
                //  -ms_per_clock = 1 / clocks_per_ms
                ms_per_clock = 6000000.0 / (msg->GetTempo() *
                                (double)tempo_scale * state.multitrack->GetClksPerBeat());

                // update variables for next tempo change (or now_t == t)
                base_t = now_t;

            }
        }
        ev_num++;
    }
    proc_lock.unlock();
    return ms_time;
}


// Inherited from MIDITICK

void MIDISequencer::Start() {
    if (!IsPlaying()) {
        std::cout << "\t\tEntered in MIDISequencer::Start() ..." << std::endl;
        MIDIManager::OpenOutPorts();
        state.Notify (MIDISequencerGUIEvent::GROUP_TRANSPORT,
                      MIDISequencerGUIEvent::GROUP_TRANSPORT_START);
        SetTimeShiftMode(true);
        MIDITickComponent::Start((tMsecs)GetCurrentTimeMs());
        std::cout << "\t\t ... Exiting from MIDISequencer::Start()" << std::endl;
    }
}


void MIDISequencer::Stop() {
    if (IsPlaying()) {
        std::cout << "\t\tEntered in MIDISequencer::Stop() ..." << std::endl;
        MIDITickComponent::Stop();
        SetTimeShiftMode(false);
        MIDIManager::AllNotesOff();
        MIDIManager::CloseOutPorts();

        state.Notify (MIDISequencerGUIEvent::GROUP_TRANSPORT,
                      MIDISequencerGUIEvent::GROUP_TRANSPORT_STOP);
        std::cout << "\t\t ... Exiting from MIDISequencer::Stop()" << std::endl;
    }
}







void MIDISequencer::StaticTickProc(tMsecs sys_time, void* pt) {
    MIDISequencer* seq_pt = static_cast<MIDISequencer *>(pt);
    seq_pt->TickProc(sys_time);
}

/* BEFORE CHANGES !!!!
void MIDISequencer::TickProc(tMsecs sys_time_)
{
    double sys_time = (double)sys_time_ - (double)sys_time_offset;
    double next_event_time = 0.0;
    int msg_track;
    MIDITimedMessage msg;

    //times++;

    // if we are in repeat mode, repeat if we hit end of the repeat region
    if(repeat_play_mode && GetCurrentMeasure() >= repeat_end_meas) {
        // yes we hit the end of our repeat block
        // shut off all notes on
        MIDIManager::AllNotesOff();

        // now move the sequencer to our start position
        GoToMeasure(repeat_start_meas);

        // our current raw system time is now the new system time offset
        sys_time_offset = sys_time;
        sys_time = 0;

        // the sequencer time offset now must be reset to the
        // time in milliseconds of the sequence start point
        seq_time_offset = (unsigned long)GetCurrentTimeMs();
    }

    // find all events that exist before or at this time,
    // but only if we have space in the output queue to do so!
    // also limit ourselves to 100 midi events max.
    int output_count = 100;

    while(
        GetNextEventTimeMs(&next_event_time)
        && (next_event_time - seq_time_offset) <= sys_time
        && (--output_count) > 0 ) {

        // found an event! get it!
        if(GetNextEvent(&msg_track, &msg) && !msg.IsMetaEvent())

            // tell the driver the send this message now
            MIDIManager::GetOutDriver(GetTrackPort(msg_track))->OutputMessage(msg);
    }
*/




void MIDISequencer::TickProc(tMsecs sys_time) {
    float next_event_time = 0.0;
    int msg_track;
    MIDITimedMessage msg;

    proc_lock.lock();
    /*
    static unsigned int times;
    times++;
    if (!(times % 100))
        std::cout << "MIDISequencer::TickProc() " << times << " times" << std::endl;
    */
    // if we are in repeat mode, repeat if we hit end of the repeat region
    if(repeat_play_mode && GetCurrentMeasure() >= repeat_end_meas) {
        // yes we hit the end of our repeat block
        // shut off all notes on
        MIDIManager::AllNotesOff();

        // now move the sequencer to our start position
        GoToMeasure(repeat_start_meas);

        // our current raw system time is now the new system time offset
        sys_time_offset = sys_time;
        // sys_time = 0;

        // the sequencer time offset now must be reset to the
        // time in milliseconds of the sequence start point
         dev_time_offset = (tMsecs)GetCurrentTimeMs();
    }

    // find all events that exist before or at this time,
    // but only if we have space in the output queue to do so!
    // also limit ourselves to 100 midi events max.
    int output_count = 100;
    tMsecs cur_time = sys_time - sys_time_offset + dev_time_offset;

    while(
        GetNextEventTimeMs(&next_event_time)
        && next_event_time <= cur_time
        && (--output_count) > 0 ) {

        // found an event! get it!
        if(GetNextEvent(&msg_track, &msg) && !msg.IsMetaEvent())

            // tell the driver the send this message now
            MIDIManager::GetOutDriver(GetTrackPort(msg_track))->OutputMessage(msg);
    }

/* OLD
    // auto stop at end of sequence
    if( !(repeat_play_mode && GetCurrentMeasure() >= repeat_end_meas) &&
            !GetNextEventTimeMs(&next_event_time))
        // no events left
        if (auto_stop_proc) {
            std::thread(auto_stop_proc, auto_stop_param).detach();
            std::string s = "\t\tExit from MIDIManager::SequencerPlayProc() times = " +
                std::to_string(times) +"\n";
            std::cout << s;
        }
    //times--;
*/
// NEW
    // auto stop at end of sequence
    if( !(repeat_play_mode && GetCurrentMeasure() >= repeat_end_meas) &&
        !GetNextEventTimeMs(&next_event_time)) {
        // no events left

        std::thread(StaticStopProc, this).detach();
        std::cout << "Stopping the sequencer: StaticStopProc called" << std::endl;
    }
    proc_lock.unlock();
}



void MIDISequencer::ScanEventsAtThisTime() {
    // save the current iterator state
    MIDIMultiTrackIteratorState istate( state.iterator.GetState() );
    int prev_measure = state.cur_measure;
    int prev_beat = state.cur_beat;
    MIDIClockTime orig_clock = state.cur_clock;
    double orig_time_ms = state.cur_time_ms;

    // process all messages up to and including this time only
    MIDIClockTime t = 0;
    int trk;
    MIDITimedMessage msg;
    while( GetNextEventTime(&t) && t == orig_clock && GetNextEvent(&trk, &msg)) {
        ;  // cycle through all events at this time
    }

    // restore the iterator state
    state.iterator.SetState(istate);
    state.cur_measure = prev_measure;
    state.cur_beat = prev_beat;
    state.cur_clock = orig_clock;
    state.cur_time_ms = double(orig_time_ms);
    if (state.cur_clock == state.last_beat_time)
        state.next_beat_time = state.cur_clock;
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
// search // NC */ for modifies
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
    seq ( seq_ ),   // NEW
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
// NC            // this has been splitted into this and MIDISequencerState::Process()
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
    seq ( s ),                                  // NC
    multitrack ( m ),
    num_tracks ( m->GetNumTracks() ),
    iterator ( m ),
    cur_clock ( 0 ),
    cur_time_ms ( 0 ),
    cur_beat ( 0 ),
    cur_measure ( 0 ),
    last_beat_time ( 0 ),                       // NC
    next_beat_time ( 0 ),
    tempobpm( 120.0 ),                          // NC
    timesig_numerator( 4 ),                     // NC
    timesig_denominator( 4 ),                   // NC
    keysig_sharpflat( 0 ),                      // NC
    keysig_mode( 0 )                            // NC
{
    for ( int i = 0; i < num_tracks; ++i )
    {
        track_state[i] = new MIDISequencerTrackState ( s, i, notifier );
    }
    *marker_name = 0;                           // NC
}

MIDISequencerState::MIDISequencerState ( const MIDISequencerState &s )
    :
    notifier ( s.notifier ),
    seq (s.seq ),                               // NC
    multitrack ( s.multitrack ),
    num_tracks ( s.num_tracks ),
    iterator ( s.iterator ),
    cur_clock ( s.cur_clock ),
    cur_time_ms ( s.cur_time_ms ),
    cur_beat ( s.cur_beat ),
    cur_measure ( s.cur_measure ),
    last_beat_time ( s.last_beat_time ),        // NC
    next_beat_time ( s.next_beat_time ),
    tempobpm( s.tempobpm ),                     // NC
    timesig_numerator( s.timesig_numerator ),   // NC
    timesig_denominator( s.timesig_denominator),// NC
    keysig_sharpflat( s.keysig_sharpflat ),     // NC
    keysig_mode( s. keysig_mode )               // NC
{
    for ( int i = 0; i < num_tracks; ++i )
    {
        track_state[i] = new MIDISequencerTrackState ( *s.track_state[i] );
    }
    memmove( marker_name, s.marker_name, sizeof( marker_name ) );    // NC
}


MIDISequencerState::~MIDISequencerState()
{
    for ( int i = 0; i < num_tracks; ++i )
    {
        jdks_safe_delete_object( track_state[i] );
    }
}


// NC NEW corrected
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
    last_beat_time = s.last_beat_time;  // NC
    next_beat_time = s.next_beat_time;
    tempobpm = s.tempobpm;
    timesig_numerator = s.timesig_numerator;
    timesig_denominator = s.timesig_denominator;
    keysig_sharpflat = s.keysig_sharpflat;
    keysig_mode = s.keysig_mode;
    memmove( marker_name, s.marker_name, sizeof ( marker_name ) );

    return *this;
}


void MIDISequencerState::Reset() {              // NC
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



bool MIDISequencerState::Process( MIDITimedBigMessage *msg ) // NC
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
            tempobpm = ( double ) ( msg->GetTempo32() / 32. );

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
            marker_name[len] = 0;   // NC: there was a bug!
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

        last_beat_time = cur_clock; // NC
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



// NC: THESE WERE BUGGY (they stopped AFTER the first event at right time)
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
            double ms_per_clock = ( double )6000000.0 / (state.tempobpm *
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



// NEW BY NC
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




// NOTE BY NC: this is simpler because every measure has as first event a beat event!

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
        // NC  // ADDED FOLLOWING LINE:  this failed in this case!!!
         ( measure == state.cur_measure && beat < state.cur_beat ) ||
         ( measure == 0 && beat == 0 ) )
    {

        state.Reset();                      // NC
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
        //double clocks_per_sec = ( ( state.track_state[0]->tempobpm *    // OLD
		double clocks_per_sec = ( ( state.tempobpm *            // NC
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





// NEW by NC
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
