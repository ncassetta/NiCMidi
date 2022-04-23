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
/// Contains the definition of the classes MIDIOutDriver and MIDIInDriver, used by the library to
/// communicate with hardware MIDI ports.


#ifndef _NICMIDI_DRIVER_H
#define _NICMIDI_DRIVER_H

#include "msg.h"
#include "processor.h"
#include "timer.h"


#include "../rtmidi-4.0.0/RtMidi.h"

#include <vector>
#include <string>
#include <mutex>
//#include <atomic>


// TODO: implements RtMidi functions (error callback, selection of input, etc.)


/// \addtogroup GLOBALS
///@{

/// This item only affects AllNotesOff() function. All modern MIDI devices should respond to all notes off
/// messages so usually there is no need to stop notes sending Note off messages.
/// If you set this to 1 the driver will keep track of all sounding notes and, if the AllNotesOff() method is
/// called, will send a Note Off message for each one, plus a damper off message for every channel. This is
/// quite expensive, so turn this on only if you experience notes sounding when you stop a sequencer.
#define DRIVER_USES_MIDIMATRIX 0
#if DRIVER_USES_MIDIMATRIX
   #include "matrix.h"
#endif // DRIVER_USES_MIDIMATRIX
///@}


// EXCLUDED FROM DOCUMENTATION BECAUSE UNDOCUMENTED
// Used by the MIDIInDriver to keep track of incoming messages. It holds
// a MIDIMessage, a timestamp in milliseconds (from the start of the timer) and the id of
// the MIDI in port from which the message comes.
struct MIDIRawMessage {
                                        MIDIRawMessage() : timestamp(0), port(0) {}
                                        MIDIRawMessage(const MIDIMessage& m, tMsecs t, int p) :
                                                        msg(m), timestamp(t), port(p) {}
        MIDIMessage                     msg;        // The MIDI Message received from the port
        tMsecs                          timestamp;  // The absolute time in msecs
        int                             port;       // The id of the MIDI in port which received the message
};


// EXCLUDED FROM DOCUMENTATION BECAUSE UNDOCUMENTED
// This is a queue of MIDIRawMessage
class MIDIRawMessageQueue {
    public:
        // The constructor creates a queue of the given size. When the queue is full, older messages
        // are pulled from the queue.

                                        MIDIRawMessageQueue(unsigned int size) :
                                                                    next_in(0), next_out(0), buffer(size) {}
        // The destructor deletes all the MIDIRawMessage objects actually contained in the queue.
        virtual                         ~MIDIRawMessageQueue()      {}
        // Empties the queue and turns into a NoOp all the messages contained.
        void                            Reset();
        // Quickly empties the queue acting only on in and out indexes.
        void                            Flush()                     { next_out = next_in; }
        // Adds the given MIDIRawMessage as the last element in the queue. If the queue was full
        // the first message is pulled out.
        void                            PutMessage(const MIDIRawMessage& msg);
        // Gets the first MIDIRawMessage in the queue, pulling it out. It returns a reference
        // to a static copy, which is valid until the next call to the function.
        MIDIRawMessage&                 GetMessage();
        // Gets the n-th MIDIRawMessage in the queue, without pulling it out. It returns a
        // direct reference to the message (and it's so faster than GetMessage(), which copies it)
        // which is valid until an operation on the queue is done. If the queue has an actual
        // size lesser than _n_, returns a NoOp message.
        MIDIRawMessage&                 ReadMessage(unsigned int n);
        // Returns *true* is the queue is empty.
        bool                            IsEmpty() const             { return next_in == next_out; }
        // Returns *true* if the queue has reached its max size (you can however add other messages,
        // deleting the older ones.
        bool                            IsFull() const              { return ((next_in + 1) % buffer.size()) == next_out; }
        // Returns the actual length of the queue.
        unsigned int                    GetLength() const           { return (next_in - next_out) % buffer.size(); }

    protected:
        unsigned int                    next_in;
        unsigned int                    next_out;
        std::vector<MIDIRawMessage>     buffer;
};


///
/// Sends MIDI messages to an hardware MIDI out port.
/// Every MIDI out port is denoted by a specific id number, enumerated by the RtMidi class, and by a
/// name, given by the OS; this class communicates between the hardware ports and the other library
/// classes. You can set a MIDIProcessor for processing outgoing MIDI messages.
///
/// When the program starts, the static MIDIManager searches for all the hardware ports in the system and
/// creates a driver for everyone of them, so you find them ready to use.
class MIDIOutDriver {
    public:
        /// Creates a MIDIOutDriver object which can send MIDI messages to the given hardware out port.
        /// \param id The id of the hardware port. Numbers of the ports and their names can be retrieved
        /// by the MIDIManager::GetNumMIDIOuts() and MIDIManager::GetMIDIOutName() static methods.
        /// \note If id is not valid or the function fails, a dummy port with no functionality is created.
        /// \note As said in the class description, the drivers are created automatically by the
        /// MIDIManager when the program starts, so usually you must not create or destroy them by yourself.
                                MIDIOutDriver (int id);
        /// Closes the hardware port and deletes the object.
        virtual                 ~MIDIOutDriver();

