#ifndef THRU_H_INCLUDED
#define THRU_H_INCLUDED

#include "driver.h"
#include "tick.h"

class MIDIThru : public MIDITICK {
    public:
                                                MIDIThru();
        virtual                                 ~MIDIThru() {}
        /// Resets the class to initial status:
        /// - In and out ports undefined
        /// - No extra processor (warning: this only sets the processor pointer to 0! The driver
        ///   doesn't own its processor).
        /// - Thru output channel: all
        /// - Thru disabled;
        virtual void            Reset();

        /// Opens the hardware out port denoted by the id number given in the ctor. This usually
        /// requires a noticeable amount of time, so it's better not to immediately start to send
        /// messages. If the port is already open the object remembers how many times it was
        /// open, so a corresponding number of ClosePort() must be called to effectively close the port.
        MIDIInDriver*           GetInPort() const               { return in_port; }
        /// Closes the hardware out port, or decrements the count (leaving it open) if it was open
        /// more than once. The function does nothing if the port is already close. If you want to force
        /// the closure call Reset().
        virtual void            SetInPort(MIDIInDriver* port);

        MIDIOutDriver*          GetOutPort() const              { return out_port; }

        virtual void            SetOutPort(MIDIOutDriver* port);


        /// Gets the out processor.
        MIDIProcessor*          GetProcessor()                  { return processor; }
        const MIDIProcessor*    GetProcessor() const            { return processor; }
        /// Sets the out processor, which can manipulate all outgoing messages (see MIDIProcessor). If you
        /// want to eliminate a processor already set, call it with 0 as parameter (this only sets the processor
        /// pointer to 0! The driver doesn't own its processor).
        virtual void            SetProcessor(MIDIProcessor* proc);
        /// Returns the thru channel (see SetThruChannel())
        int                     GetInChannel() const            { return (int)in_channel; }

        /// Sets the channel for outgoing thru messages.
        /// \param chan 0 ... 15: the driver will redirect messages to a specific channel; -1: the driver will leave
        /// channel messages unchanged (this is the default).
        /// \note for MIDI thru you need to join a MIDIInDriver (which catches incoming messages from a in port)
        /// with a MIDIOutDriver (which sends them to a out port). You can do this with driver methods,
        /// but it's better to use the MIDIManager class which has specific methods.
        virtual void            SetInChannel(char chan);

        /// Returns the thru channel (see SetThruChannel())
        int                     GetOutChannel() const            { return (int)out_channel; }

        /// Sets the channel for outgoing thru messages.
        /// \param chan 0 ... 15: the driver will redirect messages to a specific channel; -1: the driver will leave
        /// channel messages unchanged (this is the default).
        /// \note for MIDI thru you need to join a MIDIInDriver (which catches incoming messages from a in port)
        /// with a MIDIOutDriver (which sends them to a out port). You can do this with driver methods,
        /// but it's better to use the MIDIManager class which has specific methods.
        virtual void            SetOutChannel(char chan);


        virtual void            SetAll(MIDIInDriver* in_p, MIDIOutDriver* out_p, char in_c, char out_c);

        /// Sets the MIDI thru enable on and off. For effective MIDI thru you must have already
    /// set in and out thru ports (with SetThruPorts()) otherwise the method will fail and return
    /// *false*.
    bool                        SetEnable(bool f);
    /// Returns the MIDI thru enable status.
    bool                        GetEnable() const                       { return enable; }


    protected:

        void                    TickProc(tMsecs sys_time);


        MIDIInDriver*           in_port;
        MIDIOutDriver*          out_port;

        MIDIProcessor*          processor;

        char                    in_channel;
        char                    out_channel;

        bool                    enable;

    private:

        void                    SilentOut();


};


#endif // THRU_H_INCLUDED
