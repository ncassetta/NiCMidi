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
/// Contains the definition of the pure virtual MIDIProcessor class and its specializations
/// MIDIMultiProcessor, MIDIProcessorTransposer, MIDIProcessorRechannelizer, MIDIProcessorPrinter.
/// These are objects that can manipulate a MIDITimedMessage.


#ifndef _NICMIDI_PROCESSOR_H
#define _NICMIDI_PROCESSOR_H

#include "msg.h"

#include <iostream>
#include <vector>


///
/// A pure virtual class implementing an object that can manipulate a MIDI message, inspecting or changing
/// its content. Many library objects, such as MIDIInDriver, MIDIOutDriver, MIDIThru and MIDISequencer, allow you
/// to insert a MIDIProcessor into the flow of outgoing or ingoing messages with their **SetProcessor()** method.
/// The library comes with some useful subclasses of it.
///
class MIDIProcessor {
    public:
        /// The constructor.
                                        MIDIProcessor() {}
        /// The destructor.
        virtual                         ~MIDIProcessor() {}
        /// You should implement this pure virtual method in order to return the MIDIProcessor to its
        /// initial state.
        virtual void                    Reset() = 0;
        /// The Process() pure virtual method. It takes as parameter a MIDITimedMessage (which can
        /// be modified by the method) and returns a boolean, which can be used for filtering
        /// purposes (selecting some messages) or for notifying the result of the processing.
        virtual bool                    Process(MIDITimedMessage *msg) = 0;
};


///
/// Allows the user to queue many MIDIProcessor objects, passing the result of each one to the next.
///
class MIDIMultiProcessor : public MIDIProcessor {
    public:

        /// The constructor creates an object with empty queue.
                                        MIDIMultiProcessor() : process_mode(MODE_CONTINUE) {}
        /// Empties the processors queue, removing all their pointers from it.
        virtual void                    Reset();
        /// Returns a pointer to the MIDIProcessor at the given position.
        MIDIProcessor*                  GetProcessor(unsigned int pos)          { return processors[pos]; }
        /// Returns a pointer to the MIDIProcessor at the given position.
        const MIDIProcessor*            GetProcessor(unsigned int pos) const    { return processors[pos]; }
        /// Returns the processing mode (see SetProcessMode()).
        int                             GetProcessMode() const                  { return process_mode; }
        /// Inserts a MIDIProcessor object into the queue.
        /// \param proc the MIDIProcessor to be inserted (it is **not** owned by the MIDIMultiProcessor)
        /// \param pos the position in the queue. If you leave the default value the processor will be
        /// appended to the queue, otherwise the new processor substitutes the old (the method does nothing
        /// if _pos_ is not in the appropriate range)
        void                            SetProcessor(MIDIProcessor* proc, int pos = -1);
        /// Removes the MIDIProcessor at the given position. It only removes the processor pointer from
        /// the queue, and does nothing if _pos_ is not in the appropriate range.
        void                            RemoveProcessor(unsigned int pos);
        /// Searches the given MIDIProcessor in the queue and removes it (it does nothing
        /// if the processor is not in the queue). It only removes the processor pointer from the queue.
        void                            RemoveProcessor(const MIDIProcessor* proc);
        /// Determines the behaviour of the MIDIMultiProcessor if any of the processors return
        /// **false**. You can set these three values:
        /// - MODE_IGNORE :   the MIDIMultiProcessor ignores return values and always returns **true**
        /// - MODE_CONTINUE : the MIDIMultiProcessor performs all the processing, and returns **false**
        /// if any processor returned **false**, **true** otherwise (this is the default when the object is created)
        /// - MODE_STOP :     the MIDIMultiProcessor stops processing when a processor returns **false**
        /// and returns **false**, **true** otherwise.
        void                            SetProcessMode(int mode)        { process_mode = mode; }
        /// The Process method. It passes the MIDI message to the first processor and subsequently to
        /// all other processors in the queue. If any of the processors returns **false** the behaviour
        /// and the returned value are defined by the SetProcessMode() method.
        virtual bool                    Process(MIDITimedMessage *msg);
        /// These are the values to be given as parameters to the SetProcessMode() method.
        enum { MODE_IGNORE, MODE_CONTINUE, MODE_STOP };

    protected:
        /// \cond EXCLUDED
        std::vector<MIDIProcessor*>     processors;     // The array of processor pointers
        int                             process_mode;   // The process mode parameter
        /// \endcond
};


