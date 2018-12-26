#include "../include/manager.h"
#include "../include/thru.h"


MIDIThru::MIDIThru() : MIDITickComponent(PR_PRE_SEQ, StaticTickProc), in_port(0), out_port(0), in_channel(-1),
                                         out_channel(-1), processor(0)
{
    if (MIDIManager::GetNumMIDIIns() > 0)
        in_port = MIDIManager::GetInDriver(0);
    if (MIDIManager::GetNumMIDIOuts() > 0)
        out_port = MIDIManager::GetOutDriver(0);
}


void MIDIThru::Reset() {
    Stop();
    SilentOut();
    if (MIDIManager::GetNumMIDIIns() > 0)
        in_port = MIDIManager::GetInDriver(0);
    if (MIDIManager::GetNumMIDIOuts() > 0)
        out_port = MIDIManager::GetOutDriver(0);
    processor = 0;
    in_channel = -1;
    out_channel = -1;
}


void MIDIThru::SetInPort(MIDIInDriver* const port) {
    if (port == in_port)
        return;                                     // trying to assign same ports: nothing to do

    if (IsPlaying()) {
        proc_lock.lock();
        in_port->ClosePort();
        SilentOut();
        port->OpenPort();
        in_port = port;
        proc_lock.unlock();
    }
    else
        in_port = port;
}


void MIDIThru::SetOutPort(MIDIOutDriver* const port) {
    if (port == out_port)
        return;                                     // trying to assign same ports: nothing to do

    if (IsPlaying()) {
        proc_lock.lock();
        SilentOut();
        out_port->ClosePort();
        port->OpenPort();
        proc_lock.unlock();
    }
    else
        out_port = port;
}


void MIDIThru::SetProcessor(MIDIProcessor* proc) {
    if (IsPlaying()) {
        proc_lock.lock();
        SilentOut();
        processor = proc;
        proc_lock.unlock();
    }
    else
        processor = proc;
}


/* OLD WORKING FUNCTION
bool MIDIManager::SetThruPorts(unsigned int in_port, unsigned int out_port) {
    if (in_port >= MIDI_ins.size() || out_port >= MIDI_outs.size())
        return false;

    std::cout << "Step 1" << std::endl;
    if (thru_input != -1) {                         // an old MIDI thru input was set
        MIDI_ins[thru_input]->SetThruEnable(false); // turn off the MIDI thru
        MIDI_ins[thru_input]->ClosePort();          // and close the port
    }

    std::cout << "Step 2" << std::endl;

    if (thru_output != -1) {                        // an old MIDI thru output was set
        MIDI_outs[thru_output]->AllNotesOff();      // cut off sounding notes
// TODO: insert a wait?
        MIDI_outs[thru_output]->ClosePort();        // and close it
    }

    MIDI_ins[in_port]->SetThruEnable(false, MIDI_outs[out_port]);
                                                    // set the new MIDI thru
    MIDI_ins[in_port]->OpenPort();
    MIDI_outs[out_port]->OpenPort();
    MIDI_ins[in_port]->SetThruEnable(thru_enable);
    thru_input = in_port;
    thru_output = out_port;
    return true;
}
*/

void MIDIThru::SetInChannel(char chan) {
    if (IsPlaying()) {
        proc_lock.lock();
        SilentOut();
        in_channel = chan;
        proc_lock.unlock();
    }
    else
        in_channel = chan;
}


void MIDIThru::SetOutChannel(char chan) {
    if (IsPlaying()) {
        proc_lock.lock();
        SilentOut();
        out_channel = chan;
        proc_lock.unlock();
    }
    else
        out_channel = chan;
}


void MIDIThru::Start() {
    if (in_port == 0 || out_port == 0 ||            // we have not set the thru ports yet
        IsPlaying())
        return;
    in_port->OpenPort();
    out_port->OpenPort();
    MIDITickComponent::Start();
}


void MIDIThru::Stop() {
    if (in_port == 0 || out_port == 0 ||            // we have not set the thru ports yet
        !IsPlaying())
        return;
    MIDITickComponent::Stop();
    in_port->ClosePort();
    SilentOut();
    out_port->ClosePort();
}


void MIDIThru::SilentOut() {
    if (out_port) {
        if (out_channel != -1)
            out_port->AllNotesOff(out_channel);
        else
            out_port->AllNotesOff();
    }
}




void MIDIThru::StaticTickProc(tMsecs sys_time, void* pt) {
    MIDIThru* thru_pt = static_cast<MIDIThru *>(pt);
    thru_pt->TickProc(sys_time);
}


// NEW FUNCTION WITH DIRECT SEND WITH HardwareMsgOut
void MIDIThru::TickProc(tMsecs sys_time_)
{
    proc_lock.lock();
    static unsigned int times = 0;

    if (times % 1000 == 0)
        std::cout << "MIDIThru::TickProc() called " << times * 1000 << " times\n";
    times++;


    MIDIRawMessage rmsg;
    MIDITimedMessage msg;
    in_port->LockQueue();
    for (unsigned int i = 0; i < in_port->GetQueueSize(); i++) {
        std::cout << "Message found\n";
        in_port->ReadMessage(rmsg, i);
        msg = rmsg.msg;
        if (msg.IsChannelMsg()) {
            if (in_channel == msg.GetChannel() || in_channel == -1) {
                if (out_channel != -1)
                    msg.SetChannel(out_channel);
                if (processor)
                    processor->Process(&msg);
                out_port->OutputMessage(msg);
            }
        }
    }
    in_port->UnlockQueue();
    proc_lock.unlock();
}
