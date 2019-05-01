/*
 *  libjdkmidi-2004 C++ Class Library for MIDI
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


#include "../include/driver.h"
#include "../include/timer.h"


/////////////////////////////////////////////////
//         class MIDIRawMessageQueue           //
/////////////////////////////////////////////////


void MIDIRawMessageQueue::Reset() {
    next_in = next_out = 0;
    for (unsigned int i = 0; i < buffer.size(); i++)
        buffer[i] = MIDIRawMessage();
}


void MIDIRawMessageQueue::PutMessage(const MIDIRawMessage& msg) {
    buffer[next_in] = msg;
    next_in = (next_in + 1) % buffer.size();
    if (next_in == next_out)
        next_out = (next_out + 1) % buffer.size();          // we lose actual out message
}


MIDIRawMessage& MIDIRawMessageQueue::GetMessage() {
    static MIDIRawMessage msg;      // needed if we want to return a reference
    if (next_in == next_out)
        return msg;
    else {
        unsigned int old_out = next_out;
        next_out = (next_out + 1) % buffer.size();
        return buffer[old_out];
    }
}


MIDIRawMessage& MIDIRawMessageQueue::ReadMessage(unsigned int n) {
    static MIDIRawMessage msg;      // needed if we want to return a reference
    if (n >= GetLength())
        return msg;
    else
        return buffer[(next_out + n) % buffer.size()];
}


/////////////////////////////////////////////////
//           class MIDIOutDriver               //
/////////////////////////////////////////////////


MIDIOutDriver::MIDIOutDriver(int id) :
    processor(0), port_id(id), num_open(0) {
    try {
        port = new RtMidiOut();
    }
    catch (RtMidiError& error) {
        error.printMessage();
        port = new RtMidiOut(RtMidi::RTMIDI_DUMMY);// A non functional MIDI out, which won't throw further exceptions
    }
}


MIDIOutDriver::~MIDIOutDriver() {
    port->closePort();
    delete port;
}


void MIDIOutDriver::Reset() {
    port->closePort();
    processor = 0;
    num_open = 0;
}


void MIDIOutDriver::OpenPort() {
    if (num_open == 0) {
        try {
            port->openPort(port_id);
#if DRIVER_USES_MIDIMATRIX
            out_matrix.Clear();
#endif
        }
        catch (RtMidiError& error) {
            error.printMessage();
            return;
        }
    }
    num_open++;

    std::cout << "Port " << port->getPortName(port_id) << " open";
    if (num_open > 1)
        std::cout << " (" << num_open << " times)";
    std::cout<< std::endl;
}


void MIDIOutDriver::ClosePort() {
    if (num_open == 1)
        port->closePort();
    if (num_open > 0) {
        num_open--;
        std::cout << "Port " << port->getPortName(port_id) << " closed";
        if (num_open > 0)
            std::cout << " (open " << num_open << " times)";
        std::cout << std::endl;
    }
    else
        std::cout << "Attempt to close an already closed port!" << std::endl;
}

/*
void MIDIOutDriver::SetThruChannel(char chan) {
    if (chan >= -1 && chan <= 15) {
        if (thru_channel != -1)
            AllNotesOff(thru_channel);
        else
            AllNotesOff();
        if (chan == -1)
            rechannelizer.Reset();
        else
            rechannelizer.SetAllRechan(chan);
        thru_channel = chan;
    }
}
*/

void MIDIOutDriver::AllNotesOff(int chan) {
    MIDIMessage msg;

    if (!port->isPortOpen())
        return;

    if (chan == -1) {
        for (int i = 0; i < 16; i++)
            AllNotesOff(i);
        return;
    }
    out_mutex.lock();

#if DRIVER_USES_MIDIMATRIX                      // send a note off for every note on in the out_matrix
    if(out_matrix.GetChannelCount(chan) > 0) {
        for(int note = 0; note < 128; ++note) {
            while(out_matrix.GetNoteCount(chan,note) > 0) {
                msg.SetNoteOff((unsigned char)chan, (unsigned char)note, 0);
                HardwareMsgOut(msg);
            }
        }
    }
    msg.SetControlChange(chan,C_DAMPER,0 );     // send a pedal off for every channel
    HardwareMsgOut(msg);
#endif // DRIVER_USES_MIDIMATRIX

    msg.SetAllNotesOff( (unsigned char)chan );
    HardwareMsgOut(msg);

    out_mutex.unlock();
}

/*
void MIDIOutDriver::AllNotesOff() {
    out_mutex.lock();

    for(int i = 0; i < 16; ++i)
        AllNotesOff(i);

    out_mutex.unlock();
}
*/


void MIDIOutDriver::OutputMessage(const MIDITimedMessage& msg) {    // MIDITimedMessage is good also for MIDIMessage
    MIDITimedMessage msg_copy(msg);

    if (processor)
        processor->Process(&msg_copy);

    int i = 0;
    for( ; i < DRIVER_MAX_RETRIES; i++) {
        if (out_mutex.try_lock()) {
            HardwareMsgOut(msg_copy);
            out_mutex.unlock();
            break;
        }
        std::cerr << "busy driver (" << (i + 1) << ") ... " << std::endl;
        MIDITimer::Wait(1);
    }
    if (i == DRIVER_MAX_RETRIES)
        std::cerr << "MIDIOutDriver::OutputMessage() failed!" << std::endl;
}


