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
#include "tick.h"


class MIDIDriver : public MIDITick {
public:

                            MIDIDriver(int queue_size );
    virtual                     ~MIDIDriver();

    virtual void            Reset();

        // To get the midi in queue
    MIDIQueue*              InputQueue()                            { return &in_queue; }
    const MIDIQueue*        InputQueue() const                      { return &in_queue; }


        // To get the midi out queue
    MIDIQueue*              OutputQueue()                           { return &out_queue; }
    const MIDIQueue*        OutputQueue() const                     { return &out_queue; }

        // Returns true if the output queue is not full
    bool                    CanOutputMessage() const                { return out_queue.CanPut(); }

        // To set and get the MIDI thru
    void                    SetThruEnable( bool f )                 { thru_enable = f; }
    bool                    GetThruEnable() const                   { return thru_enable; }

        // To set the midi processors used for thru, out, and in
    void                    SetThruProcessor( MIDIProcessor *proc ) { thru_proc = proc; }
    void                    SetOutProcessor( MIDIProcessor *proc )  { out_proc = proc; }
    void                    SetInProcessor( MIDIProcessor *proc )   { in_proc = proc; }

        // Sets the additional tick procedure
    void                    SetTickProc( MIDITick *tick )           { tick_proc = tick; }

        // Processes the message with the OutProcessor and then puts it in the out_queue
    void                    OutputMessage( MIDITimedBigMessage &msg );

        // Send all notes off message
    void                    AllNotesOff(int chan);
    void                    AllNotesOff();

        // Call handle midi in when a parsed midi message
        // comes in to the system. Can be called by a callback function
        // or by your TimeTick() function.
    virtual bool            HardwareMsgIn( MIDITimedBigMessage &msg );


    /* NEW BY NC:
	 * NOTE: In order to develop MIDI driver classes for other OS than Windows I started to
	 * integrate older MIDIDriverWin32 methods into the base class, giving them as pure virtual. So
	 * now every subclass of a MIDIDriver must implement these.
	 */

        // Opens the MIDI in port _id_
    virtual bool            OpenMIDIInPort ( int id ) = 0;

        // Opens the MIDI out port _id_
    virtual bool            OpenMIDIOutPort ( int id ) = 0;

        // Closes the open MIDI in port
    virtual void            CloseMIDIInPort() = 0;

        // Closes the open MIDI out port
    virtual void            CloseMIDIOutPort() = 0;

        // Resets open MIDI out port
    virtual void            ResetMIDIOut() = 0;

        // Starts the hardware timer for playing MIDI. Default time resolution is 1 ms
    virtual bool            StartTimer ( int resolution_ms = DEFAULT_TIMER_RESOLUTION ) = 0;

        // Stops the hardware timer
    virtual void            StopTimer() = 0;

        // Sends the message to the hardware open MIDI port
    virtual bool            HardwareMsgOut( const MIDITimedBigMessage &msg ) = 0;

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


    /* Moreover, now the driver keeps track statically of the MIDI devices installed on the computer
     * so, by these method, you can get them
     */

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

        // The default timer resolution is 1 msec (public: used by AdvancedSequencer)
    static const int       DEFAULT_TIMER_RESOLUTION = 1;

protected:

        // the in and out queues
    MIDIQueue               in_queue;
    MIDIQueue               out_queue;

        // the processors
    MIDIProcessor*          in_proc;
    MIDIProcessor*          out_proc;
    MIDIProcessor*          thru_proc;

    bool                    thru_enable;

        // additional TimeTick procedure

    MIDITick*               tick_proc;

// to keep track of notes on going to MIDI out

      MIDIMatrix out_matrix;


/* NEW BY NC
 * these keep track of the MIDI devices present in the OS
 * NOTE: this is only a temporary situation, probably in the future we'll use RtMidi for getting the number
 * and names of the devices
 */
    static const int DEVICENAME_LEN = 80;

    static char** in_dev_names;         // Array of char* which holds the names of MIDI in devices
    static char** out_dev_names;        // Array of char* which holds the names of MIDI out devices
    static unsigned int num_in_devs;    // Number of MIDI in devices installed
    static unsigned int num_out_devs;   // Number of MIDI out devices installed
};


#endif // _JDKMIDI_DRIVER_H
