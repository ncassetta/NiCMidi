/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
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


#include "../include/recorder.h"
#include "../include/manager.h"


////////////////////////////////////////////////////////////////////////////
//                         class RecNotifier                              //
////////////////////////////////////////////////////////////////////////////

RecNotifier::RecNotifier(MIDISequencer* seq) :
    MIDISequencerGUINotifier(seq),
    meas_note(DEFAULT_MEAS_NOTE), beat_note(DEFAULT_BEAT_NOTE),
    port(0), chan(9), num_beats(1), other_notifier(0) {
    on_msg.SetNoteOn(chan, 0, 100);
    off_msg.SetNoteOn(chan, 0, 0);
}

void RecNotifier::Notify(const MIDISequencerGUIEvent &ev) {
    if (en) {               // Metronome enabled
        if (ev.GetGroup() == MIDISequencerGUIEvent::GROUP_TRANSPORT) {
            if (ev.GetItem() == MIDISequencerGUIEvent::GROUP_TRANSPORT_MEASURE) {
                MIDIManager::GetOutDriver(port)->OutputMessage(off_msg);
                on_msg.SetNote(meas_note);
                off_msg.SetNote(meas_note);
                on_msg.SetChannel(chan);
                off_msg.SetChannel(chan);
                MIDIManager::GetOutDriver(port)->OutputMessage(on_msg);
                num_beats = 0;
                //std::cout << "Meas" << std::endl;
            }
            else if (ev.GetItem() == MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT) {
                if (num_beats != 0) {
                    MIDIManager::GetOutDriver(port)->OutputMessage(off_msg);
                    on_msg.SetNote(beat_note);
                    off_msg.SetNote(beat_note);
                    //std::cout << "Beat" << std::endl;
                    on_msg.SetChannel(chan);
                    off_msg.SetChannel(chan);
                    MIDIManager::GetOutDriver(port)->OutputMessage(on_msg);
                }
                num_beats++;
            }
        }
    }
    // always send the event to other notifier (could be enabled)
    if (other_notifier)
        other_notifier->Notify(ev);
}

////////////////////////////////////////////////////////////////////////////
//                        class MIDIRecorder                              //
////////////////////////////////////////////////////////////////////////////


MIDIRecorder::MIDIRecorder(MIDISequencer* const s) :
    MIDITickComponent(PR_POST_SEQ, StaticTickProc),
    seq(s), rec_start_time(0), rec_end_time(TIME_INFINITE),
    rec_mode(REC_OVER), notifier(s),
    old_seq_mode(MIDISequencer::PLAY_BOUNDED),
    rec_on(false)
{
    //check if an in port exists
    //if (!MIDIManager::IsValidInPortNumber(0))
        //throw RtMidiError("MIDIRecorder needs almost a MIDI in port in the system\n", RtMidiError::INVALID_DEVICE);
    tracks = new MIDIMultiTrack();
    seq_tracks = s->GetState()->multitrack;
}


MIDIRecorder::~MIDIRecorder() {
    Stop();
    while (!undo_stack.empty()) {
        MIDIMultiTrack* t = undo_stack.top();
        undo_stack.pop();
        delete t;
    }
    delete tracks;
}


void MIDIRecorder::Reset() {
    tracks->Reset();
    tracks->SetClksPerBeat(seq_tracks->GetClksPerBeat());
    rec_start_time = 0;
    rec_end_time = TIME_INFINITE;
    en_tracks.resize(0);
    if (seq->GetState()->notifier)
        seq->GetState()->Notify(MIDISequencerGUIEvent::GROUP_RECORDER,
                                MIDISequencerGUIEvent::GROUP_RECORDER_RESET);
}


bool MIDIRecorder::SetTrackInPort(unsigned int trk_num, unsigned int port) {
    if (IsPlaying() || !seq_tracks->IsValidTrackNumber(trk_num) || !MIDIManager::IsValidInPortNumber(port))
        return false;
    seq_tracks->GetTrack(trk_num)->SetInPort(port);
    if (tracks->IsValidTrackNumber(trk_num))
        tracks->GetTrack(trk_num)->SetInPort(port);
    return true;
}


