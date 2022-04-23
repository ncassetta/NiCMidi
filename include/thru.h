/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2021, 2022  Nicola Cassetta
 *   https://github.com/ncassetta/NiCMidi
 *
 *   This file is part of NiCMidi.
 *
 *   NiCMidi is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation, either version 3 of
 *   the License, or (at your option) any later version.
 *
 *   NiCMidi is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with NiCMidi. If not, see <http://www.gnu.org/licenses/>.
 */


/// \file
/// Contains the definition of the class MIDIThru.


#ifndef _NICMIDI_THRU_H
#define _NICMIDI_THRU_H

#include "driver.h"
#include "tick.h"


///
/// A MIDITickComponent which immediately echoes to an out MIDI port all messages incoming
/// from an in MIDI port.
/// You can choose the in and out ports, select an unique channel for receiving and sending messages
/// (or receive and send on any channel) and insert a MIDIProcessor between in and out ports for messages
/// elaboration.
/// \note Remember that you must call the MIDIManager::AddTick() to make effective the StaticTickProc(), then
/// you can call Start() and Stop() methods to enable or disable the thru.
///
class MIDIThru : public MIDITickComponent {
    public:
        /// The constructor. It raises an exception if in your system there are no MIDI in or MIDI out ports,
        /// otherwise sets them to the OS id 0 ports.
        /// \exception RtMidiError::INVALID_DEVICE if in the system are not present MIDI out or MIDI in ports
                                MIDIThru();
        /// The destructor.
        virtual                 ~MIDIThru();
        /// Resets the class to initial status:
        /// - In and out ports set to the OS id 0
        /// - No extra processor (warning: this only sets the processor pointer to 0! The driver
        ///   doesn't own its processor).
        /// - Thru input and output channel: all
        /// - Thru disabled;
        virtual void            Reset();

        /// Returns the number of the MIDI in port from which messages are actually being received.
        unsigned int            GetInPort() const               { return in_port; }
        /// Returns a pointer to the MIDIOutDriver to whom messages are actually being sent.
        unsigned int            GetOutPort() const              { return out_port; }
        /// Returns a pointer to the MIDIProcessor attached to the thru (see SetProcessor()).
        MIDIProcessor*          GetProcessor()                  { return processor; }
        /// Returns a pointer to the MIDIProcessor attached to the thru (see SetProcessor()).
        const MIDIProcessor*    GetProcessor() const            { return processor; }
        /// Returns the thru in channel (see SetInChannel())
        int                     GetInChannel() const            { return (int)in_channel; }
        /// Returns the thru out channel (see SetOutChannel())
        int                     GetOutChannel() const            { return (int)out_channel; }
        /// Selects the hardware in port from which messages will be received.
        /// This can be done even if thru is already enabled.
        /// \return **true** if _port_ is a valid port number, **false** otherwise.
        virtual bool            SetInPort(unsigned int port);
        /// Selects the hardware out port to whom messages will be sent.
        /// This can be done even if thru is already enabled.
        /// \return **true** if _port_ is a valid port number, **false** otherwise.
        virtual bool            SetOutPort(unsigned int port);
        /// Sets the out processor, which can manipulate messages arrived to the in port before they are sent
        /// to the out port (see MIDIProcessor).
        /// If you want to eliminate a processor already set, call it with 0 as parameter (this only sets the processor
        /// pointer to 0! The class doesn't own its processor).
        virtual void            SetProcessor(MIDIProcessor* proc);
        /// Sets the channel for incoming thru messages.
        /// \param chan 0 ... 15: the thru will accept only messages with a specific channel; -1: the thru will
        /// accept all messages coming from the in port (this is the default). Non channel messages are always received.
        /// \return **true** if _chan_ is a valid channel number, **false** otherwise.
        virtual bool            SetInChannel(int chan);
        /// Sets the channel for outgoing thru messages.
        /// \param chan 0 ... 15: the thru will redirect all messages to a specific channel; -1: the thru will leave
        /// channel messages unchanged (this is the default).
        /// \return **true** if _chan_ is a valid channel number, **false** otherwise
        virtual bool            SetOutChannel(int chan);
        /// Starts the MIDI thru.
        virtual void            Start();
        /// Stops the MIDI thru.
        virtual void            Stop();

    protected:
        /// Implements the static method inherited from MIDITickComponent and called at every timer tick.
        /// It only calls the member TickProc().
        static void                     StaticTickProc(tMsecs sys_time, void* pt);
        /// Implements the pure virtual method inherited from MIDITickComponent (you must not call it directly).
        virtual void                    TickProc(tMsecs sys_time);


        /// \cond EXCLUDED
        unsigned int            in_port;        // The in port id
        unsigned int            out_port;       // The out port id
        signed char             in_channel;     // The in channel (0 .. 15, -1 for any channel)
        signed char             out_channel;    // The out channel (0 .. 15, -1 for any channel)

        MIDIProcessor*          processor;      // The MIDIProcessor you can plug to the thru
        /// \endcond

    private:
        void                    SilentOut();    // Internal use

};


#endif // THRU_H_INCLUDED
