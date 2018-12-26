#include "../include/recorder.h"


MIDIRecorder::MIDIRecorder(MIDIMultiTrack* mlt) :
    MIDITickComponent(PR_PRE_SEQ, StaticTickProc),
    tempobpm(120.0), start_time(0), rec_on(false),
    OWNS_MULTI(mlt == 0)
{
    if (mlt == 0)
        multitrack = new MIDIMultiTrack(17);
    else
        multitrack = mlt;
    for(unsigned int i = 0; i < MIDIManager::GetNumMIDIIns(); i++)
        en_ports.push_back(0);
}


void MIDIRecorder::SetTempo(float t) {
    if (IsPlaying())
        return;
    MIDITimedMessage msg;
    msg.SetTempo(t);
    if (multitrack->GetTrack(0)->InsertEvent(msg))
        tempobpm = t;
}


void MIDIRecorder::EnablePort(unsigned int port) {
    std::vector<MIDITrack*> *vp = new std::vector<MIDITrack*>(16);
    if (en_ports[port] != 0)
        delete en_ports[port];
    en_ports[port] = vp;
}


void MIDIRecorder::EnableChannel(unsigned int port, unsigned int ch, unsigned int trk) {
   (*en_ports[port])[ch] = multitrack->GetTrack(trk);
}

void MIDIRecorder::EnableAllChannels(unsigned int port) {

}


void MIDIRecorder::DisablePort(unsigned int port) {
    if (en_ports[port] != 0)
        delete en_ports[port];
    en_ports[port] = 0;
}

void MIDIRecorder::DisableChannel(unsigned int port, unsigned int ch) {
    (*en_ports[port])[ch] = 0;
}




// Inherited from MIDITICK

void MIDIRecorder::Start() {
    if (!IsPlaying()) {
        std::cout << "\t\tEntered in MIDIRecorder::Start() ..." << std::endl;
        MIDIManager::OpenInPorts();
        rec_time_offset = 0;
        sys_time_offset = MIDITimer::GetSysTimeMs();
        MIDITickComponent::Start();
        std::cout << "\t\t ... Exiting from MIDIRecorder::Start()" << std::endl;
    }
}


void MIDIRecorder::Stop() {
    if (IsPlaying()) {
        std::cout << "\t\tEntered in MIDIRecorder::Stop() ..." << std::endl;
        MIDITickComponent::Stop();
        MIDIManager::CloseInPorts();
        std::cout << "\t\t ... Exiting from MIDIRecorder::Stop()" << std::endl;
    }
}






void MIDIRecorder::StaticTickProc(tMsecs sys_time, void* pt) {
    MIDIRecorder* seq_pt = static_cast<MIDIRecorder *>(pt);
    seq_pt->TickProc(sys_time);
}


void MIDIRecorder::TickProc(tMsecs sys_time) {
    if (!rec_on.load())
        return;

    //double sys_time = (double)sys_time_ - (double)sys_time_offset;+
    MIDIClockTime msg_time;
    MIDIRawMessage rmsg;

    tMsecs cur_time = sys_time - sys_time_offset + rec_time_offset;
    float clocks_per_ms = (tempobpm * multitrack->GetClksPerBeat()) / 60000.0;

    for (unsigned int i = 0; i < MIDIManager::GetNumMIDIIns(); i++) {
        MIDIInDriver* port = MIDIManager::GetInDriver(i);
        if (en_ports[i] == 0)
            continue;
        port->LockQueue();
        for (unsigned int j = 0, out_count = 0; j < port->GetQueueSize() && out_count < 100; j++, out_count++) {
            port->ReadMessage(rmsg, j);
            msg_time = (MIDIClockTime)((rmsg.timestamp -sys_time_offset + rec_time_offset) / clocks_per_ms) + start_time;
            MIDITimedMessage msg(rmsg.msg);
            msg.SetTime(msg_time);
            if (msg.IsChannelMsg()) {
                unsigned int ch = msg.GetChannel();
                if ((*en_ports[i])[ch] != 0)
                    (*en_ports[i])[ch]->PushEvent(msg);
            }
        }
        port->UnlockQueue();
    }
}




