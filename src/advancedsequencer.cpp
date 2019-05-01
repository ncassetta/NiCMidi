#include "../include/advancedsequencer.h"
#include "../include/filereadmultitrack.h"          // for LoadMIDIFile()

#include <iostream>



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
    if(mute || solo == NOT_SOLOED)
        // yes, ignore event.
        return false;

    // is the event a NoOp?
    if(msg->IsNoOp())
        // yes, ignore event.
        return false;

    // pass the event to our extra_proc if we have one
    if(extra_proc && extra_proc->Process(msg) == false) // TODO:Set extra proc!!!!
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
            float vel = (float)msg->GetVelocity();
            vel = (int)(vel * velocity_scale / 100.0 + 0.5);

            // make sure velocity is never more than 127
            if(vel > 127)
                vel = 127;

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
//                   class AdvancedSequencer                              //
////////////////////////////////////////////////////////////////////////////



AdvancedSequencer::AdvancedSequencer(MIDISequencerGUINotifier *n) :
    MIDISequencer (new MIDIMultiTrack(17) , n),
    num_measures(0),
    file_loaded (false),
    owns_tracks (true)                           // remembers that the multitrack is owned
{
    MIDIManager::AddMIDITick(this);
    thru.SetProcessor(&thru_processor);
    thru_processor.SetProcessor(&thru_rechannelizer);
    thru_processor.SetProcessor(&thru_transposer);
    MIDIManager::AddMIDITick(&thru);
    for (unsigned  int i = 0; i < GetNumTracks(); ++i)
        track_processors[i] = new MIDISequencerTrackProcessor;
}


AdvancedSequencer::AdvancedSequencer(MIDIMultiTrack* mlt, MIDISequencerGUINotifier *n) :
    MIDISequencer (mlt, n),
    owns_tracks (false)                         // remembers that the multitrack is not owned
{
    MIDIManager::AddMIDITick(this);
    file_loaded = !state.multitrack->IsEmpty();
    ExtractWarpPositions();                     // sets warp_positions and num_measures
    thru.SetProcessor(&thru_processor);
    thru_processor.SetProcessor(&thru_rechannelizer);
    thru_processor.SetProcessor(&thru_transposer);
    MIDIManager::AddMIDITick(&thru);
    for (unsigned  int i = 0; i < GetNumTracks(); ++i)
        track_processors[i] = new MIDISequencerTrackProcessor;
}


AdvancedSequencer::~AdvancedSequencer() {
    Stop();
    MIDIManager::RemoveMIDITick(this);
    for (unsigned int i = 0; i < GetNumTracks(); ++i) {
        delete track_processors[i];
        track_processors[i] = 0;
    }
    if (owns_tracks)
        delete state.multitrack;
}



// Doesn't empty the multitrack
void AdvancedSequencer::Reset() {
    Stop();
    MIDITimer::Wait(500);       // pauses for 0.5 sec (TROUBLE WITHOUT THIS!!!! I DON'T KNOW WHY) TODO: eliminate this?
    MIDISequencer::Reset();     // syncronize the num of tracks and reset track processors (now calls GoToZero())
    //GoToZero();
    for (unsigned  int i = 0; i < GetNumTracks(); ++i)  // MIDISequencer::Reset() deletes the processors
        track_processors[i] = new MIDISequencerTrackProcessor;
    ExtractWarpPositions();
}


bool AdvancedSequencer::Load (const char *fname) {
    char realname[1024];
    strcpy (realname, fname);

    Stop();
    state.multitrack->Clear();

    file_loaded = LoadMIDIFile(realname, state.multitrack);
    Reset();                    // synchronizes the sequencer with the multitrack and goes to 0
    //seq->GoToZero();      now called by Reset()
    //ExtractWarpPositions();
    return file_loaded;
}


bool AdvancedSequencer::Load (const MIDIMultiTrack* tracks) {
    Stop();
    *state.multitrack = *tracks;
    file_loaded = !state.multitrack->IsEmpty();
    Reset();                    // synchronizes the sequencer with the multitrack and goes to 0
    return file_loaded;
}


