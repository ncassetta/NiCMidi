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
#include "timer.h"


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

///
/// This structure is used by the MIDIInDriver to keep track of incoming messages. It holds
/// a MIDIMessage, a timestamp in milliseconds (from the start of the timer) and the id of
/// the MIDI in port from which the message comes.
///
struct MIDIRawMessage {
                                        MIDIRawMessage() : timestamp(0), port(0) {}
                                        MIDIRawMessage(const MIDIMessage& m, tMsecs t, int p) :
                                                        msg(m), timestamp(t), port(p) {}
        MIDIMessage                     msg;
        tMsecs                          timestamp;
        int                             port;
};

///
/// This is a queue of MIDIRawMessage
class MIDIRawMessageQueue {
    public:
        /// The constructor creates a queue of the given size. When the queue is full, older messages
        /// are pulled from the queue.

                                        MIDIRawMessageQueue(unsigned int size) :
                                                                    next_in(0), next_out(0), buffer(size) {}
        /// The destructor deletes all the MIDIRawMessage objects actually contained in the queue.
        virtual                         ~MIDIRawMessageQueue()      {}
        /// Empties the queue and turns into a NoOp all the messages contained.
        void                            Reset();
        /// Quickly empties the queue acting only on in and out indexes.
        void                            Flush()                     { next_out = next_in; }
        /// Add the given MIDIRawMessage as the last element in the queue. If the queue was full
        /// the first message is pulled out.
        void                            PutMessage(const MIDIRawMessage& msg);
        /// Gets the first MIDIRawMessage in the queue, pulling it out. It returns a reference
        /// to a static copy, which is valid until the next call to the function.
        MIDIRawMessage&                 GetMessage();
        /// Gets the n-th MIDIRawMessage in the queue, without pulling it out. It returns a
        /// direct reference to the message (and it's so faster than GetMessage(), which copies it)
        /// which is valid until an operation on the queue is done. If the queue has an actual
        /// size lesser than _n_, returns a NoOp message.
        MIDIRawMessage&                 PeekMessage(unsigned int n);
        /// Returns *true* is the queue is empty.
        bool                            IsEmpty() const             { return next_in == next_out; }
        /// Returns *true* if the queue has reached its max size (you can however add other messages,
        /// deleting the older ones.
        bool                            IsFull() const              { return ((next_in + 1) % buffer.size()) == next_out; }
        /// Returns the actual length of the queue.
        unsigned int                    GetLength() const           { return (next_in - next_out) % buffer.size(); }

    protected:
        unsigned int                    next_in;
        unsigned int                    next_out;
        std::vector<MIDIRawMessage>     buffer;
};


///
/// The MidiOutDriver class is a device that sends MIDI messages to an hardware MIDI out port.
/// Every MIDI out port is denoted by a specific id number, enumerated by the RtMidi class.
///
class MIDIOutDriver {
    public:
        /// Creates a MIDIOutDriver object sending messages to the given out port. The number of
        /// available MIDI out ports and their names can be retrieved by the
        /// MIDIManager::GetNumMIDIOutPorts() and MIDIManager::GetMIDIOutName() methods.
            // TODO: what to do if id is not valid?
                                MIDIOutDriver (int id);
        /// Closes the port and deletes the object.
        virtual                 ~MIDIOutDriver();

        /// Resets the driver to default conditions:
        /// - Hardware MIDI port closed (resets the open count)
        /// - No extrs processor (warning: this only sets the processor pointer to 0! The driver
        ///   doesn't own its processor).
        /// - Thru output channel: all
        virtual void            Reset();

        /// Opens the MIDI out port denoted by the id number given in the ctor. This usually requires
        /// a noticeable amount of time, so it's better not to immediately start to send messages.
        /// The function does nothing if the port is already open.
        virtual void            OpenPort();
        /// Closes the MIDI out port. The function does nothing if the port isn't open.
        virtual void            ClosePort();
        virtual void            ForcedClosePort();

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
        virtual void            SetThruChannel(char chan);
        int                     GetThruChannel() const          { return (int)thru_channel; }

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
        /// This function is called internally during the MIDI thru process. The user shouldn't
        /// call it directly.
        virtual void            MIDIThru(MIDITimedMessage msg);

    protected:
        /// Sends the message to the hardware MIDI port
        virtual void            HardwareMsgOut(const MIDIMessage &msg);

        MIDIProcessor*          processor;  ///< The out processor
        RtMidiOut*              port;       ///< The hardware port
        const int               port_id;    ///< The id of the port, used for opening it
        int                     num_open;   ///< Counts the number of OpenPort() calls