///
/// A MIDIProcessor which shifts the pitch of MIDI note and polyphonic pressure messages by a given amount of semitones.
/// You can set a different amount for each channel, or the same amount for all the channels.
///
class MIDIProcessorTransposer : public MIDIProcessor {
    public:
        /// The constructor.
                                        MIDIProcessorTransposer();
        /// Resets to the default status (no transposing on all channels).
        virtual void                    Reset();
        /// Gets the transposing amount (in semitones) for the given channel.
        /// See \ref NUMBERING
        int                             GetChannelTranspose (int chan) const        { return trans_amount[chan]; }
        /// Sets the transposing amount.
        /// \param chan the channel to be transposed. See \ref NUMBERING
        /// \param trans The amount of transposing in semitones
        void                            SetChannelTranspose (int chan, int trans)   { trans_amount[chan] = trans; }
        /// Sets the same transposing amount (in semitones) for all the channels.
        void                            SetAllTranspose (int trans);
        /// The process() method. It affects only Note on, Note off and Poly pressure
        /// messages, changing their note number according to the transposing amount. If the resulting
        /// note number is not in the MIDI range (0 ... 127) the message is left unchanged and
        /// the method returns **false**, otherwise it returns **true**.
        virtual bool                    Process (MIDITimedMessage *msg);

    protected:
        /// \cond EXCLUDED
        int                             trans_amount[16];   // The transposing amount
        /// \endcond
};

///
/// A MIDIProcessor which changes the channel of all processed MIDI channel messages.
/// You can set a one-to-one correspondence between channels, or send all channel messages
/// to a single channel.
///
class MIDIProcessorRechannelizer : public MIDIProcessor {
    public:
        /// The constructor.
                                        MIDIProcessorRechannelizer();

        /// Resets the default status (no rechannelizing).
        void                            Reset();
        /// Gets the corresponding channel for the (source) _src_chan_.
        /// See \ref NUMBERING
        int                             GetRechanMap (int src_chan) const           { return rechan_map[src_chan]; }
        /// Set the correspondence between two channels (will transform _src_chan_ into _dest_chan_).
        /// See \ref NUMBERING, however you can set _dest_chan_ to -1 (see the Process() method).
        void                            SetRechanMap (int src_chan, int dest_chan)  { rechan_map[src_chan] = dest_chan; }
        /// Sends all channel messages to channel _dest_chan_.
        void                            SetAllRechan (int dest_chan);
        /// The Process() method. If _msg_ is not a channel message it is left unchanged
        /// and the function returns **true**. Otherwise its channel is changed according to the
        /// rechannel map. If its destination channel is -1 the message remains unchanged but
        /// the method returns **false** (you can use this for filtering messages by channel)
        /// otherwise returns **true**.
        virtual bool                    Process(MIDITimedMessage *msg);

    protected:
        /// \cond EXCLUDED
        int                             rechan_map[16];         // The rechannel map
        /// \endcond
};


///
/// A MIDIProcessor which prints a human-readable description of the processed messages to a std::ostream.
/// Useful for debugging purposes (you could want to see, for example, all messages passing through a driver).
/// You can turn the printing on/off.
///
class MIDIProcessorPrinter : public MIDIProcessor {
    public:
        /// The constructor sets the std::ostream that will print the messages (default: std::cout).
                                        MIDIProcessorPrinter(std::ostream& stream = std::cout, unsigned char from_1 = 0) :
                                                             print_on(true), chan_from_1(from_1 != 0), ost(stream) {}
        /// Same of SetPrint(true)
        virtual void                    Reset()                                     { print_on = true; }
        /// Returns the numbering of the first MIDI channel in message printing (0 or 1).
        /// See \ref NUMBERING.
        int                             GetChanFrom() const                         { return chan_from_1; }
        /// Returns the printing status.
        bool                            GetPrint() const                            { return print_on; }
        /// Sets the numbering of MIDI channels in message printing. If c == 0 they will be numbered 0 ... 15,
        /// else 1 ... 16. See \ref NUMBERING.
        void                            SetChanFrom(unsigned char c)                { chan_from_1 = (c != 0); }
        /// Sets the printing on and off (default is on).
        void                            SetPrint(bool on_off)                       { print_on = on_off; }
        /// The Process method. It prints a human-readable description of the message to the std::ostream given
        /// in the constructor. The message is left unchanged and always returns **true**.
        virtual bool                    Process(MIDITimedMessage *msg);

    protected:
        /// \cond EXCLUDED
        bool                            print_on;           // The on/off printing flag
        unsigned char                   chan_from_1;        // Starting number for MIDI channels (0 or 1)
        std::ostream&                   ost;                // The out stream
        /// \endcond
};


#endif