void AdvancedSequencer::UnLoad() {
    Stop();
    state.multitrack->Clear();
    state.multitrack->SetClksPerBeat(MIDIMultiTrack::DEFAULT_CLKS_PER_BEAT);
    file_loaded = false;
    Reset();
    //seq->GoToZero();      now called by Reset()
    //ExtractWarpPositions();	idem
}


bool AdvancedSequencer::GetSoloMode() const {
    for (unsigned int i = 0; i < GetNumTracks(); ++i ) {
        if(((MIDISequencerTrackProcessor *)track_processors[i])->solo == MIDISequencerTrackProcessor::NOT_SOLOED)
            return true;
    }
    return false;
}


unsigned int AdvancedSequencer::GetCurrentMeasure() const {
    if (!file_loaded)
        return 0;
    return MIDISequencer::GetCurrentMeasure();
}


unsigned int AdvancedSequencer::GetCurrentBeat() const {
    if (!file_loaded)
        return 0;
    return MIDISequencer::GetCurrentBeat();
}


MIDIClockTime AdvancedSequencer::GetCurrentBeatOffset() const {
    if (!file_loaded)
        return 0;
    return MIDISequencer::GetCurrentBeatOffset();
}


int AdvancedSequencer::GetTimeSigNumerator() const {
    if (!file_loaded)
        return MIDI_DEFAULT_TIMESIG_NUMERATOR;
    return state.timesig_numerator;
}


int AdvancedSequencer::GetTimeSigDenominator() const {
    if (!file_loaded)
        return MIDI_DEFAULT_TIMESIG_DENOMINATOR;
    return state.timesig_denominator;
}


int AdvancedSequencer::GetKeySigSharpsFlats() const {
    if (!file_loaded)
        return MIDI_DEFAULT_KEYSIG_KEY;
    return state.keysig_sharpflat;
}


int AdvancedSequencer::GetKeySigMode() const {
    if (!file_loaded)
        return MIDI_DEFAULT_KEYSIG_MODE;
    return state.keysig_mode;
}


std::string AdvancedSequencer::GetCurrentMarker() const {
    if (!file_loaded)
        return "";
    return state.marker_text;
}


std::string AdvancedSequencer::GetTrackName (unsigned int trk) const {
    if (!file_loaded)
        return "";
    return GetTrackState(trk)->track_name;
}


char AdvancedSequencer::GetTrackVolume (unsigned int trk) const {
    if (!file_loaded)
        return 100;
    return GetTrackState(trk)->control_values[C_MAIN_VOLUME];
}


char AdvancedSequencer::GetTrackProgram (unsigned int trk) const {
    if (!file_loaded)
        return 0;
    return GetTrackState(trk)->program;
}


int AdvancedSequencer::GetTrackNoteCount (unsigned int trk) const {
    if (!file_loaded || !IsPlaying())
        return 0;
    else
        return GetTrackState(trk)->note_matrix.GetTotalCount();
}


unsigned int AdvancedSequencer::GetTrackVelocityScale (unsigned int trk) const {
    if (!file_loaded)
        return 100;
    return ((MIDISequencerTrackProcessor *)track_processors[trk])->velocity_scale;
}


int AdvancedSequencer::GetTrackRechannelize (unsigned int trk) const {
    if (!file_loaded)
        return -1;
    return GetTrackProcessor(trk)->rechannel;
}


int AdvancedSequencer::GetTrackChannel (unsigned int trk) {
    if (!file_loaded)
        return -1;
    return (GetTrackProcessor(trk)->rechannel == -1 ?
            GetMultiTrack()->GetTrack(trk)->GetChannel() : GetTrackProcessor(trk)->rechannel);
}


int AdvancedSequencer::GetTrackTranspose (unsigned int trk) const {
    if (!file_loaded)
        return 0;
    return GetTrackProcessor(trk)->transpose;
}


int AdvancedSequencer::GetTrackTimeShift (unsigned int trk) const {
    if (!file_loaded)
        return 0;
    return GetTrackTimeShift(trk);
}


void AdvancedSequencer::SetMIDIThruEnable(bool on_off) {
    if (on_off)
        thru.Start();
    else {
        thru.Stop();            // calls AllNotesOff() on thru out channel
    }
}


