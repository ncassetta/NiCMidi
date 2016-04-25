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



#ifndef _JDKMIDI_DRIVER_H
#define _JDKMIDI_DRIVER_H

#include "msg.h"
#include "matrix.h"
#include "process.h"
#include "queue.h"


#include "../rtmidi-2.1.1/RtMidi.h"

#include <vector>
#include <string>
#include <mutex>
#include <atomic>


class MIDIOutDriver {
public:

                            MIDIOutDriver (int id);
    virtual                 ~MIDIOutDriver();

        // Clears the queue and the matrix
    virtual void            Reset();

    std::string             GetPortName()                           { return port->getPortName(port_id); }

        // Send all notes off message
    void                    AllNotesOff(int chan);
    void                    AllNotesOff();

    void                    OutputMessage(const MIDITimedBigMessage& msg);


        // Open the MIDI out port _id_
    virtual void            OpenPort();

          // Close the open MIDI out port
    virtual void            ClosePort();

    bool                    IsPortOpen() const      { return port->isPortOpen();}




/*
        // The time tick procedure inherited from MIDITick:
        // 	    manages in/out/thru to hardware
        //
        // if you need to poll midi in hardware,
        // you can override this method - Call MIDIDriver::TimeTick(t)
        // first, You may then poll the midi in
        // hardware, parse the bytes, form a message, and give the
        // resulting message to HandleMsgIn to process it and put it in
        // the in_queue.
    virtual void            TimeTick( unsigned long sys_time );
*/      // TODO: revise


protected:


         // Processes the message with the OutProcessor and then  ends it to the hardware MIDI port
    virtual void            HardwareMsgOut( const MIDITimedBigMessage &msg );

        // additional TimeTick procedure
/*
    MIDITick*               tick_proc;
*/

        // the hardware port
    RtMidiOut*                      port;
    const int                       port_id;

        //std::recursive_mutex    out_mutex;

    std::atomic<unsigned char> busy;        // TODO: use the mutex???

        // to keep track of notes on going to MIDI out
    MIDIMatrix out_matrix;

        // this vector is used by HardwareMsgOut to feed the port
    std::vector<unsigned char>      msg_bytes;
};



/* FOR NOW COMMENTED: GIVE ERRORS

class MIDIInDriver {
public:

                            MIDIInDriver(int queue_size );
    virtual                 ~MIDIInDriver();

    virtual void            Reset();


        // To set and get the MIDI thru
    void                    SetThruEnable( bool f )                 { thru_enable = f; }
    bool                    GetThruEnable() const                   { return thru_enable; }

        // To set the midi processors used for thru, out, and in
    void                    SetThruProcessor( MIDIProcessor *proc ) { thru_proc = proc; }
    void                    SetInProcessor( MIDIProcessor *proc )   { in_proc = proc; }

        // Call handle midi in when a parsed midi message
        // comes in to the system. Can be called by a callback function
        // or by your TimeTick() function.
    virtual bool            HardwareMsgIn( MIDITimedBigMessage &msg );



        // Opens the MIDI in port _id_
    virtual bool            OpenPort();

           // Closes the open MIDI in port
    virtual void            ClosePort();


/*
        // Starts the hardware timer for playing MIDI. Default time resolution is 1 ms
    virtual bool            StartTimer ( int resolution_ms = DEFAULT_TIMER_RESOLUTION ) = 0;

        // Stops the hardware timer
    virtual void            StopTimer() = 0;



        // The time tick procedure inherited from MIDITick:
        // 	    manages in/out/thru to hardware
        //
        // if you need to poll midi in hardware,
        // you can override this method - Call MIDIDriver::TimeTick(t)
        // first, You may then poll the midi in
        // hardware, parse the bytes, form a message, and give the
        // resulting message to HandleMsgIn to process it and put it in
        // the in_queue.
    virtual void            TimeTick( unsigned long sys_time );




protected:

        // the in and out queues
    MIDIQueue               in_queue;
    MIDIQueue               out_queue;




        // the processors
    MIDIProcessor*          in_proc;
    MIDIProcessor*          thru_proc;

    bool                    thru_enable;

// to keep track of notes on going to MIDI out

      MIDIMatrix out_matrix;

      RtMidiIn              device;

};
*/

#endif // _JDKMIDI_DRIVER_H
