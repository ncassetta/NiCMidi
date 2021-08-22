/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2020  Nicola Cassetta
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
    port(0), chan(9) {
    on_msg.SetNoteOn(chan, 0, 100);
    off_msg.SetNoteOn(chan, 0, 0);
}

void RecNotifier::Notify(const MIDISequencerGUIEvent &ev) {
// reworked with an unique call to ost <<, so that there's no trouble with
// cout call in other threads. (Crashed???)
    if (sequencer == 0) return;
    if (en) {               // Metronome enabled
        if (ev.GetGroup() == MIDISequencerGUIEvent::GROUP_TRANSPORT) {
            MIDIManager::GetOutDriver(port)->OutputMessage(off_msg);
            if (ev.GetItem() == MIDISequencerGUIEvent::GROUP_TRANSPORT_MEASURE) {
                on_msg.SetNote(meas_note);
                off_msg.SetNote(meas_note);
                std::cout << "Meas" << std::endl;
            }
            else if (ev.GetItem() == MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT) {
                on_msg.SetNote(beat_note);
                off_msg.SetNote(beat_note);
                std::cout << "Beat" << std::endl;
            }
            on_msg.SetChannel(chan);
            off_msg.SetChannel(chan);
            MIDIManager::GetOutDriver(port)->OutputMessage(on_msg);
        }
    }
    if (other_notifier)
        other_notifier->Notify(ev);
}

////////////////////////////////////////////////////////////////////////////
//                        class MIDIRecorder                              //
////////////////////////////////////////////////////////////////////////////


MIDIRecorder::MIDIRecorder(MIDISequencer* const s) :
    MIDITickComponent(PR_POST_SEQ, StaticTickProc),
    seq(s), rec_start_time(0), rec_end_time(TIME_INFINITE),
    rec_on(false), notifier(s)
{
    tracks = new MIDIMultiTrack();
    seq_tracks = s->GetState()->multitrack;
}


MIDIRecorder::~MIDIRecorder() {
    Stop();
    delete tracks;
}


void MIDIRecorder::Reset() {
    tracks->Reset();
    tracks->SetClksPerBeat(seq_tracks->GetClksPerBeat());
    rec_start_time = 0;
    rec_end_time = TIME_INFINITE;
    en_tracks.resize(tracks->GetNumTracks());
    en_tracks.assign(tracks->GetNumTracks(), false);
}


bool MIDIRecorder::SetTrackInPort(unsigned int trk_num, unsigned int port) {
    if (!tracks->IsValidTrackNumber(trk_num) || !MIDIManager::IsValidInPortNumber(port))
        return false;
    proc_lock.lock();
    if (IsPlaying())
        tracks->GetTrack(trk_num)->CloseOpenEvents(seq->GetCurrentMIDIClockTime());
    tracks->GetTrack(trk_num)->SetInPort(port);
    proc_lock.unlock();
    return true;
}


bool MIDIRecorder::SetTrackRecChannel(unsigned int trk_num, char chan) {
    if (!tracks->IsValidTrackNumber(trk_num) || (chan < -1 || chan > 15))
        return false;
    proc_lock.lock();
    if (IsPlaying())
        tracks->GetTrack(trk_num)->CloseOpenEvents(seq->GetCurrentMIDIClockTime());
    tracks->GetTrack(trk_num)->SetRecChannel(chan);
    proc_lock.unlock();
    return true;
}


bool MIDIRecorder::InsertTrack(int trk_num) {
    if (!seq->GetMultiTrack()->IsValidTrackNumber(trk_num))
        return false;
    proc_lock.lock();
    if (tracks->InsertTrack(trk_num))
        en_tracks.insert(en_tracks.begin() + trk_num, false);
    proc_lock.unlock();
    return true;
}


bool MIDIRecorder::DeleteTrack(int trk_num) {
    if (!seq->GetMultiTrack()->IsValidTrackNumber(trk_num))
        return false;
    proc_lock.lock();
    if (tracks->DeleteTrack(trk_num))
        en_tracks.erase(en_tracks.begin() + trk_num);
    proc_lock.unlock();
    return true;
}


bool MIDIRecorder::MoveTrack(int from, int to) {
/*
    if (from == to) return true;                        // nothing to do
    proc_lock.lock();
    if (state.multitrack->MoveTrack(from, to)) {        // checks if from and to are valid
        MIDIProcessor* temp_processor = track_processors[from];
        //int temp_offset = time_shifts[from];
        //int temp_port = track_ports[from];
        track_processors.erase(track_processors.begin() + from);
        //time_shifts.erase(time_shifts.begin() + from);
        //track_ports.erase(track_ports.begin() + from);
        if (from < to)
            to--;
        track_processors.insert(track_processors.begin() + to, temp_processor);
        //time_shifts.insert(time_shifts.begin() + to, temp_offset);
        //track_ports.insert(track_ports.begin() + to, temp_port);
        MIDIClockTime now = state.cur_clock;            // remember current time
        state.Reset();                                  // reset the state (syncs the iterator)
        GoToTime(now);                                  // returns to current time
        if (state.notifier)                             // cause a complete GUI refresh
            state.notifier->Notify(MIDISequencerGUIEvent::GROUP_ALL);
        ret = true;
    }
    proc_lock.unlock();
    return ret;
*/
    return true;
}