void AdvancedSequencer::SetMIDIThruChannel (int chan) {
    thru_rechannelizer.SetAllRechan (chan);
    MIDIManager::AllNotesOff();
}


void AdvancedSequencer::SetMIDIThruTranspose (int amt) {
    thru_transposer.SetAllTranspose (amt);
    MIDIManager::AllNotesOff();
}


/* NOTE BY NC: soloing and muting has been enhanced to keep count of muted CC, SYSEX changes previously muted
 * so when we unmute a track it sounds with correct parameters even if they weren't tramsmitted before
 * So member function are changed
 */


void AdvancedSequencer::SetTrackSolo (unsigned int trk) {   // unsoloing done by UnSoloTrack()
    if (!file_loaded || !state.multitrack->IsValidTrackNumber(trk))
        return;
    proc_lock.lock();
    for (unsigned int i = 0; i < GetNumTracks(); ++i ) {
        if(i == trk) {
            ((MIDISequencerTrackProcessor *)track_processors[i])->solo = MIDISequencerTrackProcessor::SOLOED;
            if (IsPlaying())
                // track could be muted before soloing: this sets appropriate CC, PC, etc
                // not previously sent
                CatchEventsBefore(trk);     // MUST be here! We must previously unmute the track!
        }
        else {
            ((MIDISequencerTrackProcessor *)track_processors[i])->solo = MIDISequencerTrackProcessor::NOT_SOLOED;
            if (IsPlaying() && GetTrackChannel(i) != -1) {
                MIDIManager::GetOutDriver(GetTrackPort(trk))->AllNotesOff(GetTrackChannel(i));
                GetTrackState(i)->note_matrix.Clear();
            }
        }
    }
    proc_lock.unlock();
}

/*
void AdvancedSequencer::SetSoloMode(bool m, int trk)  {
    proc_lock.lock();
    solo_mode = m;
    for(unsigned int i = 0; i < GetNumTracks(); ++i ) {
        if(i == (unsigned)trk)
            (MIDISequencerTrackProcessor *)track_processors[i]->solo = true;
        else
            (MIDISequencerTrackProcessor *)track_processors[i]->solo = false;
    }
    proc_lock.unlock();
}
*/


void AdvancedSequencer::UnSoloTrack()  {
    if (!file_loaded)
        return;
    proc_lock.lock();
    for(unsigned int i = 0; i < GetNumTracks(); ++i ) {
        bool old_solo = ((MIDISequencerTrackProcessor *)track_processors[i])->solo;
        ((MIDISequencerTrackProcessor *)track_processors[i])->solo = MIDISequencerTrackProcessor::NO_SOLO;
        if (IsPlaying() && !old_solo)
            // this sets appropriate CC, PC, etc for previously muted tracks
            CatchEventsBefore();
    }
    proc_lock.unlock();
}


void AdvancedSequencer::SetTrackMute (unsigned int trk, bool f) {
    if (!file_loaded || !state.multitrack->IsValidTrackNumber(trk))
        return;
    proc_lock.lock();
    GetTrackProcessor(trk)->mute = f;
    int channel = GetTrackChannel(trk);
    if (IsPlaying() && channel != -1) {
        if(f) {
            MIDIManager::GetOutDriver(GetTrackPort(trk))->AllNotesOff(channel);
            GetTrackState(trk)->note_matrix.Clear();
        }
        else
            // track was muted: this set appropriate CC, PC, etc not previously sent
            CatchEventsBefore(trk);
    }
    proc_lock.unlock();
}


void AdvancedSequencer::UnmuteAllTracks() {
    if (!file_loaded)
        return;
    proc_lock.lock();
    for (unsigned int i = 0; i < GetNumTracks(); ++i)
        GetTrackProcessor(i)->mute = false;
    if (IsPlaying())
        // this set appropriate CC, PC, etc for previously muted tracks
        CatchEventsBefore();
    proc_lock.unlock();
}


void AdvancedSequencer::SetTrackVelocityScale (unsigned int trk, unsigned int scale) {
    if (!file_loaded || !state.multitrack->IsValidTrackNumber(trk))
        return;
    proc_lock.lock();
    ((MIDISequencerTrackProcessor *)track_processors[trk])->velocity_scale = scale;
    proc_lock.unlock();
}


