/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2021  Nicola Cassetta
 *   https://github.com/ncassetta/NiCMidi
 *
 *   This file is part of NiCMidi.
 *
 *   NiCMidi is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   NiCMidi is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with NiCMidi.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "../include/advancedsequencer.h"
#include "../include/manager.h"

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
// sets warp_positions and num_measures (needed even if multitrack is empty, otherwise warp_position would be empty)
    ExtractWarpPositions(); 
    /* *///FCKX
    if (MIDIManager::IsValidInPortNumber(0)) {
        thru = new MIDIThru();
        thru_transposer = new MIDIProcessorTransposer();
        thru->SetProcessor(thru_transposer);
        MIDIManager::AddMIDITick(thru);
    }
    /* */
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

    // sets the embedded MIDIThru only if the system has almost an in port
    if (MIDIManager::IsValidInPortNumber(0)) {
        thru = new MIDIThru();
        thru_transposer = new MIDIProcessorTransposer();
        thru->SetProcessor(thru_transposer);
        MIDIManager::AddMIDITick(thru);
    }
    for (unsigned  int i = 0; i < GetNumTracks(); ++i)
        track_processors[i] = new MIDISequencerTrackProcessor;
}


AdvancedSequencer::~AdvancedSequencer() {
    Stop();
    if (thru) {
        delete thru;            // removes thru from the manager queue
        delete thru_transposer;
    }
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
    for (unsigned  int i = 0; i < GetNumTracks(); ++i) {    // MIDISequencer::Reset() deletes the processors
        track_processors[i] = new MIDISequencerTrackProcessor;
        // causes a call to Analyze()
        GetTrack(i)->GetStatus();
    }
    file_loaded = !state.multitrack->IsEmpty();      // the multitrack is not cleared by this
    ExtractWarpPositions();
}


bool AdvancedSequencer::Load (const char *fname) {
    Stop();
    // copy the old multitrack in case we need to rstore it
    MIDIMultiTrack undo_multi = *state.multitrack;
    //state.multitrack->Reset();    this is done by LoadMIDIFile

    bool load = LoadMIDIFile(fname, state.multitrack, &header);
    if(!load)
        *state.multitrack = undo_multi;
    Reset();                    // synchronizes the sequencer with the multitrack, goes to 0, calls
                                // ExtractWarpPositions() and sets file_loaded
    // UpdateStatus();
    return load;                // file_loaded could be true if load fails and a file was already loaded
}


bool AdvancedSequencer::Load (const MIDIMultiTrack* tracks) {
    Stop();
    *state.multitrack = *tracks;
    //file_loaded = !state.multitrack->IsEmpty();   this is done by Reset
    Reset();                    // synchronizes the sequencer with the multitrack and goes to 0
    //UpdateStatus();             // all done by Reset()
    return true;
}