bool MIDIRecorder::SetTrackRecChannel(unsigned int trk_num, int chan) {
    if (IsPlaying() || !tracks->IsValidTrackNumber(trk_num) || (chan < -1 || chan > 15))
        return false;
    seq_tracks->GetTrack(trk_num)->SetRecChannel(chan);
    if (tracks->IsValidTrackNumber(trk_num))
        tracks->GetTrack(trk_num)->SetRecChannel(chan);
    return true;
}


bool MIDIRecorder::SetRecMode(int mode) {
    if (IsPlaying())
        return false;
    if (mode == REC_MERGE || mode == REC_OVER) {
        rec_mode = mode;
        return true;
    }
    return false;
}


bool MIDIRecorder::SetStartRecTime(MIDIClockTime t) {
    if (IsPlaying())
        return false;
    rec_start_time = t;
    if (rec_end_time < t)
        rec_end_time = t;
    return true;
}


bool MIDIRecorder::SetEndRecTime(MIDIClockTime t) {
    if (IsPlaying())
        return false;
    rec_end_time = t;
    if (rec_start_time > t)
        rec_start_time = t;
    return true;
}



bool MIDIRecorder::InsertTrack(int trk_num) {
    if (IsPlaying())
        return false;
    proc_lock.lock();
    if ((int)en_tracks.size() > trk_num && trk_num != -1){
        tracks->InsertTrack(trk_num);
        en_tracks.insert(en_tracks.begin() + trk_num, false);
    }
    proc_lock.unlock();
    return true;
}


bool MIDIRecorder::DeleteTrack(int trk_num) {
    if (IsPlaying())
        return false;
    proc_lock.lock();
    if (tracks->DeleteTrack(trk_num))
        en_tracks.erase(en_tracks.begin() + trk_num);
    proc_lock.unlock();
    return true;
}


bool MIDIRecorder::MoveTrack(int from, int to) {
    if (from == to) return true;                        // nothing to do
    if (IsPlaying())
        return false;
    proc_lock.lock();
    bool has_from = tracks->IsValidTrackNumber(from);
    bool temp_en = false;
    MIDITrack temp_track;
    if (has_from) {
        temp_en = en_tracks[from];
        temp_track = *tracks->GetTrack(from);
        en_tracks.erase(en_tracks.begin() + from);
        tracks->DeleteTrack(from);
    }
    if (from < to)
        to--;
    bool has_to = tracks->IsValidTrackNumber(to);
    if (has_to) {
        en_tracks[to] = temp_en;
        *tracks->GetTrack(to) = temp_track;
    }
    else if (temp_en == true) {
        ResizeTracks(to + 1);
        en_tracks[to] = temp_en;
        *tracks->GetTrack(to) = temp_track;
    }
    proc_lock.unlock();
    return true;
}


bool MIDIRecorder::EnableTrack(unsigned int trk_num) {
    if (!seq->GetMultiTrack()->IsValidTrackNumber(trk_num) || IsPlaying())
        return false;
    // if the recorder multitrack doesn't have enough tracks add them
    if (!tracks->IsValidTrackNumber(trk_num))
        ResizeTracks(trk_num + 1);
    else
        tracks->GetTrack(trk_num)->Clear();
    en_tracks[trk_num] = true;
    return true;
}


bool MIDIRecorder::DisableTrack(unsigned int trk_num) {
    if (!tracks->IsValidTrackNumber(trk_num) || IsPlaying())
        return false;
    en_tracks[trk_num] = 0;
    ResizeTracks(tracks->GetNumTracks());
    return true;
}


bool MIDIRecorder::UndoRec() {
    if (!undo_stack.empty()) {
        const MIDIMultiTrack* undo_tracks = undo_stack.top();
        for (unsigned int i = 0; i < undo_tracks->GetNumTracks(); i++)
            if (!undo_tracks->GetTrack(i)->IsEmpty())
                seq_tracks->SetTrack(undo_tracks->GetTrack(i) , i);
        undo_stack.pop();
        if (seq->GetState()->notifier)
            seq->GetState()->Notify(MIDISequencerGUIEvent::GROUP_ALL);
        delete undo_tracks;
        return true;
    }
    return false;
}






// Inherited from MIDITICK