void AdvancedSequencer::SetTrackRechannelize (unsigned int trk, int chan) {
    if (!file_loaded || !state.multitrack->IsValidTrackNumber(trk))
        return;
    proc_lock.lock();
    if (IsPlaying() && GetTrackChannel(trk) != chan && !(GetTrackChannel(trk) == -1)) {
        MIDIManager::GetOutDriver(GetTrackPort(trk))->AllNotesOff(GetTrackChannel(trk));
        GetTrackState(trk)->note_matrix.Clear();
    }
    GetTrackProcessor(trk)->rechannel = chan;
    proc_lock.unlock();
}


void AdvancedSequencer::SetTrackTranspose (unsigned int trk, int trans) {
    if (!file_loaded || !state.multitrack->IsValidTrackNumber(trk))
        return;
    proc_lock.lock();
    if (IsPlaying() && GetTrackTranspose(trk) != trans && !(GetTrackChannel(trk) == -1)) {
        MIDIManager::GetOutDriver(GetTrackPort(trk))->AllNotesOff(GetTrackChannel(trk));
        GetTrackState (trk)->note_matrix.Clear();
    }
    GetTrackProcessor(trk)->transpose = trans;
    proc_lock.unlock();
}


/*  NO! Inherited from MIDISequencer
// must be rewritten, so it calls AdvancedSequencer::GoToTime()
void AdvancedSequencer::SetTrackTimeShift (unsigned int trk, int offset) {
    if (!file_loaded || !state.multitrack->IsValidTrackNumber(trk))
        return;
    char channel = state.multitrack->GetTrack(trk)->GetChannel();
    proc_lock.lock();
    if (IsPlaying() && channel != -1) {
        MIDIManager::GetOutDriver(GetTrackPort(trk))->AllNotesOff(channel);
        GetTrackState(trk)->note_matrix.Clear();
    }
    time_shifts[trk] = offset;
    //GoToTime(GetCurrentMIDIClockTime());        // syncronize the iterator
    proc_lock.unlock();
}
*/


void AdvancedSequencer::GoToTime (MIDIClockTime t) {
    if (!file_loaded)
        return;

    // figure out which warp item we use
    // try warp to the last warp point BEFORE the
    // requested measure

    unsigned int warp_to_item = 0;
    for (; warp_to_item < warp_positions.size(); warp_to_item++) {
        if (warp_positions[warp_to_item].cur_clock > t)
            break;
    }
    if (warp_to_item == warp_positions.size() && warp_to_item != 0)
        warp_to_item--;

    if (IsPlaying()) {
        proc_lock.lock();
        SetState (&warp_positions[warp_to_item]);
        MIDISequencer::GoToTime (t);
        CatchEventsBefore();
        proc_lock.unlock();
    }
    else {
        SetState (&warp_positions[warp_to_item]);
        MIDISequencer::GoToTime (t);
        for (unsigned int i = 0; i < GetNumTracks(); ++i)
            GetTrackState(i)->note_matrix.Clear();
    }
}


void AdvancedSequencer::GoToMeasure (int measure, int beat) {
    if (!file_loaded)
        return;

    // figure out which warp item we use
    // try warp to the last warp point BEFORE the
    // requested measure
    unsigned int warp_to_item = measure / MEASURES_PER_WARP;

    if (warp_to_item >= warp_positions.size())
        warp_to_item = warp_positions.size() - 1;

    if (IsPlaying()) {
        proc_lock.lock();
        SetState (&warp_positions[warp_to_item]);
        MIDISequencer::GoToMeasure (measure, beat);
        CatchEventsBefore();
        proc_lock.unlock();
    }
    else {
        SetState (&warp_positions[warp_to_item]);
        MIDISequencer::GoToMeasure (measure, beat);
        for (unsigned int i = 0; i < GetNumTracks(); ++i)
            GetTrackState (i)->note_matrix.Clear();
    }
}