        /// Resets the driver to default conditions:
        /// - Hardware MIDI port closed (resets the open count)
        /// - No extra processor (warning: this only sets the processor pointer to 0! The driver
        ///   doesn't own its processor).
        virtual void            Reset();

        /// Returns the id number of the hardware out port
        int                     GetPortId() const               { return port_id; }
        /// Returns the name of the hardware out port.
        std::string             GetPortName()                   { return port->getPortName(port_id); }
        /// Returns **true** is the hardware port is open.
        bool                    IsPortOpen() const              { return port->isPortOpen(); }
        /// Returns a pointer to the out processor.
        MIDIProcessor*          GetOutProcessor()               { return processor; }
        /// Returns a pointer to the out processor.
        const MIDIProcessor*    GetOutProcessor() const         { return processor; }

        /// Sets the out processor, which can manipulate all outgoing messages (see MIDIProcessor). If you
        /// want to eliminate a processor already set, call it with 0 as parameter (this only sets the processor
        /// pointer to 0! The driver doesn't own its processor).
        virtual void            SetOutProcessor(MIDIProcessor* proc)
                                                                { processor = proc; }

        /// Opens the hardware out port. This usually requires a noticeable amount of time, so it's better
        /// not to immediately start to send messages. If the port is already open the object remembers how many
        /// times it was open, so a corresponding number of ClosePort() must be called to effectively close the port.
        virtual void            OpenPort();
        /// Closes the hardware out port. If the port was open more than once it only decrements the count
        /// (leaving it open), while it does nothing if the port is already close. If you want to force
        /// the closure call Reset().
        virtual void            ClosePort();
        /// Turns off all the sounding notes on the port (or on the given MIDI channel). This is normally
        /// done by sending an All Notes Off message, but you can change this behaviour (see \ref DRIVER_USES_MIDIMATRIX).
        /// See also \ref NUMBERING.
        /// \param chan if you left the default silences all channels, otherwise you can give an unique channel
        /// to turn off
        virtual void            AllNotesOff(int chan = -1);
        /// Makes a copy of the message, processes it with the out processor and then sends it to
        /// the hardware port. If the port is busy waits 1 msec and retries until \ref DRIVER_MAX_RETRIES
        /// is reached.
            // TODO: actually it writes to cerr, Should we raise an exception?
        virtual void            OutputMessage(const MIDITimedMessage& msg);

    protected:
        /// The maximum number of retries the method OutputMessage() will try before hanging (and skipping a message).
        static const int        DRIVER_MAX_RETRIES = 100;
        /// The number of milliseconds the driver waits after sending a MIDI system exclusive message.
        static const int        DRIVER_WAIT_AFTER_SYSEX = 20;
        /// Sends the message to the hardware MIDI port using the RtMidi library functions.
        virtual void            HardwareMsgOut(const MIDIMessage &msg);

       /// \cond EXCLUDED
        MIDIProcessor*          processor;  // The out processor
        RtMidiOut*              port;       // The hardware port
        const int               port_id;    // The id of the port
        int                     num_open;   // Counts the number of OpenPort() calls
        std::recursive_mutex    out_mutex;  // Used internally for thread safe operating

#if DRIVER_USES_MIDIMATRIX
        MIDIMatrix              out_matrix; // To keep track of notes on going to MIDI out
#endif // DRIVER_USES_MIDIMATRIX
        /// \endcond

    private:
        // this vector is used by HardwareMsgOut to feed the port
        std::vector<unsigned char>      msg_bytes;
};