void MIDIRecorder::Start() {
    if (!IsPlaying()) {
        std::cout << "\t\tEntered in MIDIRecorder::Start() ..." << std::endl;
        MIDIManager::OpenInPorts();
        MIDIMultiTrack* undo_multi = new MIDIMultiTrack(en_tracks.size(), seq_tracks->GetClksPerBeat());
        for (unsigned int i = 0; i < en_tracks.size(); i++) {
            if (en_tracks[i]) {
                undo_multi->SetTrack(seq_tracks->GetTrack(i), i);
                PrepareTrack(i);
                if (rec_mode == REC_OVER)
                    seq_tracks->SetTrack(tracks->GetTrack(i), i);
            }
        }
        undo_stack.push(undo_multi);
        rec_on.store(false);            // will be set to true by the static StaticProc()
        SetSeqNotifier();
        old_seq_mode = seq->GetPlayMode();
        seq->SetPlayMode(MIDISequencer::PLAY_UNBOUNDED);
        seq->SetCountIn(true);
        seq->Start();
        SetDevOffset(seq->GetDevOffset());
        MIDITickComponent::Start();
        std::cout << "\t\t ... Exiting from MIDIRecorder::Start()" << std::endl;
    }
}


void MIDIRecorder::Stop() {
    if (IsPlaying()) {
        std::cout << "\t\tEntered in MIDIRecorder::Stop() ..." << std::endl;
        if (rec_on.load() == true) {
            MIDISequencerGUIEvent ev = MIDISequencerGUIEvent(MIDISequencerGUIEvent::GROUP_RECORDER,
                                                             0,
                                                             MIDISequencerGUIEvent::GROUP_RECORDER_STOP);
            notifier.Notify(ev);
            rec_on.store(false);
        }
        MIDITickComponent::Stop();
        MIDIManager::CloseInPorts();
        seq->MIDISequencer::Stop();         //AdvancedSequencer calls GoToMeasure()
        seq->SetCountIn(false);
        ResetSeqNotifier();
        for (unsigned int i = 0; i < en_tracks.size(); i++) {
            if (en_tracks[i]) {
                tracks->GetTrack(i)->CloseOpenEvents(rec_start_time, rec_end_time);
                seq_tracks->SetTrack(tracks->GetTrack(i), i);
            }
        }
        seq->UpdateStatus();
        seq->SetPlayMode(old_seq_mode);
        //stops the sequencer on a beat
        seq->GoToMeasure(seq->GetCurrentMeasure(), seq->GetCurrentBeat());
        std::cout << "\t\t ... Exiting from MIDIRecorder::Stop()" << std::endl;
    }
}


void MIDIRecorder::ResizeTracks(unsigned int new_size) {
    if (new_size <= en_tracks.size()) {
        int max_enabled = -1;
        for (unsigned int i = 0; i < new_size; i++)
            if (en_tracks[i])
                max_enabled = i;
        if (max_enabled < (int)en_tracks.size() - 1)
            while ((int)tracks->GetNumTracks() > max_enabled + 1)
                tracks->DeleteTrack();
        en_tracks.resize(max_enabled + 1);
    }
    else {
        while (tracks->GetNumTracks() < new_size)
            tracks->InsertTrack();
        en_tracks.resize(new_size, false);
    }
}


void MIDIRecorder::SetSeqNotifier() {
    notifier.SetOtherNotifier(seq->GetState()->notifier);
    seq->GetState()->notifier = &notifier;
}


void MIDIRecorder::ResetSeqNotifier() {
    seq->GetState()->notifier = notifier.GetOtherNotifier();
}


void MIDIRecorder::PrepareTrack(unsigned int trk_num) {
    bool has_prog, has_vol = false;
    MIDITrack* track = tracks->GetTrack(trk_num);
    MIDITimedMessage* msg_ptr;

    // copy the sequencer track into the recorder multitrack
    *track = *seq_tracks->GetTrack(trk_num);
    // truncate notes, pedaland pitch bend at rec_start_time
    if (rec_start_time > 0)
        track->CloseOpenEvents(0, rec_start_time);
    // truncate notes, pedal and pitch bend at rec_end_time (so we
    // don't leave note off messages after rec_end_time)
    if (rec_end_time <= track->GetEndTime())
        track->CloseOpenEvents(rec_start_time, rec_end_time);
    // delete all channel events in the recording area
    // start at i = 1 because the i-- at the line below
    for (unsigned int i = 1; i <= track->GetNumEvents(); i++) {
        msg_ptr = track->GetEventAddress(i - 1);
        if (msg_ptr->GetTime() >= rec_end_time)
            break;
        if (msg_ptr->GetTime() >= rec_start_time) {
            if (msg_ptr->IsProgramChange() && !has_prog) {
                has_prog = true;
                continue;
            }
            else if (msg_ptr->IsVolumeChange() && !has_vol) {
                has_vol = true;
                continue;
            }
            if (msg_ptr->IsChannelMsg() && track->DeleteEvent(*msg_ptr))
                i--;
        }
        else {
            if (msg_ptr->IsProgramChange() && !has_prog)
                has_prog = true;
            else if (msg_ptr->IsVolumeChange() && !has_vol)
                has_vol = true;
        }
    }
    en_ports.insert(track->GetInPort());
}