void AdvancedSequencer::Start () {
    if (!file_loaded)
        return;

    std::cout << "\t\tEntered in AdvancedSequencer::Start() ...\n";
    MIDISequencer::Stop();
    if (repeat_play_mode)
        GoToMeasure (repeat_start_meas);

    MIDIManager::OpenOutPorts();
    CatchEventsBefore();
    // this intercepts any CC, SYSEX and TEMPO messages and send them to the out port
    // allowing to start with correct values; we could incorporate this in the
    // sequencer state, but it would track even CC (not difficult) and SYSEX messages

    MIDISequencer::Start();             // calls OpenOutPorts() again
    std::cout << "\t\t ... Exiting from AdvancedSequencer::Start()" << std::endl;
}


void AdvancedSequencer::Stop() {
    if (!file_loaded || !IsPlaying())
        return;

    std::cout << "\t\tEntered in AdvancedSequencer::Stop() ...\n";
    MIDISequencer::Stop();              // calls CloseOutPorts()
    //MIDIManager::CloseOutPorts();
    //mgr->AllNotesOff();       // already done by SeqStop()
    //mgr->Reset();
    // stops on a beat (and clear midi matrix)
    GoToMeasure(state.cur_measure, state.cur_beat);
    MIDIManager::CloseOutPorts();       // must call again this (see Start())
    std::cout << "\t\t ... Exiting from AdvancedSequencer::Stop()" << std::endl;
}


void AdvancedSequencer::OutputMessage(MIDITimedMessage& msg, unsigned int port) {
    bool was_open = true;
    proc_lock.lock();
    MIDIOutDriver* driver = MIDIManager::GetOutDriver(port);
    if (!driver->IsPortOpen()) {
        was_open = false;
        driver->OpenPort();
    }
    driver->OutputMessage(msg);
    if (!was_open)
        driver->ClosePort();
    proc_lock.unlock();
}

/*
void AdvancedSequencer::SetRepeatPlay (bool f, int start_measure, int end_measure) {
    if (!file_loaded)
        return;

    if (start_measure < end_measure && start_measure >= 0 && end_measure <= num_measures)
        seq->SetRepeatPlay (f, start_measure, end_measure);
    else
        mgr->SetRepeatPlay(false, 0, 0);

    if (mgr->IsSeqPlay() && mgr->GetRepeatPlay()) {
        mgr->SeqStop();
        GoToMeasure (start_measure);
        CatchEventsBefore();
        mgr->SeqPlay();
    }
}
*/











void AdvancedSequencer::SetSMPTE(SMPTE* s) {
    if (!file_loaded || IsPlaying())
        return;

    const MIDIMessage* msg = 0;

    // search a SMPTE event in track 0, time 0
    const MIDITrack* trk = GetMultiTrack()->GetTrack(0);
    for (unsigned int i = 0; i < trk->GetNumEvents(); i++) {
        if (trk->GetEventAddress(i)->GetTime() > 0)
            break;
        if (trk->GetEventAddress(i)->IsSMPTEOffset()) {
            msg = trk->GetEventAddress(i);
            break;
        }
    }
    if (msg != 0) {                 // event found
        const MIDISystemExclusive* offset = msg->GetSysEx();
        char format = (offset->GetData(0) & 0x7f) >> 5;
        switch (format) {
            case 0:
                s->SetSMPTERate(SMPTE_RATE_24);
                break;
            case 1:
                s->SetSMPTERate(SMPTE_RATE_25);
                break;
            case 2:
                s->SetSMPTERate(SMPTE_RATE_30DF);
                break;
            case 3:
                s->SetSMPTERate(SMPTE_RATE_30);
                break;
        }
        s->SetOffset(offset->GetData(0) & 0x1f, offset->GetData(1), offset->GetData(2),
                     offset->GetData(3), offset->GetData(4));
    }
    else {
        s->SetSMPTERate(SMPTE_RATE_30);
        s->SetOffset(0);

    }
}

/*      OLD WITH InternalStop()
void AdvancedSequencer::SetChanged() {
        // IMPORTANT: REWRITTEN: WAS BUGGY!!!!!
    bool was_playing = false;
    if (IsPlaying()) {
        was_playing = true;
//        InternalStop();     // however you should avoid to edit the MIDIMultiTrack during playback!
    }
    file_loaded = !GetMultiTrack()->IsEmpty();
    ExtractWarpPositions();
    if (was_playing);
//        InternalStart();
}
*/


