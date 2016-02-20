/*
 *  libjdkmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  slightly modified by NICOLA CASSETTA
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

#ifndef _JDKMIDI_DRIVERWIN32_H
#define _JDKMIDI_DRIVERWIN32_H


#include "driver.h"
#include "sequencer.h"

#ifdef _WIN32
#include "windows.h"
#include "mmsystem.h"


class MIDIDriverWin32 : public MIDIDriver {
public:
                            MIDIDriverWin32( int queue_size = DEFAULT_QUEUE_SIZE );
    virtual                 ~MIDIDriverWin32();



        // These are the implementations of the pure virtual functions in base class

        // Opens the MIDI in port _id_
    virtual bool            OpenMIDIInPort ( int id = DEFAULT_IN_PORT );

        // Opens the MIDI out port _id_
    virtual bool            OpenMIDIOutPort ( int id = DEFAULT_OUT_PORT );

        // Closes the open MIDI in port
    virtual void            CloseMIDIInPort();

        // Closes the open MIDI out port
    virtual void            CloseMIDIOutPort();

        // Resets open MIDI out port
    virtual void            ResetMIDIOut();

        // Start the hardware timer for playing MIDI. Default time resolution is 1 ms
    virtual bool            StartTimer ( int resolution_ms = DEFAULT_TIMER_RESOLUTION );

        // Stops the hardware timer
    virtual void            StopTimer();

        // Sends the MIDITimedBigMessage _msg_ to the open MIDI out port
    virtual bool            HardwareMsgOut ( const MIDITimedBigMessage &msg );


protected:

    static const int DEFAULT_IN_PORT = MIDI_MAPPER;         // The default in port
	static const int DEFAULT_OUT_PORT = MIDI_MAPPER;        // The default out port
    static const int DEFAULT_QUEUE_SIZE = 256;              // The default queue size
    static const int DEFAULT_SYSEX_BUFFER_SIZE = 384;       // The default sysex buffer size

    static bool             InitDevices();                  // Ask Windows for MIDI devices name and number; used in initialization


    static void CALLBACK win32_timer(
        UINT wTimerID,
        UINT msg,
        DWORD dwUser,
        DWORD dw1,
        DWORD dw2 );

    static void CALLBACK win32_midi_in(
        HMIDIIN hMidiIn,
        UINT wMsg,
        DWORD dwInstance,
        DWORD dwParam1,
        DWORD dwParam2 );

    HMIDIIN                 in_handle;                  // Windows handle to the MIDI in port
    HMIDIOUT                out_handle;                 // Windows handle to the MIDI out port
    int                     timer_id;                   // Windows id of the hardware timer
    int                     timer_res;                  // Resolution of the hardware timer

    bool                    in_open;                    // True if the MIDI in port is open
    bool                    out_open;                   // True if the MIDI out port is open
    bool                    timer_open;                 // True if the timer is open

    int                     sysex_buffer_size;          // Size of the sysex buffer
    char*                   sysex_buffer;               // Buffer for sysex temporary storage

    static bool             init_devices_flag;          // this is used only for initializing static members of MIDIDriver

};


#endif  // _WIN32

#endif // _JDKMIDI_DRIVERWIN32_H
