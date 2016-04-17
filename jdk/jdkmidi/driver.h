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

// Updated to reflect changes in jdksmidi


#ifndef _JDKMIDI_DRIVER_H
#define _JDKMIDI_DRIVER_H

#include "msg.h"
#include "matrix.h"
#include "process.h"
#include "queue.h"


#include "../../rtmidi-2.1.1/RtMidi.h"

#include<vector>


class MIDIOutDriver {
public:

                            MIDIOutDriver(int id, int queue_size = -1 );
    virtual                 ~MIDIOutDriver();

    virtual void            Reset();


        // Returns true if the output queue is not full
    bool                    CanOutputMessage() const                { return out_queue.CanPut(); }

        // To set and get the MIDI thru
    void                    SetThruEnable( bool f )                 { thru_enable = f; }
    bool                    GetThruEnable() const                   { return thru_enable; }

        // To get the message queue

    MIDIQueue*              GetQueue()                              { return &out_queue; }

        // To set the midi processors used for thru, out, and in
    void                    SetThruProcessor( MIDIProcessor *proc ) { thru_proc = proc; }
    void                    SetOutProcessor( MIDIProcessor *proc )  { out_proc = proc; }

         // Processes the message with the OutProcessor and then puts it in the out_queue
    void                    OutputMessage( MIDITimedBigMessage &msg );

        // Send all notes off message
    void                    AllNotesOff(int chan);
    void                    AllNotesOff();


        // Opens the MIDI out port _id_

    virtual bool            OpenPort()              { port->openPort(port_id); return true; }
          // Closes the open MIDI out port

    virtual void            ClosePort()             { port->closePort(); }


/*
        // Starts the hardware timer for playing MIDI. Default time resolution is 1 ms
    virtual bool            StartTimer ( int resolution_ms = DEFAULT_TIMER_RESOLUTION ) = 0;

        // Stops the hardware timer
    virtual void            StopTimer() = 0;


*/
        // Sends the message to the hardware open MIDI port
    bool                    HardwareMsgOut( const MIDITimedBigMessage &msg );


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



/*
        // Gets the nunber of MIDI in ports present on the computer.
    static unsigned int     GetNumMIDIInDevs()                      { return num_in_devs; }

        // Gets the number of MIDI out ports present on the computer.
    static unsigned int     GetNumMIDIOutDevs()                     { return num_out_devs; }

        // Gets the name of the MIDI in port _i_.
	static const char*      GetMIDIInDevName(unsigned int i)        { if ( i < num_in_devs ) return in_dev_names[i];
                                                                      else return ""; }

        // Gets the name of the MIDI out port _i_.
    static const char*      GetMIDIOutDevName(unsigned int i)       { if ( i < num_out_devs ) return out_dev_names[i];
                                                                    else return ""; }
*/


protected:

        // the in and out queues
    MIDIQueue               out_queue;

    static const int        DEFAULT_QUEUE_SIZE = 128;




        // the processors
    MIDIProcessor*          out_proc;
    MIDIProcessor*          thru_proc;

    bool                    thru_enable;

        // additional TimeTick procedure
/*
    MIDITick*               tick_proc;
*/


// to keep track of notes on going to MIDI out

    MIDIMatrix out_matrix;

    RtMidiOut*                      port;
    const int                       port_id;
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
