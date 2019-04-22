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
/*
** Copyright 2016 By N. Cassetta
** myjdkmidi library
**
** CHECKED with jdksmidi. NO CHANGES
*/

#ifndef _JDKMIDI_PROCESS_H
#define _JDKMIDI_PROCESS_H

#include "msg.h"

#include <iostream>
#include <vector>

/// \file
/// Contains the definition of the pure virtual MIDIProcessor class and its specializations
/// MIDIMultiProcessor, MIDIProcessorTransposer, MIDIProcessorRechannelizer, MIDIProcessorPrinter.
/// These are devices that can manipulate a MIDITimedMessage.


///
/// A pure virtual class implementing a device that can manipulate a MIDI message, inspecting or changing
/// its content. Many objects, such as MIDIDriver, MIDIManager and MIDISequencer, allow you to insert a
/// MIDIProcessor into the flow of outgoing or ingoing messages.
///
class MIDIProcessor {
    public:
        /// The constructor.
                                        MIDIProcessor() {}
        /// The destructor.
        virtual                         ~MIDIProcessor() {}
        /// Resets the MIDIProcessor to its initial state.
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
        /// Empties the processors queue.
        virtual void                    Reset();
        /// Inserts a MIDIProcessor object into the queue.
        /// \param proc the MIDIProcessor to be inserted (it is NOT owned by the MIDIMultiProcessor)
        /// \param pos the position in the queue (no check is done). If you leave the default value the
        /// processor will be appended to the queue.
        void                            SetProcessor(MIDIProcessor* proc, int pos = -1);
        /// Returns a pointer to the MIDIProcessor at the given position.
        MIDIProcessor*                  GetProcessor(int pos)           { return processors[pos]; }
        /// Returns a pointer to the MIDIProcessor at the given position.
        const MIDIProcessor*            GetProcessor(int pos) const     { return processors[pos]; }

        /// Determines the behavior of the MIDIMultiProcessor if any of the processors return
        /// **false**. You can set these three values:
        /// - MODE_IGNORE :   the MIDIMultiProcessor ignores return values and always returns **true**
        /// - MODE_CONTINUE : the MIDIMultiProcessor performs all the processing, and returns **false**
        /// if any processor returned **false**, **true** otherwise (this the default when the object is created)
        /// - MODE_STOP :     the MIDIMultiProcessor stops processing when a processor returns **false**
        /// and returns **false**, **true** otherwise.
        void                            SetProcessMode(int mode)        { process_mode = mode; }
        /// Gets the processing mode.
        int                             GetProcessMode() const          { return process_mode; }
        /// Removes the MIDIProcessor at the given position. It only deletes the processor pointer from the queue.
        void                            RemoveProcessor(int pos);
        /// Searches the given MIDIProcessor in the queue and removes it (it does nothing
        /// if the processor isn't in the queue). It only removes the processor pointer from the queue.
        void                            RemoveProcessor(const MIDIProcessor* proc);
        /// The process method passes the MIDI message to the first processor and subsequently to
        /// all other processors in the queue. If any of the processors returns *false* the behaviour
        /// and the returned value are defined by the SetProcessMode() method.
        virtual bool                    Process(MIDITimedMessage *msg);
        /// These are the values to be given as parameters to the SetProcessMode() method.
        enum { MODE_IGNORE, MODE_CONTINUE, MODE_STOP };

    private:
        std::vector<MIDIProcessor*>     processors;
        int                             process_mode;
};

///
/// A MIDIProcessor which shifts the pitch of MIDI note and polyphonic pressure messages by a given amount of semitones.
/// You can set a different amount for each channel, or the same amount for all the channels.
///
class MIDIProcessorTransposer : public MIDIProcessor {
    public:
        /// The constructor.
                                        MIDIProcessorTransposer();
        /// Resets the default status (no transposing on all channels).
        virtual void                    Reset();
        /// Sets the transposing amount.
        /// \param chan the channel to be transposed
        /// \param trans The amount of transposing in semitones
        void                            SetTransposeChannel (int chan, int trans)   { trans_amount[chan] = trans; }
        /// Gets the transposing amount (in semitones) for the given channel
        int                             GetTransposeChannel (int chan) const        { return trans_amount[chan]; }
        /// Sets the same transposing amount (in semitones) for all the channels.
        void                            SetAllTranspose (int trans);
        /// The process() method. It affects only Note on, Note off and Poly pressure
        /// messages, changing their note number according to the transposing amount. If the resulting
        /// note number is not in the MIDI range (0 ... 127) the message is left unchanged and
        /// the method returns **false**, otherwise it returns **true**.
        virtual bool                    Process (MIDITimedMessage *msg);

    private:
        int                             trans_amount[16];
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
        /// Set the correspondence between two channels (will transform _src_chan_ into
        /// _dest_chan_). Channel range is 0 ... 15, however you can set _dest_chan_ to -1
        /// (see the Process() method).
        void                            SetRechanMap (int src_chan, int dest_chan)  { rechan_map[src_chan] = dest_chan; }
        /// Gets the corresponding channel for _src_chan_.
        int                             GetRechanMap (int src_chan) const           { return rechan_map[src_chan]; }
        /// Sends all channel messages to channel _dest_chan_.
        void                            SetAllRechan (int dest_chan);
        /// The Process() method. If _msg_ is not a channel message it is left unchanged
        /// and the function returns **true**. Otherwise its channel is changed according to the
        /// rechannel map. If its destination channel is -1 the msg remains unchanged but
        /// the function returns **false** (you can use this for filtering messages by channel)
        /// otherwise returns **true**.
        virtual bool                    Process(MIDITimedMessage *msg);

    private:
        int                             rechan_map[16];
};


///
/// A MIDIProcessor which prints a human-readable description of the processed messages to a std::ostream.
/// Useful for debugging purposes (you could want to see, for example, all messages
/// passing through a driver).
///
class MIDIProcessorPrinter : public MIDIProcessor {
    public:
        /// The constructor sets the std::ostream that will print the messages (default: std::cout).
                                        MIDIProcessorPrinter(std::ostream& stream = std::cout) :
                                                             print_on(true), ost(stream) {}
        /// Same of SetPrint(false)
        virtual void                    Reset()                                     { print_on = false; }
        /// Sets the printing on and off (default is on).
        void                            SetPrint(bool on_off)                       { print_on = on_off; }
        /// Gets the printing status.
        bool                            GetPrint() const                            { return print_on; }
        /// Processes the message, printing a human-readable description of it to the std::ostream given
        /// in the constructor. The message is left unchanged and always returns **true**.
        virtual bool                    Process(MIDITimedMessage *msg);

    private:
        bool                            print_on;
        std::ostream&                   ost;
};




#endif
