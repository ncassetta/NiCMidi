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


#include "../rtmidi-2.1.1/RtMidi.h"

#include <vector>
#include <string>
#include <mutex>
#include <atomic>


// TODO: resolve using mutex or atomic
// TODO: implements RtMidi functions (error callback, selection of input, etc.)
// TODO: finish documentation


/// This item only affects AllNotesOff() function. All modern MIDI devices should respond to all notes off
/// messages so usually there is no need to stop notes sending Note off messages.
/// If you set this to 1 the driver will keep track of all sounding notes and, if the AllNotesOff() method is
/// called, will send a Note Off message for each one, plus a damper off message for every channel. This is
/// quite expensive, so turn this on only if you experience notes sounding when you stop a sequencer.
#define DRIVER_USES_MIDIMATRIX 0
/// This is the maximum number of retries the method OutputMessage() will try before hanging (and skipping
/// a message).
#define DRIVER_MAX_RETRIES 100
/// This is the number of milliseconds the driver waits after sending a MIDI system exclusive message.
#define DRIVER_WAIT_AFTER_SYSEX 20

class MIDIOutDriver {
    public:
        /// Creates a MIDIOutDriver object sending messages to the given out port. The number of
        /// available MIDI out ports and their names can be retrieved by the
        /// MIDIManager::GetNumMIDIOutPorts() and MIDIManagerGetMIDIOutName() methods.
            // TODO: what to do if id is not valid?
                                MIDIOutDriver (int id);
        /// Close the port and delete the object.
        virtual                 ~MIDIOutDriver();

        /// Returns the id number of the MIDI out port
        int                     GetPortId() const               { return port_id; }
        /// Returns the name of the port.
        std::string             GetPortName()                   { return port->getPortName(port_id); }
        /// Returns *true* is the port is open.
        bool                    IsPortOpen() const              { return port->isPortOpen(); }
        /// Gets the out processor.
        MIDIProcessor*          GetOutProcessor()               { return processor; }
        const MIDIProcessor*    GetOutProcessor() const         { return processor; }
        /// Sets the out processor. If you want to eliminate a processor already set, call it
        /// with 0 as parameter.
        virtual void            SetOutProcessor(MIDIProcessor* proc)
                                                                { processor = proc; }
        /// Opens the MIDI out port denoted by the id number given in the ctor.. This usually requires
        /// a noticeable amount of time, so it's better not to immediately start to send messages.
        /// The function does nothing if the port is already open.
        virtual void            OpenPort();
        /// Closes the MIDI out port. The function does nothing if the port isn't open.
        virtual void            ClosePort();
        /// Turns off all the sounding notes on the given MIDI channel. This is normally done by
        /// sending an All Notes Off message, but you can change this behaviour
        /// (\see DRIVER_USES_MIDIMATRIX).
        virtual void            AllNotesOff(int chan);
        /// Turns off all sounding notes on the port. \see  AllNotesOff(chan).
        virtual void            AllNotesOff();
        /// Makes a copy of the message, processes it with the out processor and then sends it to
        /// the hardware port. If the port is busy waits 1 msec and retries until DRIVER_MAX_RETRIES
        /// is reached.
            // TODO: actually it writes to cerr, Should raise an exception?
        virtual void            OutputMessage(const MIDITimedMessage& msg);

    protected:
        /// Sends the message to the hardware MIDI port
        virtual void            HardwareMsgOut(const MIDIMessage &msg);

        MIDIProcessor*          processor;  ///< The out processor
        RtMidiOut*              port;       ///< The hardware port
        const int               port_id;    ///< The id of the port, used for opening it

        //std::recursive_mutex    out_mutex;

        std::atomic<unsigned char> busy;        // TODO: use the mutex???

#if DRIVER_USES_MIDIMATRIX
        MIDIMatrix              out_matrix; ///< To keep track of notes on going to MIDI out
#endif // DRIVER_USES_MIDIMATRIX
    private:
        /// this vector is used by HardwareMsgOut to feed the port
        std::vector<unsigned char>      msg_bytes;
};





class MIDIInDriver {
    public:

                                MIDIInDriver(int queue_size );
        virtual                 ~MIDIInDriver();

        //virtual void            Reset();

        /// Returns the id number of the MIDI out port
        int                     GetPortId() const               { return port_id; }
        /// Returns the name of the port.
        std::string             GetPortName()                   { return port->getPortName(port_id); }
        /// Returns *true* is the port is open.
        bool                    IsPortOpen() const              { return port->isPortOpen(); }
        /// Gets the in processor.
        MIDIProcessor*          GetInProcessor()                { return processor; }
        const MIDIProcessor*    GetInProcessor() const          { return processor; }
        /// Sets the in processor. If you want to eliminate a processor already set, call it
        /// with 0 as parameter.
        virtual void            SetInProcessor(MIDIProcessor* proc)
                                                                { processor = proc; }
        /// Opens the MIDI in port _id_
        virtual void            OpenPort();

        /// Closes the open MIDI in port
        virtual void            ClosePort();

        virtual bool            InputMessage(MIDITimedMessage& msg);


protected:

        virtual bool            HardwareMsgIn( MIDIMessage &msg );



        // the processor
        MIDIProcessor*          processor;  ///< The in processor
        RtMidiIn*               port;       ///< The hardware port
        const int               port_id;    ///< The id of the port, used for opening it

        std::atomic<unsigned char> busy;        // TODO: use the mutex???
private:
        /// this vector is used by HardwareMsgOut to feed the port
        std::vector<unsigned char>      msg_bytes;
};


#endif // _JDKMIDI_DRIVER_H