void AdvancedSequencer::UnLoad() {
    Stop();
    state.multitrack->Reset();
    //file_loaded = false;      // done by reset
    Reset();
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


std::string AdvancedSequencer::GetTrackName (unsigned int trk_num) const {
    if (!file_loaded)
        return "";
    return GetTrackState(trk_num)->track_name;
}


char AdvancedSequencer::GetTrackVolume (unsigned int trk_num) const {
    if (!file_loaded)
        return 100;
    return GetTrackState(trk_num)->control_values[C_MAIN_VOLUME];
}


char AdvancedSequencer::GetTrackProgram (unsigned int trk_num) const {
    if (!file_loaded)
        return 0;
    return GetTrackState(trk_num)->program;
}


int AdvancedSequencer::GetTrackNoteCount (unsigned int trk_num) const {
    if (!file_loaded || !IsPlaying())
        return 0;
    else
        return GetTrackState(trk_num)->note_matrix.GetTotalCount();
}


unsigned int AdvancedSequencer::GetTrackVelocityScale (unsigned int trk_num) const {
    if (!file_loaded)
        return 100;
    return ((MIDISequencerTrackProcessor *)track_processors[trk_num])->velocity_scale;
}


int AdvancedSequencer::GetTrackRechannelize (unsigned int trk_num) const {
    if (!file_loaded)
        return -1;
    return GetTrackProcessor(trk_num)->rechannel;
}


int AdvancedSequencer::GetTrackChannel (unsigned int trk_num) {
    if (!file_loaded)
        return -1;
    return (GetTrackProcessor(trk_num)->rechannel == -1 ?
            GetTrack(trk_num)->GetChannel() : GetTrackProcessor(trk_num)->rechannel);
}


int AdvancedSequencer::GetTrackTranspose (unsigned int trk_num) const {
    if (!file_loaded)
        return 0;
    return GetTrackProcessor(trk_num)->transpose;
}


int AdvancedSequencer::GetTrackTimeShift (unsigned int trk_num) const {
    if (!file_loaded)
        return 0;
    return GetTrackTimeShift(trk_num);
}


bool AdvancedSequencer::SetMIDIThruEnable(bool on_off) {
    if (!thru)
        return false;
    if (on_off)
        thru->Start();
    else
        thru->Stop();            // calls AllNotesOff() on thru out channel
    return true;
}


bool AdvancedSequencer::SetMIDIThruChannel (char chan) {
    return (thru != 0 && thru->SetOutChannel(chan));
}


bool AdvancedSequencer::SetMIDIThruTranspose (char amt) {
    if (thru_transposer) {
        thru_transposer->SetAllTranspose (amt);
        MIDIManager::AllNotesOff();
        return true;
    }
    return false;
}


/* NOTE BY NC: soloing and muting has been enhanced to keep count of muted CC, SYSEX changes previously muted
 * so when we unmute a track it sounds with correct parameters even if they weren't tramsmitted before
 * So member function are changed
 */


bool AdvancedSequencer::SetTrackSolo (unsigned int trk_num) {   // unsoloing done by UnSoloTrack()
    if (!file_loaded || !state.multitrack->IsValidTrackNumber(trk_num))
        return false;
    proc_lock.lock();
    for (unsigned int i = 0; i < GetNumTracks(); ++i ) {
        if(i == trk_num) {
            ((MIDISequencerTrackProcessor *)track_processors[i])->solo = MIDISequencerTrackProcessor::SOLOED;
            if (IsPlaying())
                // track could be muted before soloing: this sets appropriate CC, PC, etc
                // not previously sent
                CatchEventsBefore(trk_num);     // MUST be here! We must previously unmute the track!
        }
        else {
            ((MIDISequencerTrackProcessor *)track_processors[i])->solo = MIDISequencerTrackProcessor::NOT_SOLOED;
            if (IsPlaying() && GetTrackChannel(i) != -1) {
                MIDIManager::GetOutDriver(GetTrackOutPort(trk_num))->AllNotesOff(GetTrackChannel(i));
                GetTrackState(i)->note_matrix.Reset();
            }
        }
    }
    proc_lock.unlock();
    return true;
}


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


bool AdvancedSequencer::SetTrackMute (unsigned int trk_num, bool f) {
    if (!file_loaded || !state.multitrack->IsValidTrackNumber(trk_num))
        return false;
    proc_lock.lock();
    GetTrackProcessor(trk_num)->mute = f;
    int channel = GetTrackChannel(trk_num);
    if (IsPlaying() && channel != -1) {
        if(f) {
            MIDIManager::GetOutDriver(GetTrackOutPort(trk_num))->AllNotesOff(channel);
            GetTrackState(trk_num)->note_matrix.Reset();
        }
        else
            // track was muted: this set appropriate CC, PC, etc not previously sent
            CatchEventsBefore(trk_num);
    }
    proc_lock.unlock();
    return true;
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


bool AdvancedSequencer::SetTrackVelocityScale (unsigned int trk_num, unsigned int scale) {
    if (!file_loaded || !state.multitrack->IsValidTrackNumber(trk_num))
        return false;
    proc_lock.lock();
    ((MIDISequencerTrackProcessor *)track_processors[trk_num])->velocity_scale = scale;
    proc_lock.unlock();
    return true;
}


bool AdvancedSequencer::SetTrackRechannelize (unsigned int trk_num, char chan) {
    if (!file_loaded || !state.multitrack->IsValidTrackNumber(trk_num))
        return false;
    proc_lock.lock();
    if (IsPlaying() && GetTrackChannel(trk_num) != chan && !(GetTrackChannel(trk_num) == -1)) {
        MIDIManager::GetOutDriver(GetTrackOutPort(trk_num))->AllNotesOff(GetTrackChannel(trk_num));
        GetTrackState(trk_num)->note_matrix.Reset();
    }
    GetTrackProcessor(trk_num)->rechannel = chan;
    proc_lock.unlock();
    return true;
}


bool AdvancedSequencer::SetTrackTranspose (unsigned int trk_num, char amt) {
    if (!file_loaded || !state.multitrack->IsValidTrackNumber(trk_num))
        return false;
    proc_lock.lock();
    if (IsPlaying() && GetTrackTranspose(trk_num) != amt && !(GetTrackChannel(trk_num) == -1)) {
        MIDIManager::GetOutDriver(GetTrackOutPort(trk_num))->AllNotesOff(GetTrackChannel(trk_num));
        GetTrackState(trk_num)->note_matrix.Reset();
    }
    GetTrackProcessor(trk_num)->transpose = amt;
    proc_lock.unlock();
    return true;
}


bool AdvancedSequencer::GoToTime (MIDIClockTime time_clk) {
    bool ret;

    // figure out which warp item we use
    // try warp to the last warp point BEFORE the
    // requested time //measure
    unsigned int warp_to_item = 0;
    for (; warp_to_item < warp_positions.size() - 1; warp_to_item++) {
        if (warp_positions[warp_to_item + 1].cur_clock > time_clk)
            break;
    }

    SetState (&warp_positions[warp_to_item]);
    ret = MIDISequencer::GoToTime (time_clk);
    if (ret) {              // we have effectively moved time
        if (IsPlaying())

            CatchEventsBefore();


    else
        for (unsigned int i = 0; i < GetNumTracks(); ++i)
            GetTrackState(i)->note_matrix.Reset();
 

 }
     proc_lock.unlock();
    return ret;
}


bool AdvancedSequencer::GoToTimeMs(float time_ms) {
    bool ret;
     proc_lock.lock();
    // figure out which warp item we use
    // try warp to the last warp point BEFORE the
    // requested time //measure
    unsigned int warp_to_item = 0;
   for (; warp_to_item < warp_positions.size() - 1; warp_to_item++) {
        if (warp_positions[warp_to_item + 1].cur_time_ms > time_ms)
            break;
    }
    
    SetState (&warp_positions[warp_to_item]);
    ret = MIDISequencer::GoToTimeMs (time_ms);
    if (ret) {              // we have effectively moved time
        if (IsPlaying())

  CatchEventsBefore();
  
else
 for (unsigned int i = 0; i < GetNumTracks(); ++i)
                GetTrackState(i)->note_matrix.Reset();
    }
        proc_lock.unlock();
    return ret;
}



bool AdvancedSequencer::GoToMeasure (int measure, int beat) {
    bool ret;           
    proc_lock.lock();
    // figure out which warp item we use
    // try warp to the last warp point BEFORE the
    // requested measure
    unsigned int warp_to_item = measure / MEASURES_PER_WARP;

    if (warp_to_item >= warp_positions.size())
        warp_to_item = warp_positions.size() - 1;

    SetState (&warp_positions[warp_to_item]);
    ret = MIDISequencer::GoToMeasure (measure, beat);
    if (ret) {                  // we have effectively moved time
        if (IsPlaying())
        
            CatchEventsBefore();

          else
        
            for (unsigned int i = 0; i < GetNumTracks(); ++i)
                GetTrackState (i)->note_matrix.Reset();
    }
     proc_lock.unlock();
    return ret;
}


void AdvancedSequencer::Start () {
    // If you call this while already playing the sequencer will restart from the
    // loop initial measure (if repeat play is on)
    if (!file_loaded && play_mode == PLAY_BOUNDED) {
        return;
    }
    
    proc_lock.lock();
    std::cout << "\t\tEntered in AdvancedSequencer::Start() ...\n";
    MIDISequencer::Stop();
    if (repeat_play_mode)
        GoToMeasure (repeat_start_meas);

    MIDIManager::OpenOutPorts();
    // this intercepts any CC, SYSEX and TEMPO messages and send them to the out port
    // allowing to start with correct values; we could incorporate this in the
    // sequencer state, but it would track even CC (not difficult) and SYSEX messages
    CatchEventsBefore();

    state.iterator.SetTimeShiftMode(true);
    if (GetCountInEnable())
            CountInPrepare();
        else
            state.Notify (MIDISequencerGUIEvent::GROUP_TRANSPORT,
                          MIDISequencerGUIEvent::GROUP_TRANSPORT_START);

    SetDevOffset((tMsecs)GetCurrentTimeMs());
    MIDITickComponent::Start();
    std::cout << "\t\t ... Exiting from AdvancedSequencer::Start()" << std::endl;
    //std::cout << "sys_time_offset = " << sys_time_offset << " sys_time = " << MIDITimer::GetSysTimeMs() << std::endl;
  proc_lock.unlock();
}


void AdvancedSequencer::Stop() {
    if (!IsPlaying())
        return;

    proc_lock.lock();
    std::cout << "\t\tEntered in AdvancedSequencer::Stop() ...\n";
    // waits until the timer thread has stopped
    MIDITickComponent::Stop();
    // resets the autostop flag
    state.playing_status &= ~AUTO_STOP_PENDING;
    state.iterator.SetTimeShiftMode(time_shift_mode);
    MIDIManager::AllNotesOff();
    MIDIManager::CloseOutPorts();
    state.Notify (MIDISequencerGUIEvent::GROUP_TRANSPORT,
                  MIDISequencerGUIEvent::GROUP_TRANSPORT_STOP);
    // stops on a beat (and clear midi matrix)
    GoToMeasure(state.cur_measure, state.cur_beat);
    std::cout << "\t\t ... Exiting from AdvancedSequencer::Stop()" << std::endl;
    proc_lock.unlock();
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


bool AdvancedSequencer::SetSMPTE(SMPTE* s) {
    if (!file_loaded || IsPlaying())
        return false;

    const MIDIMessage* msg = 0;

    // search a SMPTE event in track 0, time 0
    const MIDITrack* trk = GetTrack(0);
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
    return true;
}


void AdvancedSequencer::UpdateStatus() {
    proc_lock.lock();
    file_loaded = !GetMultiTrack()->IsEmpty();
    ExtractWarpPositions();
     //GoToTime(state.cur_clock);     //   done by ExtractWarpPositions()
    proc_lock.unlock();
}



//
// protected members
//


void AdvancedSequencer::ExtractWarpPositions() {
    //MIDISequencerGUINotifier* notifier = state.notifier;

    Stop();         //TODO: this forbids to edit the multitrack while the sequencer is playing
                    // is this right?
    // warp_positions is now a vector of objects ( not pointers ) so we can minimize memory dealloc/alloc

    MIDIClockTime cur_time = GetCurrentMIDIClockTime();

    // temporarily disable the gui notifier
  //  bool notifier_mode = false;
  //  if (notifier) {
  //      notifier_mode = notifier->GetEnable();
  //      notifier->SetEnable (false);
  //  }
    char old_play_mode = play_mode;
    play_mode = PLAY_BOUNDED;

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
           warp_positions.resize(num_warp_positions, MIDISequencerState(GetMultiTrack(), state.notifier));
   
   // now find the actual number of measures
    num_measures = (num_warp_positions - 1) * MEASURES_PER_WARP;
    while (MIDISequencer::GoToMeasure(num_measures + 1))
        num_measures++;

    GoToTime(cur_time);

    play_mode = old_play_mode;
    // re-enable the gui notifier if it was enabled previously
//    if (notifier) {
//        notifier->SetEnable (notifier_mode);
        // cause a full gui refresh now
        //notifier->Notify (MIDISequencerGUIEvent::GROUP_ALL);
   // }
}


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
        trk = GetTrack(i);
        if (!(trk->HasSysex())) continue;
        msg.SetTime(0);
        port = GetTrackOutPort(i);
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
        trk = GetTrack(i);
        if (!GetTrackMute(i) &&
            (trk->GetType() == MIDITrack::TYPE_CHAN || trk->GetType() == MIDITrack::TYPE_IRREG_CHAN)) {
            int channel = GetTrackChannel(i);
            port = GetTrackOutPort(i);
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
        trk = GetTrack(i);
        if (trk->GetType() == MIDITrack::TYPE_MIXED_CHAN) {
            port = GetTrackOutPort(i);
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


void AdvancedSequencer::CatchEventsBefore(int trk_num) {
    MIDITimedMessage msg;
    MIDITrack* trk = GetTrack(trk_num);
    unsigned int port = GetTrackOutPort(trk_num);
    int events_sent = 0;

    if (GetCurrentMIDIClockTime() == 0)         // nothing to do
        return;
    std::cout << "Catch events before started for track " << trk_num << " ..." << std::endl;

    MIDIManager::OpenOutPorts();

    //first send sysex (but not reset ones)
    if (trk->HasSysex()) {
        msg.SetTime(0);
        for (unsigned int i = 0; i < trk->GetNumEvents() && msg.GetTime() <= GetCurrentMIDIClockTime(); i++) {
            msg = trk->GetEvent(i);
            if (msg.IsSysEx() &&
                !(msg.GetSysEx()->IsGMReset() || msg.GetSysEx()->IsGSReset() || msg.GetSysEx()->IsXGReset())) {

                OutputMessage(msg, port);
                events_sent++;
            }
        }
    }


    // restore program, pitch bend and controllers as registered in the sequencer state
    if (!GetTrackMute(trk_num)) {
        // if this is an ordinary channel track ...
        if (trk->GetType() == MIDITrack::TYPE_CHAN || trk->GetType() == MIDITrack::TYPE_IRREG_CHAN) {
            int channel = GetTrackChannel(trk_num);     // takes into account rechannelize
            const MIDISequencerTrackState* trk_state = GetTrackState(trk_num);
            // set the current program
            if (trk_state->program != -1) {
                msg.SetProgramChange(channel, trk_state->program);
                OutputMessage(msg, port);
                events_sent++;
            }
            // set the current pitch bend value
            msg.SetPitchBend(channel, trk_state->bender_value);
            OutputMessage(msg, port);
            events_sent ++;
            // set the controllers
            for (unsigned int j = 0; j < C_ALL_NOTES_OFF; j++) {
                if (trk_state->control_values[j] != -1) {
                    msg.SetControlChange(channel, j, trk_state->control_values[j]);
                    OutputMessage(msg, port);
                    events_sent++;
                }   // TODO: RPN and NRPN
            }
        }
        // if the track has mixed channels send all previous messages
        else if (trk->GetType() == MIDITrack::TYPE_MIXED_CHAN) {
            msg.SetTime(0);
            for (unsigned int i = 0; i < trk->GetNumEvents() && msg.GetTime() <= GetCurrentMIDIClockTime(); i++) {
                msg = trk->GetEvent(i);
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


