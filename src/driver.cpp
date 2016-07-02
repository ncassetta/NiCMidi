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


#include "../include/world.h"
#include "../include/driver.h"
#include "../include/timer.h"



MIDIOutDriver::MIDIOutDriver(int id) : processor(0), port_id(id), busy(0) {
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


void MIDIOutDriver::OpenPort() {
    if (!port->isPortOpen()) {
        try {
            port->openPort(port_id);
#if DRIVER_USES_MIDIMATRIX
            out_matrix.Clear();
#endif
            std::cout << "Port " << port->getPortName() << " open" << std::endl;
        }
        catch (RtMidiError& error) {
            error.printMessage();
        }
    }
}


void MIDIOutDriver::ClosePort() {
    if (port->isPortOpen()) {
        port->closePort();
        std::cout << "Port " << port->getPortName() << " closed" << std::endl;
    }
}


void MIDIOutDriver::AllNotesOff( int chan ) {
    MIDIMessage msg;

    busy++;
    //out_mutex.lock();

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

    //out_mutex.unlock();
    busy--;
}


void MIDIOutDriver::AllNotesOff() {
    busy++;
    //out_mutex.lock();

    for(int i = 0; i < 16; ++i)
        AllNotesOff(i);

    //out_mutex.unlock();
    busy--;
}


void MIDIOutDriver::OutputMessage(const MIDITimedMessage& msg) {    // MIDITimedMessage is good also for MIDIMessage
    MIDITimedMessage msg_copy(msg);

    if (processor)
        processor->Process(&msg_copy);

    int i = 0;
    for( ; i < DRIVER_MAX_RETRIES; i++) {
        if (!busy) {
            HardwareMsgOut(msg_copy);
            break;
        }
        std::cerr << "busy driver ... " << std::endl;
        MIDITimer::Wait(1);
    }
    if (i == 100)
        std::cerr << "MIDIOutDriver::OutputMessage() failed!" << std::endl;
}


void MIDIOutDriver::HardwareMsgOut(const MIDIMessage &msg) {
    if (!port->isPortOpen())
        return;
    //out_mutex.lock();
    busy++;
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
    }
    if (msg.IsSysEx()) // || msg.IsReset())
        MIDITimer::Wait(DRIVER_WAIT_AFTER_SYSEX);
    //out_mutex.unlock();
    busy--;
}





MIDIInDriver::MIDIInDriver(int id) : processor(0), port_id(id), busy(0) {
    try {
        port = new RtMidiIn();
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


void MIDIInDriver::OpenPort() {
    if (!port->isPortOpen()) {
        try {
            port->openPort(port_id);
            std::cout << "Port " << port->getPortName() << " open" << std::endl;
        }
        catch (RtMidiError& error) {
            error.printMessage();
        }
    }
}


void MIDIInDriver::ClosePort() {
    if (port->isPortOpen()) {
        port->closePort();
        std::cout << "Port " << port->getPortName() << " closed" << std::endl;
    }
}


/*
bool MIDIInDriver::HardwareMsgIn( MIDITimedBigMessage &msg )
  {
    // put input midi messages thru the in processor

    if( in_proc )
    {
      if( in_proc->Process( &msg )==false )
      {
        // message was deleted, so ignore it.
        return true;
      }
    }

    // stick input into in queue

    if( in_queue.CanPut() )
    {
      in_queue.Put( msg );
    }
    else
    {
      return false;
    }


    // now stick it through the THRU processor

    if( thru_proc )
    {
      if( thru_proc->Process( &msg )==false )
      {
        // message was deleted, so ignore it.
        return true;
      }
    }


    if( thru_enable )
    {
      // stick this message into the out queue so the tick procedure
      // will play it out asap. Put frees eventual old sysex pointers

      if( out_queue.CanPut() )
      {
        out_queue.Put( msg );
      }
      else
      {
        return false;
      }
    }

    return true;
  }
*/


bool MIDIInDriver::InputMessage(MIDITimedMessage &msg) {
    MIDIMessage m;
    if (!HardwareMsgIn(m))
        return false;
    msg = m;
    if (processor)
        return processor->Process(&msg);
    return true;
}


bool MIDIInDriver::HardwareMsgIn(MIDIMessage &msg) {
    if (!port->isPortOpen())
        return false;
    //out_mutex.lock();
    busy++;
    msg_bytes.clear();
    try {
        port->getMessage(&msg_bytes);    // try to get a message from the RtMidi in queue
    }                                    // (we don't use RtMidi timestamp)
    catch (RtMidiError& error) {
        error.printMessage();
        return false;
    }
    if (msg_bytes.size() == 0)          // no message in the queue
        return false;
    msg.Clear();
    msg.SetStatus(msg_bytes[0]);        // in msg_bytes[0] there is the status byte
    if (msg.IsSysEx()) {
        msg.AllocateSysEx(msg_bytes.size());
        for (unsigned int i = 0; i < msg_bytes.size(); i++)
            msg.GetSysEx()->PutSysByte(msg_bytes[i]);   // puts the 0xf0 also in the buffer
        return true;
    }
    else if (msg.GetStatus() == 0xff)   // this is a reset message, NOT a meta
        return true;
    else {
        if (msg.GetLength() > 1)
            msg.SetByte1(msg_bytes[1]);
        if (msg.GetLength() > 2)
            msg.SetByte2(msg_bytes[2]); // byte3 surely 0 in non-meta messages
    }
}