void MIDIRecorder::StaticTickProc(tMsecs sys_time, void* pt) {
    MIDIRecorder* seq_pt = static_cast<MIDIRecorder *>(pt);
    seq_pt->TickProc(sys_time);
}



// The idea of writing new messages directly into the sequencer (and not the recorder)
// must be discarded because in recording you are messing sequencer tracks while playing
// them (you should call MIDISequencer::UpdateStatus() at every MIDIRecorder::TickProc())
// TODO: perhaps it is possible to write a Sequencer::InsertEvent() method

void MIDIRecorder::TickProc(tMsecs sys_time) {
    //static unsigned int times;
    //times++;
    //if (!(times % 100))
        //std::cout << "MIDIRecorder::TickProc() " << times << " times" << std::endl;

    // the sequencer is counting in, nothing to do for the recorder
    if (seq->GetCountInPending())
        return;

    proc_lock.lock();
    MIDIClockTime cur_time = seq->GetCurrentMIDIClockTime();
    // we are recording
    if (cur_time >= rec_start_time && cur_time < rec_end_time) {    // TODO or <= rec_end_rime ??
        // if this is the first time send a message to the GUI
        if (rec_on.load() == false) {
            MIDISequencerGUIEvent ev(MIDISequencerGUIEvent::GROUP_RECORDER,
                                     0,
                                     MIDISequencerGUIEvent::GROUP_RECORDER_START);
            notifier.Notify(ev);
            rec_on.store(true);
        }
        MIDIRawMessage rmsg;

        //tMsecs cur_time = sys_time - sys_time_offset + rec_time_offset;
        //float clocks_per_ms = (tempobpm * multitrack->GetClksPerBeat()) / 60000.0;

        // collect messages incoming from MIDI in ports
        for (unsigned int i = 0; i < MIDIManager::GetNumMIDIIns(); i++) {
            if (en_ports.count(i) == 0)
                continue;
            MIDIInDriver* port = MIDIManager::GetInDriver(i);
            port->LockQueue();
            for (unsigned int j = 0, out_count = 0; j < port->GetQueueSize() && out_count < 100; j++, out_count++) {
                port->ReadMessage(rmsg, j);
                MIDITimedMessage msg(rmsg.msg);
                msg.SetTime(cur_time);
                if (msg.IsChannelMsg()) {
                    // search among the tracks which can accept the message
                    signed char ch1 = msg.GetChannel();
                    for (i = 0; i < tracks->GetNumTracks(); i++) {
                        if (!en_tracks[i]) continue;
                        signed char ch2 = tracks->GetTrack(i)->GetRecChannel();
                        if (ch1 == ch2 || ch2 == -1) {
                            // insert the event into the track
                            tracks->InsertEvent(i, msg);
                        }
                        // tell the driver to send this message
                        MIDIManager::GetOutDriver(tracks->GetTrack(i)->GetOutPort())->OutputMessage(msg);
                    }
                    //if ((*en_ports[i])[ch] != 0)
                    //    (*en_ports[i])[ch]->PushEvent(msg);
                    // std::cout << "Added MIDI channel message to track " << std::endl;
                }
                else
                    tracks->GetTrack(0)->PushEvent(msg);
            }
            port->UnlockQueue();
        }
    }
    // we are after the rec end time
    else if (cur_time >= rec_end_time) {
        // if this is the first time send a message to the GUI
        if (rec_on.load() == true) {
            MIDISequencerGUIEvent ev(MIDISequencerGUIEvent::GROUP_RECORDER,
                                     0,
                                     MIDISequencerGUIEvent::GROUP_RECORDER_STOP);
            notifier.Notify(ev);
            rec_on.store(false);
        }
    }
    proc_lock.unlock();
}





