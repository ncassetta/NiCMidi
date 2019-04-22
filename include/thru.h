#ifndef THRU_H_INCLUDED
#define THRU_H_INCLUDED

#include "driver.h"
#include "tick.h"
#include <mutex>


/// A MIDITickComponent which immediately echoes to an out MIDI port all messages incoming
/// from an in MIDI port.
/// You can choose the in and out ports, select an unique channel for receiving and sending messages
/// (or leave them unchanged) and insert a MIDiProcessor between in and out ports for messages elaboration.
/// \note Remember that you must call the MIDIManager::AddTick() to make effective the StaticTickProc(), then
/// you can call Start() and Stop() methods to enable or disable the thru.
class MIDIThru : public MIDITickComponent {
    public:
                                                MIDIThru();
        virtual                                 ~MIDIThru() {}
        /// Resets the class to initial status:
        /// - In and out ports set to the OS id 0
        /// - No extra processor (warning: this only sets the processor pointer to 0! The driver
        ///   doesn't own its processor).
        /// - Thru input and output channel: all
        /// - Thru disabled;
        virtual void            Reset();

        /// Returns a pointer to the MIDIInDriver from which messages are actually being received.
        MIDIInDriver*           GetInPort() const               { return in_port; }
        /// Selects the hardware in port from which messages will be received.
        /// This can be done even if thru is already enabled.
        virtual void            SetInPort(MIDIInDriver* port);
        /// Returns a pointer to the MIDIOutDriver to whom messages are actually being sent.
        MIDIOutDriver*          GetOutPort() const              { return out_port; }
        /// Selects the hardware out port to whom messages will be sent.
        /// This can be done even if thru is already enabled.
        virtual void            SetOutPort(MIDIOutDriver* port);
        /// Gets the out processor.
        MIDIProcessor*          GetProcessor()                  { return processor; }
        const MIDIProcessor*    GetProcessor() const            { return processor; }
        /// Sets the out processor, which can manipulate messages arrived to the in port before they are sent
        /// to the out port (see MIDIProcessor).
        /// If you want to eliminate a processor already set, call it with 0 as parameter (this only sets the processor
        /// pointer to 0! The class doesn't own its processor).
        virtual void            SetProcessor(MIDIProcessor* proc);
        /// Returns the thru in channel (see SetInChannel())
        int                     GetInChannel() const            { return (int)in_channel; }
        /// Sets the channel for incoming thru messages.
        /// \param chan 0 ... 15: the thru will accept only messages with a specific channel; -1: the thru will
        /// accept all messages coming from the in port (this is the default). Non channel messages are always received.
        virtual void            SetInChannel(char chan);

        /// Returns the thru out channel (see SetOutChannel())
        int                     GetOutChannel() const            { return (int)out_channel; }
        /// Sets the channel for outgoing thru messages.
        /// \param chan 0 ... 15: the driver will redirect all messages to a specific channel; -1: the driver will leave
        /// channel messages unchanged (this is the default).
        virtual void            SetOutChannel(char chan);
        /// Starts the MIDI thru.
        virtual void            Start();
        /// Stops the MIDI thru.
        virtual void            Stop();





    protected:

        static void             StaticTickProc(tMsecs sys_time, void* pt);
        void                    TickProc(tMsecs sys_time);


        MIDIInDriver*           in_port;
        MIDIOutDriver*          out_port;
        char                    in_channel;
        char                    out_channel;

        MIDIProcessor*          processor;

    protected:
        void                    SilentOut();

};


#endif // THRU_H_INCLUDED