void MIDIOutDriver::HardwareMsgOut(const MIDIMessage &msg) {
    if (!port->isPortOpen())
        return;
    msg_bytes.clear();
#if DRIVER_USES_MIDIMATRIX
    if (msg.IsChannelMsg())
        out_matrix.Process (msg);
#endif

    if (msg.IsSysEx()) {
        for (int i = 0; i < msg.GetSysEx()->GetLength(); i++)
            msg_bytes.push_back(msg.GetSysEx()->GetData(i));
        //std::cout << "Driver sent sysex of " << msg.GetSysEx()->GetLength() << " bytes ... ";
    }

    //else if (msg.IsReset())         // a reset message, with the same status of meta events
    //    msg_bytes.push_back(msg.GetStatus()) TODO: for now don't send reset messages

    else if (msg.IsMetaEvent())
        return;                     // don't send meta events

    else {                          // other messages
        msg_bytes.push_back(msg.GetStatus());
        if (msg.GetLength() > 1)
            msg_bytes.push_back(msg.GetByte1());
        if (msg.GetLength() > 2)
            msg_bytes.push_back(msg.GetByte2());
    }

    if (msg_bytes.size() > 0) {
        try {
            port->sendMessage(&msg_bytes);
        }
        catch (RtMidiError& error) {
            error.printMessage();
        }
        //std::cout << "Driver sent nonSysex message" << std::endl;
    }
    if (msg.IsSysEx()) // || msg.IsReset())
        MIDITimer::Wait(DRIVER_WAIT_AFTER_SYSEX);
}


/////////////////////////////////////////////////
//           class MIDIInDriver                //
/////////////////////////////////////////////////


MIDIInDriver::MIDIInDriver(int id, unsigned int queue_size) :
    processor(0), port_id(id), num_open(0), in_queue(queue_size) {
    try {
        port = new RtMidiIn();
        port->setCallback(HardwareMsgIn, this);
        port->ignoreTypes(false, true, true);
    }
    catch (RtMidiError& error) {
        error.printMessage();
        port = new RtMidiIn(RtMidi::RTMIDI_DUMMY);// A non functional MIDI out, which won't throw further exceptions
    }
}


MIDIInDriver::~MIDIInDriver() {
    port->closePort();
    delete port;
}


void MIDIInDriver::Reset() {
    port->closePort();
    num_open = 0;
    in_queue.Reset();

    processor = 0;
}


void MIDIInDriver::OpenPort() {
    if (num_open == 0) {
        try {
            port->openPort(port_id);
        }
        catch (RtMidiError& error) {
            error.printMessage();
            return;
        }
    }
    num_open++;

    std::cout << "Port " << port->getPortName() << " open";
    if (num_open > 1)
        std::cout << " (" << num_open << " times)";
    std::cout<< std::endl;
}


void MIDIInDriver::ClosePort() {
    if (num_open == 1)
            port->closePort();
    if (num_open > 0) {
        num_open--;

        std::cout << "Port " << port->getPortName() << " closed";
        if (num_open > 0)
            std::cout << " (" << num_open << " times)";
        std::cout << std::endl;
    }
    else
        std::cout << "Attempt to close an already closed port!" << std::endl;
}


void MIDIInDriver::FlushQueue() {
    in_mutex.lock();
    in_queue.Flush();
    in_mutex.unlock();
}


void MIDIInDriver::SetProcessor(MIDIProcessor* proc) {
    in_mutex.lock();
    processor = proc;
    in_mutex.unlock();
}


bool MIDIInDriver::InputMessage(MIDIRawMessage &msg) {
    if (!in_queue.IsEmpty()) {
        msg = in_queue.GetMessage();
        return true;
    }
    return false;
}


bool MIDIInDriver::ReadMessage(MIDIRawMessage& msg, unsigned int n) {
    if (in_queue.GetLength() > 0) {
        msg = in_queue.ReadMessage(n);
        return true;
    }
    return false;
}

void MIDIInDriver::HardwareMsgIn(double time,
                                 std::vector<unsigned char>* msg_bytes,
                                 void* p) {

    MIDIInDriver* drv = static_cast<MIDIInDriver*>(p);

    std::cout << drv->GetPortName() << " callback executed\n";

    if (!drv->port->isPortOpen() || msg_bytes->size() == 0)
        return;

    drv->in_mutex.lock();
    MIDITimedMessage msg;
    msg.SetStatus(msg_bytes->operator[](0));        // in msg_bytes[0] there is the status byte
    if (msg.IsSysEx()) {
        msg.AllocateSysEx(msg_bytes->size());
        for (unsigned int i = 0; i < msg_bytes->size(); i++)
            msg.GetSysEx()->PutSysByte(msg_bytes->operator[](i));   // puts the 0xf0 also in the sysex buffer
    }
    else if (msg.GetStatus() == 0xff) { // this is a reset message, NOT a meta
    }
    else {
        if (msg.GetLength() > 1)
            msg.SetByte1(msg_bytes->operator[](1));
        if (msg.GetLength() > 2)
            msg.SetByte2(msg_bytes->operator[](2)); // byte3 surely 0 in non-meta messages
    }

    if (!msg.IsNoOp()) {                            // now we have a valid message

        if (drv->processor)
            drv->processor->Process(&msg);          // process it with the in processor
                                                    // adds the message to the queue
        drv->in_queue.PutMessage(MIDIRawMessage(msg,
                                                MIDITimer::GetSysTimeMs(),
                                                drv->port_id));
        std::cout << "Callback executed! Queue size: " << drv->in_queue.GetLength() << std::endl;
    }
    drv->in_mutex.unlock();
}