bool MIDIRecorder::EnableTrack(unsigned int trk_num) {
    if (!seq->GetMultiTrack()->IsValidTrackNumber(trk_num))
        return false;
    if (tracks->IsValidTrackNumber(trk_num))
        tracks->GetTrack(trk_num)->Clear();
    else {
        while (tracks->GetNumTracks() <= trk_num) {
            tracks->InsertTrack();
            en_tracks.push_back(false);
        }
    }
    en_tracks[trk_num] = true;
    MIDITrack* dest = tracks->GetTrack(trk_num);
    MIDITrack* src = seq_tracks->GetTrack(trk_num);
    unsigned int in_port = src->GetInPort();
    dest->SetInPort(in_port);
    en_ports.insert(in_port);
    for (unsigned int i = 0; i < src->GetNumEvents(); i++)
        if (!src->GetEvent(i).IsNote())
            dest->InsertEvent(src->GetEvent(i));
    dest->SetRecChannel(dest->GetChannel());
    return true;
/*
    if (en_ports[port] != 0)
        return;
    if (multitrack->GetNumTracks() == 0)
        multitrack->InsertTrack();
    std::vector<MIDITrack*> *vp = new std::vector<MIDITrack*>(16);
    en_ports[port] = vp;
    for (unsigned int i = 0; i < 16; i++) {
        if (en_chans) {
            multitrack->InsertTrack();
            (*en_ports[port])[i] = multitrack->GetTrack(multitrack->GetNumTracks() - 1);
        }

        else
            (*en_ports[port])[i] = 0;
    }
*/
}


bool MIDIRecorder::DisableTrack(unsigned int trk_num) {
    if (!tracks->IsValidTrackNumber(trk_num))
        return false;
    en_tracks[trk_num] = false;
    en_ports.clear();
    int max_enabled = -1;
    for (unsigned int i = 0; i < en_tracks.size(); i++)
        if (en_tracks[i] == true) {
            max_enabled = i;
            en_ports.insert(tracks->GetTrack(i)->GetInPort());
        }
    if (max_enabled < (int)trk_num) {
        while ((int)tracks->GetNumTracks() > max_enabled + 1)
            tracks->DeleteTrack();
        en_tracks.resize(max_enabled + 1);
    }
    else
        tracks->GetTrack(trk_num)->Clear();
    return true;
}






// Inherited from MIDITICK

void MIDIRecorder::Start() {
    if (!IsPlaying()) {
        std::cout << "\t\tEntered in MIDIRecorder::Start() ..." << std::endl;
        MIDIManager::OpenInPorts();
        tracks->ClearTracks();
        SetSeqNotifier();
        dev_time_offset = 0;
        rec_on.store(true);
        seq->SetAutoStop(false);
        seq->Start();
        MIDITickComponent::Start();
        std::cout << "\t\t ... Exiting from MIDIRecorder::Start()" << std::endl;
    }
}


void MIDIRecorder::Stop() {
    if (IsPlaying()) {
        std::cout << "\t\tEntered in MIDIRecorder::Stop() ..." << std::endl;
        rec_on.store(false);
        MIDITickComponent::Stop();
        seq->Stop();
        seq->SetAutoStop(true);
        ResetSeqNotifier();
        MIDIManager::CloseInPorts();
        std::cout << "\t\t ... Exiting from MIDIRecorder::Stop()" << std::endl;
    }
}


void MIDIRecorder::SetSeqNotifier() {
    notifier.SetOtherNotifier(seq->GetState()->notifier);
    seq->GetState()->notifier = &notifier;
}


void MIDIRecorder::ResetSeqNotifier() {
    seq->GetState()->notifier = notifier.GetOtherNotifier();
}



void MIDIRecorder::StaticTickProc(tMsecs sys_time, void* pt) {
    MIDIRecorder* seq_pt = static_cast<MIDIRecorder *>(pt);
    seq_pt->TickProc(sys_time);
}


void MIDIRecorder::TickProc(tMsecs sys_time) {
    if (!rec_on.load())
        return;

    proc_lock.lock();
    MIDIClockTime cur_time = seq->GetCurrentMIDIClockTime();
    if (cur_time < rec_start_time || cur_time > rec_end_time) {
        proc_lock.unlock();
        return;
    }
    MIDIRawMessage rmsg;

    //tMsecs cur_time = sys_time - sys_time_offset + rec_time_offset;
    //float clocks_per_ms = (tempobpm * multitrack->GetClksPerBeat()) / 60000.0;

    for (unsigned int i = 0; i < MIDIManager::GetNumMIDIIns(); i++) {
        if (en_ports.count(i) == 0)
            continue;
        MIDIInDriver* port = MIDIManager::GetInDriver(i);
        port->LockQueue();
        for (unsigned int j = 0, out_count = 0; j < port->GetQueueSize() && out_count < 100; j++, out_count++) {
            port->ReadMessage(rmsg, j);
            //msg_time = (MIDIClockTime)((rmsg.timestamp - sys_time_offset + rec_time_offset) * clocks_per_ms);
            MIDITimedMessage msg(rmsg.msg);
            msg.SetTime(cur_time);
            if (msg.IsChannelMsg()) {
                char ch1 = msg.GetChannel();
                for (i = 0; i < tracks->GetNumTracks(); i++) {
                    if (!en_tracks[i]) continue;
                    char ch2 = tracks->GetTrack(i)->GetRecChannel();
                    if (ch1 == ch2 || ch2 == -1)
                        tracks->InsertEvent(i, msg);
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
    proc_lock.unlock();
}