void AdvancedSequencer::SetChanged() {
    proc_lock.lock();
    file_loaded = !GetMultiTrack()->IsEmpty();
    ExtractWarpPositions();
    proc_lock.unlock();
}



//
// protected members
//


void AdvancedSequencer::ExtractWarpPositions() {
    if (!file_loaded) {
        warp_positions.clear();
        num_measures = 0;
        return;
    }

    MIDISequencerGUINotifier* notifier = state.notifier;

    Stop();         //TODO: this forbids to edit the multitrack while the sequencer is playing
                    // is this right?
    // warp_positions is now a vector of objects ( not pointers ) so we can minimize memory dealloc/alloc

    MIDIClockTime cur_time = GetCurrentMIDIClockTime();

    // temporarily disable the gui notifier
    bool notifier_mode = false;

    if (notifier) {
        notifier_mode = notifier->GetEnable();
        notifier->SetEnable (false);
    }

    unsigned int num_warp_positions = 0;
    while (MIDISequencer::GoToMeasure (num_warp_positions * MEASURES_PER_WARP)) {
        // save current sequencer state at this position
        if (num_warp_positions < warp_positions.size())
            // copy if it's already contained ...
            warp_positions[num_warp_positions] = state;
        else
            // ... or push back
            warp_positions.push_back(state);

        num_warp_positions++;
    }
    if (warp_positions.size() > num_warp_positions)
        // adjust vector size if it was greater than actual
        warp_positions.resize(num_warp_positions, MIDISequencerState(GetMultiTrack(), notifier));

    // now find the actual number of measures
    num_measures = (num_warp_positions - 1) * MEASURES_PER_WARP;
    while (MIDISequencer::GoToMeasure(num_measures + 1))
        num_measures++;

    GoToTime(cur_time);

    // re-enable the gui notifier if it was enabled previously
    if (notifier) {
        notifier->SetEnable (notifier_mode);
        // cause a full gui refresh now
        notifier->Notify (MIDISequencerGUIEvent::GROUP_ALL);
    }
}

/*
void AdvancedSequencer::CatchEventsBefore() {
    MIDITimedMessage msg;
    MIDITimedMessage *msgp;
    MIDIMultiTrackIterator iter( seq->GetState()->multitrack );
    int events_sent = 0;

    if (GetCurrentMIDIClockTime() == 0)         // nothing to do
        return;
    std::cout << "Catch events before started ..." << std::endl;

    //mgr->OpenOutPorts();

    int trk;
    iter.GoToTime(0);
    // re-send all sysex, except real-time ones
    while (iter.GetNextEvent( &trk, &msgp ) && msgp->GetTime() < seq->GetCurrentMIDIClockTime()) {
        msg = *msgp;
        unsigned int port = seq->GetTrackPort(trk);

        if (msg.IsSysEx() &&
            !(msg.GetSysEx()->IsGMReset() || msg.GetSysEx()->IsGSReset() || msg.GetSysEx()->IsXGReset())) {
            OutputMessage(msg, port);
            events_sent++;
        }
        //iter.GoToNextEvent();     not yet used
    }

    // now set program, controls and pitch bend
    for (int i = 0; i < GetNumTracks(); i++) {
        if (!GetTrackMute(i)) {
            int channel = GetTrackChannel(i);
            unsigned int port = seq->GetTrackPort(i);
            if (channel == -1)
                continue;
            const MIDISequencerTrackState* state = seq->GetTrackState(i);
            // set the current program
            msg.SetProgramChange(channel, state->program);
            OutputMessage(msg, port);
            // set thecurrent pitch bend value
            msg.SetPitchBend(channel, state->bender_value);
            OutputMessage(msg, port);
            events_sent += 2;
            // set the controllers
            for (unsigned int j = 0; j < C_ALL_NOTES_OFF; j++) {
                if (state->control_values[j] != -1) {
                    msg.SetControlChange(channel, j, state->control_values[j]);
                    OutputMessage(msg, port);
                    events_sent++;
                }   // TODO: RPN and NRPN
            }
        }
    }

    //mgr->CloseOutPorts();
    std::cout << "CatchEventsBefore finished: events sent: " << events_sent << std::endl;
}
*/