        char                    thru_channel;
                                            ///< The channel of the MIDI thru messages
        MIDIProcessorRechannelizer  rechannelizer;
                                            ///< Used internally for rechannelizing thru messagess

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
        /// Creates a MIDIInDriver object getting messages from the given in port. The number of
        /// available MIDI in ports and their names can be retrieved by the
        /// MIDIManager::GetNumMIDIInPorts() and MIDIManager::GetMIDIInName() methods.
        /// The incoming MIDI messages are queued into a MIDIRawMessageQueue (i.e. the messages are
        /// stamped with the system time in milliseconds and the port number), and you can get them
        /// with the methods GetMessage() and PeekMessage(). The default queue size is 256.
                                MIDIInDriver(int id, unsigned int queue_size = DEFAULT_QUEUE_SIZE);
        /// Closes the port and deletes the object.
        virtual                 ~MIDIInDriver();
        /// Closes the port and empties the in queue.
        virtual void            Reset();
        /// Opens the MIDI in port denoted by the id number given in the ctor. This usually requires
        /// a noticeable amount of time, so it's better not to immediately start to get messages.
        /// The function does nothing if the port is already open.
        virtual void            OpenPort();
        /// Closes the open MIDI in port. The function does nothing if the port is already closed.
        virtual void            ClosePort();
        virtual void            ForcedClosePort();
        /// Returns the id number of the MIDI in port.
        int                     GetPortId() const               { return port_id; }
        /// Returns the name of the port.
        std::string             GetPortName()                   { return port->getPortName(port_id); }
        /// Returns *true* is the port is open.
        bool                    IsPortOpen() const              { return port->isPortOpen(); }
        /// Returns *true* if the in queue is non-empty.
        bool                    CanGet() const                  { return in_queue.GetLength() > 0; }
        /// Returns the queue size.
        unsigned int            GetQueueSize() const            { return in_queue.GetLength(); }
        /// Locks the queue so it cannot be written by other threads (such as the Rtidi callback). You
        /// can then safely deal with its data.
        void                    LockQueue()                     { in_mutex.lock(); }
        /// Unlocks the queue.
        void                    UnlockQueue()                   { in_mutex.unlock(); }
        /// Gets the in processor.
        MIDIProcessor*          GetProcessor()                  { return processor; }
        const MIDIProcessor*    GetProcessor() const            { return processor; }
        /// Sets the in processor. If you want to eliminate a processor already set, call it
        /// with 0 as parameter.
        virtual void            SetProcessor(MIDIProcessor* proc);
        /// Sets the thru enable on/off and the MIDIOutDriver to which incoming messages are sent.
        /// \param f true or false
        /// \param driver the first time you set the thru on the out driver is undefined, so you
        /// *must* specify a driver (otherwise the thru isn't enabled and the function return *false*).
        /// In the subsequent calls, if you leave the default parameter, the already set driver is kept,
        /// otherwise a new driver is set.
        /// \return *false* only if you failed to set the MIDI thru on, *true* otherwise.
        /// \note The MIDI thru can be managed more easily by the class MIDIManager, which has methods
        /// involving both the in and out driver.
        virtual bool            SetThruEnable(bool f, MIDIOutDriver* driver = 0);
        /// Gets the thru status.
        bool                    GetThruEnable() const           { return thru_enable; }
        /// Sets the MIDI thru channel. You can specify -1 for omni mode (all incoming messages are sent
        /// to the thru out driver), otherwise you can specify a number between 0 ... 15. Other values are
        /// ignored.
        /// \note If you set a specific channel, all channel messages of other channels will not be sent.
        /// If you want to rechannelize these messages to a specific out channel you must use the
        /// MIDIOutDriver::SetThruChannel() method.
        virtual void            SetThruChannel(char chan);
        /// Returns the MIDI thru channel. If it is -1 the driver sends to the out all incoming messages,
        /// otherwise it discards all channel messages with a different channel.
        int                     GetThruChannel() const          { return (int)thru_channel; }
        /// Gets the next message in the queue, copying it into _msg_ (the message is deleted from
        /// the queue). Returns *true* if the queue was not empty (and _msg_ is valid), otherwiae *false*.
        virtual bool            InputMessage(MIDIRawMessage& msg);
        /// Gets the n-th message in the queue without deleting it (so the message remains
        /// available for other purposes). Returns *true* if such a message really exists in the queue
        /// (and _msg_ is valid), otherwiae *false*.
        virtual bool            PeekMessage(MIDIRawMessage& msg, unsigned int n);

        /// This is the default queue size.
        static const unsigned int       DEFAULT_QUEUE_SIZE = 256;

protected:

        /// This is the RtMidi callback function (you must not call it directly)
        static void             HardwareMsgIn(double time,
                                              std::vector<unsigned char>* msg_bytes,
                                              void* p);

        MIDIProcessor*          processor;      ///< The in processor

        bool                    thru_enable;    ///< Sets the MIDI thru on/off
        char                    thru_channel;   ///< The receiving channel (-1 for omni)
        MIDIOutDriver*          thru_driver;    ///< The driver to which thru messages are sent

        RtMidiIn*               port;           ///< The hardware port
        const int               port_id;        ///< The id of the port, used for opening it
        int                     num_open;

        MIDIRawMessageQueue     in_queue;       ///< The incoming message queue (see MIDIRawMessage)
        std::recursive_mutex    in_mutex;       ///< Locks/unlocks the queue
};


#endif // _JDKMIDI_DRIVER_H