///
/// Receives MIDI messages from an hardware MIDI in port.
/// Every MIDI in port is denoted by a specific id number, enumerated by the RtMidi class, and by a
/// name, given by the OS; this class communicates between the hardware ports and the other library
/// classes. The incoming MIDI messages are stamped with the system time in milliseconds and the
/// port number (see the MIDIRawMessage struct) and put in a queue; you can get them with the
/// InputMessage() and ReadMessage() methods. Moreover you can set a MIDIProcessor for processing them.
///
/// When the program starts, the static MIDIManager searches for all the hardware ports in the system and
/// creates a driver for everyone of them, so you find them ready to use.
class MIDIInDriver {
    public:
        /// Creates a MIDIInDriver object which can receive MIDI messages from the given hardware in port.
        /// \param id The id of the hardware port. Numbers of the ports and their names can be retrieved
        /// by the MIDIManager::GetNumMIDIOutPorts() and MIDIManager::GetMIDIOutName() static methods.
        /// \param queue_size The size of the queue; you could try to change this if you have trouble in
        /// receiving MIDI messages from the hardware, otherwise left unchanged (default size is 256).
        /// \note If id is not valid or the function fails, a dummy port with no functionality is created.
        /// \note As said in the class description, the drivers are created automatically by the
        /// MIDIManager when the program starts, so usually you must not create or destroy them by yourself.
                                MIDIInDriver(int id, unsigned int queue_size = DEFAULT_QUEUE_SIZE);
        /// Closes the hardware port and deletes the object.
        virtual                 ~MIDIInDriver();
        /// Resets the driver to default conditions:
        /// - Hardware MIDI port closed (resets the open count)
        /// - In queue empty
        /// - No extra processor (this only sets the processor pointer to 0! The driver
        ///   doesn't own its processor).
        virtual void            Reset();

        /// Returns the id number of the hardware in port.
        int                     GetPortId() const               { return port_id; }
        /// Returns the name of the hardware in port.
        std::string             GetPortName()                   { return port->getPortName(port_id); }
        /// Returns **true** is the hardware port is open.
        bool                    IsPortOpen() const              { return port->isPortOpen(); }
        /// Returns **true** if the queue is non-empty.
        bool                    CanGet() const                  { return in_queue.GetLength() > 0; }
        /// Returns the queue size.
        unsigned int            GetQueueSize() const            { return in_queue.GetLength(); }
        /// Returns a pointer to the in processor.
        MIDIProcessor*          GetProcessor()                  { return processor; }
        /// Returns a pointer to the in processor.
        const MIDIProcessor*    GetProcessor() const            { return processor; }

        /// Sets the in processor, which can manipulate all incoming messages (see MIDIProcessor). If you
        /// want to eliminate a processor already set, call it with 0 as parameter (this only sets the
        /// processor pointer to 0! The driver doesn't own its processor).
        virtual void            SetProcessor(MIDIProcessor* proc);

        /// Opens the hardware in port. This usually requires a noticeable amount of time, so it's better
        /// not to immediately start to send messages. If the port is already open the object remembers how many
        /// times it was open, so a corresponding number of ClosePort() must be called to effectively close the port.
        virtual void            OpenPort();
        /// Closes the hardware out port. If the port was open more than once it only decrements the count
        /// (leaving it open), while it does nothing if the port is already close. If you want to force
        /// the closure call Reset().
        virtual void            ClosePort();
        /// Locks the queue so it cannot be written by other threads (such as the RtMidi callback). You
        /// can then safely inspect and get its data, unlocking it when you have finished and want to get
        /// new messages.
        void                    LockQueue()                     { in_mutex.lock(); }
        /// Unlocks the queue (see LockQueue()).
        void                    UnlockQueue()                   { in_mutex.unlock(); }
        /// Empties the queue in a thread-safe way.
        void                    FlushQueue();
        /// Gets the next message in the queue, copying it into _msg_ (the message is deleted from
        /// the queue).
        /// \param [out] msg the message got from the queue
        /// \return **true** if the queue was not empty (and _msg_ is valid), otherwise **false**.
        virtual bool            InputMessage(MIDIRawMessage& msg);
        // TODO: the processor processes it?
        /// Gets the n-th message in the queue without deleting it (so the message remains
        /// available for other purposes).
        /// \param [out] msg is a direct reference to the queued message, so it's your
        /// responsability not to alter it
        /// \param n the message index in the in queue
        /// \return **true** if such a message really exists in the queue (and _msg_ is valid), otherwise **false**.
        virtual bool            ReadMessage(MIDIRawMessage& msg, unsigned int n);

protected:

        /// This is the RtMidi callback function (you must not call it directly)
        static void             HardwareMsgIn(double time,
                                              std::vector<unsigned char>* msg_bytes,
                                              void* p);

        /// \cond EXCLUDED
        // This is the default queue size.
        static const unsigned int       DEFAULT_QUEUE_SIZE = 256;

        MIDIProcessor*          processor;      // The in processor
        RtMidiIn*               port;           // The hardware port
        const int               port_id;        // The id of the port
        int                     num_open;       // Counts the number of OpenPort() calls

        MIDIRawMessageQueue     in_queue;       // The incoming message queue (see MIDIRawMessage)
        std::recursive_mutex    in_mutex;       // Locks/unlocks the queue
        /// \endcond
};


#endif // _JDKMIDI_DRIVER_H
