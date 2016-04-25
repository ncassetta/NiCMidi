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


MIDIOutDriver::MIDIOutDriver(int id) : port_id(id), busy(0) {
    port = new RtMidiOut;
}


MIDIOutDriver::~MIDIOutDriver() {
    port->closePort();
    delete port;
}


void MIDIOutDriver::Reset() {
    port->closePort();
    out_matrix.Clear();
}


void MIDIOutDriver::AllNotesOff( int chan ) {   // TODO: can we avoid to send a NOTE OFF for every note?
    MIDITimedBigMessage msg;                    // we could eliminate the MIDIMatrix

    // send a note off for every note on in the out_matrix

    //out_mutex.lock();
    busy++;
    if(out_matrix.GetChannelCount(chan) > 0) {
        for(int note = 0; note < 128; ++note) {
            while(out_matrix.GetNoteCount(chan,note) > 0) {
                    // make a note off with note on msg, velocity 0
                msg.SetNoteOn( (unsigned char)chan, (unsigned char)note, 0 );
                HardwareMsgOut(msg);
            }
        }
    }

    msg.SetControlChange(chan,C_DAMPER,0 );
    HardwareMsgOut(msg);

    msg.SetAllNotesOff( (unsigned char)chan );
    HardwareMsgOut(msg);
    //out_mutex.unlock();
    busy--;
}


void MIDIOutDriver::AllNotesOff() {
    //out_mutex.lock();
    busy++;
    for(int i = 0; i < 16; ++i)
        AllNotesOff(i);
    //out_mutex.unlock();
    busy--;
}



// TODO: this could be unneeded. We only could use HardwareMsgOut with mutex

void MIDIOutDriver::OutputMessage(const MIDITimedBigMessage& msg) {
    int i = 0;
    for( ; i < 100; i++) {
        if (!busy) {
            HardwareMsgOut(msg);
            break;
        }
        std::cout << "driver busy ... " << std::endl;
        MIDITimer::Wait(1);
    }
    if (i == 100)
        std::cout << "MIDIOutDriver::OutputMessage() failed!" << std::endl;
}


void MIDIOutDriver::OpenPort() {
    if (!port->isPortOpen()) {
        port->openPort(port_id);
        std::cout << "Port " << port->getPortName() << " opened" << std::endl;
    }
}


void MIDIOutDriver::ClosePort() {
    if (port->isPortOpen()) {
        port->closePort();
        std::cout << "Port " << port->getPortName() << "closed" << std::endl;
    }
}


/* ALL IS MOVED TO MIDIManager: driver no more inherits from MIDITick
  void MIDIDriver::TimeTick( unsigned long sys_time )
  {
    // run the additional tick procedure if we need to
    if( tick_proc )
    {
      tick_proc->TimeTick( sys_time );
    }

    // feed as many midi messages from out_queue to the hardware out port
    // as we can

    while( out_queue.CanGet() )
    {
      // use the Peek() function to avoid allocating memory for
      // a duplicate sysex

      if( HardwareMsgOut( *(out_queue.Peek() ) )==true )
      {
        // ok, got and sent a message - update our out_queue now
        // added by me: we always must delete eventual sysex pointers before reassigning to ev
        out_queue.Next();
      }
      else
      {
        // cant send any more, stop now.
        break;
      }

    }

  }
  */


void MIDIOutDriver::HardwareMsgOut(const MIDITimedBigMessage &msg) {
    if (!port->isPortOpen())
        return;
    //out_mutex.lock();
    busy++;
    msg_bytes.clear();
    if (msg.IsChannelMsg()) {
        out_matrix.Process (msg);
        msg_bytes.push_back(msg.GetStatus());
        msg_bytes.push_back(msg.GetByte1());
        msg_bytes.push_back(msg.GetByte2());
        //std::cout << "Driver sent channel message ... ";
    }

    else if (msg.IsSysEx()) {
        for (int i = 0; i < msg.GetSysEx()->GetLength(); i++)
            msg_bytes.push_back(msg.GetSysEx()->GetData(i));
        std::cout << "Driver sent sysex of " << msg.GetSysEx()->GetLength() << " bytes ... ";
        MIDITimer::Wait( 50 );
    }
    else {
        char s[100];
        msg.MsgToText(s);
        std::cout << "Driver skipped message ... " << s;
    }
    if (msg_bytes.size() > 0)
        port->sendMessage(&msg_bytes);
    //out_mutex.unlock();
    //std::cout << "done!" << std::endl;
    busy--;
}



/* jdksmidi window only version
bool MIDIDriverWin32::HardwareMsgOut ( const MIDITimedBigMessage &msg )
{
    if ( out_open )
    {
        // msg is a channel message
        if ( msg.IsChannelMsg() )
        {
            DWORD winmsg;
            winmsg =
                ( ( ( DWORD ) msg.GetStatus() & 0xFF )       )
                | ( ( ( DWORD ) msg.GetByte1()  & 0xFF ) <<  8 )
                | ( ( ( DWORD ) msg.GetByte2()  & 0xFF ) << 16 );

            if ( midiOutShortMsg ( out_handle, winmsg ) != MMSYSERR_NOERROR )
            {
                char s[100];
                std::cout << "Driver FAILED to send short message " << msg.MsgToText(s) << std::endl;
                return false;
            }
        }

        else if ( msg.IsSysEx() )
        {
            MIDIHDR hdr;
// TODO: the buffer of the MIDISystemExclusive class holds only sysex bytes, without the 0xF0 status, so we
// need sysex_buffer and put 0xF0 as 1st charachter. If the status byte would be held
// in the MIDISystemExclusive this function would be simpler. This is possible, but perhaps there are
// compatibility problems with older software using GetBuf(). WHAT TO DO?


            if ( msg.GetSysEx()->GetLength() + 1 > sysex_buffer_size )
            {   // reallocate sysex_buffer
                delete[] sysex_buffer;
                sysex_buffer_size = msg.GetSysEx()->GetLength() + 1;
                sysex_buffer = new CHAR[ sysex_buffer_size ];
            }

            sysex_buffer[0] = msg.GetStatus();
            memcpy(sysex_buffer + 1, msg.GetSysEx()->GetBuf(), msg.GetSysEx()->GetLength());
            hdr.lpData = sysex_buffer;
            hdr.dwBufferLength = msg.GetSysEx()->GetLength() + 1;
            hdr.dwFlags = 0;

            if ( midiOutPrepareHeader( out_handle, &hdr, sizeof ( MIDIHDR ) ) != MMSYSERR_NOERROR ) {
                char s[100];
                std::cout << "Driver FAILED to send SysEx on PrepareHeader " << msg.MsgToText(s) << std::endl;
                return false;
            }

            if ( midiOutLongMsg( out_handle, &hdr, sizeof ( MIDIHDR ) ) != MMSYSERR_NOERROR )
            {
                char s[100];
                std::cout << "Driver FAILED to send SysEx on OurLongMsg " << msg.MsgToText(s) << std::endl;
                return false;
            }
            while ( midiOutUnprepareHeader( out_handle, &hdr, sizeof( MIDIHDR ) ) == MIDIERR_STILLPLAYING )
            {
                // Should put a delay in here rather than a busy-wait
            }
            char s[100];
            std::cout << "Driver sent Sysex msg " << msg.MsgToText(s) << std::endl;
        }

        else
        {
            char s[100];
            std::cout << "Driver skipped message " << msg.MsgToText(s) << std::endl;
        }

        return true;
    }

    // std::cout << "Driver not open!" << std::endl;
    return false;
}
*/

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