void AdvancedSequencer::CatchEventsBefore() {
    MIDITimedMessage msg;
    MIDITrack* trk;
    unsigned int port;
    int events_sent = 0;

    if (GetCurrentMIDIClockTime() == 0)         // nothing to do
        return;
    std::cout << "Catch events before started ..." << std::endl;

    MIDIManager::OpenOutPorts();

    //first send sysex (but not reset ones)
    for (unsigned int i = 0; i < GetNumTracks(); i++) {
        trk = GetMultiTrack()->GetTrack(i);
        if (!(trk->HasSysex())) continue;
        msg.SetTime(0);
        port = GetTrackPort(i);
        for (unsigned int j = 0; j < trk->GetNumEvents() && msg.GetTime() <= GetCurrentMIDIClockTime(); j++) {
            msg = trk->GetEvent(j);
            if (msg.IsSysEx() &&
                !(msg.GetSysEx()->IsGMReset() || msg.GetSysEx()->IsGSReset() || msg.GetSysEx()->IsXGReset())) {

                OutputMessage(msg, port);
                events_sent++;
            }
        }
    }

    // then set program, pitch bend and controls of ordinary channel tracks, according to the
    // sequencer state
    for (unsigned int i = 0; i < GetNumTracks(); i++) {
        trk = GetMultiTrack()->GetTrack(i);
        if (!GetTrackMute(i) &&
            (trk->GetType() == MIDITrack::TYPE_CHAN || trk->GetType() == MIDITrack::TYPE_IRREG_CHAN)) {
            int channel = GetTrackChannel(i);
            port = GetTrackPort(i);
            const MIDISequencerTrackState* tr_state = GetTrackState(i);
            // set the current program
            if (tr_state->program != -1) {
                msg.SetProgramChange(channel, tr_state->program);
                OutputMessage(msg, port);
                events_sent++;
            }
            // set the current pitch bend value
            msg.SetPitchBend(channel, tr_state->bender_value);
            OutputMessage(msg, port);
            events_sent ++;
            // set the controllers
            for (unsigned int j = 0; j < C_ALL_NOTES_OFF; j++) {
                if (tr_state->control_values[j] != -1) {
                    msg.SetControlChange(channel, j, tr_state->control_values[j]);
                    OutputMessage(msg, port);
                    events_sent++;
                }   // TODO: RPN and NRPN
            }
        }
    }

    // and now send program, controls and pitch bend of non compliant tracks
    for (unsigned int i = 0; i < GetMultiTrack()->GetNumTracks(); i++) {
        trk = GetMultiTrack()->GetTrack(i);
        if (trk->GetType() == MIDITrack::TYPE_MIXED_CHAN) {
            port = GetTrackPort(i);
            msg.SetTime(0);
            for (unsigned int j = 0; j < trk->GetNumEvents() && msg.GetTime() <= GetCurrentMIDIClockTime(); j++) {
                msg = trk->GetEvent(j);
                if (msg.IsProgramChange() || msg.IsControlChange() || msg.IsPitchBend()) {
                    OutputMessage(msg, port);
                    events_sent++;
                }
            }
        }
    }

    MIDIManager::CloseOutPorts();
    std::cout << "CatchEventsBefore finished: events sent: " << events_sent << std::endl;
}




void AdvancedSequencer::CatchEventsBefore(int trk) {
    MIDITimedMessage msg;
    MIDITrack* t = GetMultiTrack()->GetTrack(trk);
    unsigned int port = GetTrackPort(trk);
    int events_sent = 0;

    if (GetCurrentMIDIClockTime() == 0)         // nothing to do
        return;
    std::cout << "Catch events before started for track " << trk << " ..." << std::endl;

    MIDIManager::OpenOutPorts();

    //first send sysex (but not reset ones)
    if (t->HasSysex()) {
        msg.SetTime(0);
        for (unsigned int i = 0; i < t->GetNumEvents() && msg.GetTime() <= GetCurrentMIDIClockTime(); i++) {
            msg = t->GetEvent(i);
            if (msg.IsSysEx() &&
                !(msg.GetSysEx()->IsGMReset() || msg.GetSysEx()->IsGSReset() || msg.GetSysEx()->IsXGReset())) {

                OutputMessage(msg, port);
                events_sent++;
            }
        }
    }


    // restore program, pitch bend and controllers as registered in the sequencer state
    if (!GetTrackMute(trk)) {
        // if this is an ordinary channel track ...
        if (t->GetType() == MIDITrack::TYPE_CHAN || t->GetType() == MIDITrack::TYPE_IRREG_CHAN) {
            int channel = GetTrackChannel(trk);     // takes into account rechannelize
            const MIDISequencerTrackState* tr_state = GetTrackState(trk);
            // set the current program
            if (tr_state->program != -1) {
                msg.SetProgramChange(channel, tr_state->program);
                OutputMessage(msg, port);
                events_sent++;
            }
            // set the current pitch bend value
            msg.SetPitchBend(channel, tr_state->bender_value);
            OutputMessage(msg, port);
            events_sent ++;
            // set the controllers
            for (unsigned int j = 0; j < C_ALL_NOTES_OFF; j++) {
                if (tr_state->control_values[j] != -1) {
                    msg.SetControlChange(channel, j, tr_state->control_values[j]);
                    OutputMessage(msg, port);
                    events_sent++;
                }   // TODO: RPN and NRPN
            }
        }
        // if the track has mixed channels send all previous messages
        else if (t->GetType() == MIDITrack::TYPE_MIXED_CHAN) {
            msg.SetTime(0);
            for (unsigned int i = 0; i < t->GetNumEvents() && msg.GetTime() <= GetCurrentMIDIClockTime(); i++) {
                msg = t->GetEvent(i);
                if (msg.IsProgramChange() || msg.IsControlChange() || msg.IsPitchBend()) {
                    OutputMessage(msg, port);
                    events_sent++;
                }
            }
        }
    }

    MIDIManager::CloseOutPorts();
    std::cout << "CatchEventsBefore finished: events sent: " << events_sent << std::endl;
}


/*
        for (unsigned int i = 0; i < t->GetNumEvents(); ++i) {
            msg = t->GetEvent(i);
            if (msg.GetTime() >= GetCurrentMIDIClockTime())
                break;

            if (msg.IsSysEx() &&
                !(msg.GetSysEx()->IsGMReset() || msg.GetSysEx()->IsGSReset() || msg.GetSysEx()->IsXGReset())) {
                OutputMessage(msg, port);
                events_sent++;
            }
        }

        int channel = GetTrackChannel(trk);
        if (channel != -1) {
            // set the current program
            msg.SetProgramChange(channel, state.program);
            OutputMessage(msg, port);
            // set the current pitch bend value
            msg.SetPitchBend(channel, state.bender_value);
            OutputMessage(msg, port);
            events_sent += 2;
            // set the controllers
            for (unsigned int i = 0; i < C_ALL_NOTES_OFF; i++) {
                if (state.control_values[i] != -1) {
                    msg.SetControlChange(channel, i, state.control_values[i]);
                    OutputMessage(msg, port);
                    events_sent++;
                }   // TODO: RPN and NRPN
            }
        }
    }

    MIDIManager::CloseOutPorts();
    std::cout << "CatchEventsBefore finished: events sent: " << events_sent << std::endl;

}
*/

/*
void AdvancedSequencer::InternalStart() {
    if (!IsPlaying()) {
        std::cout << "\t\tExecuting AdvancedSequencer::InternalStart() ..." << std::endl;
        SetTimeShiftMode(true);
        MIDITickComponent::Start((tMsecs)GetCurrentTimeMs());
    }
}


void AdvancedSequencer::InternalStop() {
    if (IsPlaying()) {
        std::cout << "\t\tExecuting AdvancedSequencer::InternalStop() ..." << std::endl;
        MIDITickComponent::Stop();
        SetTimeShiftMode(false);
        MIDIManager::AllNotesOff();
    }
}
*/
/*
void AdvancedSequencer::AutoStopProc(void* p) {
    std::cout << "\t\tEntered in AdvancedSquencer::AutoStopProc ...\n";
    AdvancedSequencer* seq = static_cast<AdvancedSequencer *>(p);
    seq->Stop();
    std::cout << "\t\tExiting from AdvancedSequencer::AutoStopProc()\n";
}
*/
