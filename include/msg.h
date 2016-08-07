/*
 *
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
**	Copyright 1986 to 1998 By J.D. Koftinoff Software, Ltd.
**
**	All rights reserved.
**
**	No one may duplicate this source code in any form for any reason
**	without the written permission given by J.D. Koftinoff Software, Ltd.
**
*/




#ifndef _JDKMIDI_MSG_H
#define _JDKMIDI_MSG_H

#include "midi.h"
#include "sysex.h"

#include <string>

///
/// This class stores data representing a MIDI message. It consists of a status byte, three data bytes for subsequent
/// data and a pointer to a MIDISystemExclusive object that can store larger amounts of data. Ordinary channel messages
/// only use the data bytes, while some meta messages and sysex also use the pointer. Lots of methods are provided for
/// setting and inspecting the message data without worrying about exadecimal values. See the file midi.h for a set of
/// enum values for MIDI messages data.
///
class 	MIDIMessage {
    public:
        ///@name The Constructors, Destructor and Initializing methods
        //@{

        /// Creates a a MIDIMessage object which holds no value. The message will be ignored in playing.
                                MIDIMessage();
        /// The copy constructor. If the target message has a MIDISystemExclusive object it is duplicated,
        /// so every MIDIBigMessage has its own object.
                                MIDIMessage(const MIDIMessage &msg);
        /// The destructor
        virtual                 ~MIDIMessage();
        /// Resets the message and frees the MIDISystemExclusive. The message becomes non valid.
        void	                Clear();
        /// Frees the MIDISystemExclusive without changing other bytes.
        void                    ClearSysEx();
        /// The equal operator. It primarily frees the old MIDISystemExclusive object if it was allocated,
        /// then duplicates the (eventual) new MIDISystemExclusive, so every MIDIBigMessage has its own object.
        const MIDIMessage&      operator= (const MIDIMessage &msg);

        //@}

        /// Create a human readable ascii string describing the message.  This is potentially unsafe as the 'txt'
        /// param must point to a buffer of at least 64 chars long.
        std::string	            MsgToText() const;

        ///@name The Query methods.
        //@{

        /// Returns the length in bytes of the entire message.
        char	                GetLength() const;
        /// Returns the status byte of the message.
        unsigned char	        GetStatus() const	        { return (unsigned char)status;	}
        /// If the message is a channel message, returns its MIDI channel (0...15).
        unsigned char	        GetChannel() const          { return (unsigned char)(status  &0x0f);	}
        /// If the message is a channel message, returns the relevant top 4 bits which describe
        /// what type of channel message it is.
        unsigned char	        GetType() const             { return (unsigned char)(status & 0xf0);	}
        /// If the message is a meta-message, returns the type byte.
        unsigned char	        GetMetaType() const	        { return byte1;	}
        /// Accesses the raw byte 1 of the message.
        unsigned char	        GetByte1() const	        { return byte1;	}
        /// Accesses the raw byte 2 of the message.
        unsigned char	        GetByte2() const	        { return byte2;	}
        /// Accesses the raw byte 3 of the message.
        unsigned char	        GetByte3() const	        { return byte3;	}
        /// These return a pointer to the MIDISystemExclusive object (0 if it is not allocated).
        MIDISystemExclusive*    GetSysEx()                  { return sysex; }
        const MIDISystemExclusive*GetSysEx() const          { return sysex; }
        /// If the message is a note on, note off, or poly aftertouch message, returns the note number.
        unsigned char	        GetNote() const		        { return byte1;	}
        /// If the message is a note on, note off, or poly aftertouch message, returns the velocity
        /// (or pressure).
        unsigned char	        GetVelocity() const	        { return byte2;	}
        /// If the message is a channel pressure message, returns the pressure value.
        unsigned char           GetChannelPressure() const  { return byte1; }
        /// If the message is a program change message, returns the program number.
        unsigned char	        GetProgramValue() const     { return byte1;	}
        /// If the message is a control change message, returns the controller number.
        unsigned char	        GetController() const	    { return byte1; }
        /// If the message is a control change message, returns the controller value.
        unsigned char	        GetControllerValue() const  { return byte2;	}
        /// If the message is a bender message, returns the signed 14 bit bender value.
        short	                GetBenderValue() const      { return (short)(((byte2 << 7) | byte1) - 8192); }
        /// If the message is a meta-message, returns the unsigned 14 bit value attached.
        unsigned short	        GetMetaValue()	const       { return (unsigned short)((byte3 << 8) | byte2); }
        /// If the message is a time signature meta-message, returns the numerator of the time signature.
        unsigned char           GetTimeSigNumerator() const { return byte2; }
        /// If the message is a time signature meta-message, returns the denominator of the time signature.
        unsigned char           GetTimeSigDenominator() const { return byte3; }
        /// If the message is a key signature meta-message, returns the standard midi file format of the key.
        /// Negative values means that many flats, positive numbers means that many sharps.
        signed char             GetKeySigSharpFlats() const { return (signed char)byte2; }
        /// If the message is a key signature meta-message, returns the standard midi file format of the key
        /// major/minor flag. 0 means a major key, 1 means a minor key.
        unsigned char           GetKeySigMajorMinor() const { return byte3; }
        /// If the message is a tempo change meta-message, returns the tempo value in bpm
        double	                GetTempo() const;
        /// If the message is a tempo change meta-message, returns the internal 3 bytes value stored in the message
        /// (the number of microseconds per quarter note).
        unsigned long           GetInternalTempo() const;
        /// If the message is a text meta-message, returns the associated text as a std::string
        std::string             GetText() const;

        /// Returns *true* if the message is some sort of channel message. You can then call
        /// GetChannel() for further information.
        bool	                IsChannelMsg() const        { return (status >= 0x80) && (status < 0xf0); }
        /// Returns *true* if the message is a note on message (status = NOTE_ON and velocity > 0).
        /// You can then call GetChannel(), GetNote() and GetVelocity() for further information.
        bool	                IsNoteOn() const            { return ((status & 0xf0) == NOTE_ON) && byte2; }
        /// Returns *true* if the message is a note off message (status = NOTE_OFF or status = NOTE_ON
        /// and velocity = 0). You can then call GetChannel(), GetNote() and GetVelocity() for
        /// further information.
        bool	                IsNoteOff() const           { return ((status & 0xf0) == NOTE_OFF) ||
                                                              (((status & 0xf0) == NOTE_ON) && byte2 == 0); }
        /// Returns *true* if the message is a note on or a note off message.
        bool                    IsNote() const              { return IsNoteOn() || IsNoteOff(); }
        /// Returns *true* if the message is a polyphonic pressure channel message. You can then
        /// call GetChannel(), GetNote() and GetVelocity() for further information.
        bool	                IsPolyPressure() const      { return ((status & 0xf0) == POLY_PRESSURE); }
        /// Returns *true* if the message is a control change message. You can then call GetChannel(),
        /// GetController() and GetControllerValue() for further information. \note This will return *false*
        /// on channel mode messages (which have the same status byte of control changes).
        bool	                IsControlChange() const     { return ((status & 0xf0) == CONTROL_CHANGE) &&
                                                                        (byte1 < C_ALL_SOUND_OFF); }
        /// Returns *true* if the message is a volume change message (control = 0x07). You can then
        /// call GetChannel() and GetControllerValue() for further information.
        bool                    IsVolumeChange() const      { return IsControlChange() && GetController() == C_MAIN_VOLUME; }
        /// Returns *true* if the message is a pan change message (control = 0x0A). You can
        /// then call GetChannel() and GetControllerValue() for further information.
        bool                    IsPanChange() const         { return IsControlChange() && GetController() == C_PAN; }
        /// Returns *true* if the message is a pedal on message (control = 0x40 and value >= 0x64). You can
        /// then call GetChannel() for further information.
        bool                    IsPedalOn() const           { return IsControlChange() && GetController() == C_DAMPER &&
                                                                                          GetControllerValue() & 0x40; }
        /// Returns *true* if the message is a pedal off message (control = 0x40 and value < 0x64). You can
        /// then call GetChannel() for further information.
        bool                    IsPedalOff() const          { return IsControlChange() && GetController() == C_DAMPER &&
                                                                                          !(GetControllerValue() & 0x40); }
        /// Returns *true if the message is a program change message.  You can then call GetChannel()
        /// and GetProgramValue() for further information.
        bool	                IsProgramChange() const     { return ((status & 0xf0) == PROGRAM_CHANGE); }
        /// Returns *true* if the message is a channel pressure message. You can then call GetChannel()
        /// and GetChannelPressure() for further information.
        bool	                IsChannelPressure() const   { return ((status & 0xf0) == CHANNEL_PRESSURE); }
        /// Returns *true* if the message is a bender message. You can then call GetChannel()
        /// and GetBenderValue() for further information.
        bool	                IsPitchBend() const         { return ((status & 0xf0) == PITCH_BEND); }
        /// Returns *true* if the message is a channel mode message.
        bool                    IsChannelMode() const       { return ((status & 0xf0) == CONTROL_CHANGE) &&
                                                                      (byte1 >= C_ALL_SOUND_OFF); }
        /// Returns *true* if the message is a all notes off message (a control change with control >= 0x7A).  // TODO: revise this
        bool	                IsAllNotesOff() const       { return ((status & 0xf0) == CONTROL_CHANGE) &&
                                                                      (byte1 == C_ALL_NOTES_OFF); }
        /// Returns *true* if the message is a system message (the status byte is 0xf0 or higher).
        bool	                IsSystemMessage() const     { return (status & 0xf0) == 0xf0; }
        /// Returns*true* if the message is a system exclusive message.
        bool	                IsSysEx() const             { return (status == SYSEX_START); }
        /// Returns *true* if the message is a meta event message. You can then call GetMetaType()
        /// and GetMetaValue() for further information.
        bool	                IsMetaEvent() const         { return (status == META_EVENT); }
        /// Returns *true* if the message is a text message (a subset of meta events). You can then
        /// call GetText() for further information.
        bool 	                IsTextEvent() const         { return (status == META_EVENT && byte1 >= 0x1 && byte1 <= 0xf); }
        /// Returns *true* if the message is a track name meta-message. You can then
        /// call GetText() for further information.
        bool                    IsTrackName() const         { return (IsTextEvent() && GetMetaType() == META_TRACK_NAME); }
        /// Returns *true* if the message is a marker text meta-message. You can then
        /// call GetText() for further information.
        bool                    IsMarkerText() const        { return (IsTextEvent() && GetMetaType() == META_MARKER_TEXT); }
        /// Returns *true* if the message is a tempo change meta-message. You can then call GetTempo32()
        /// for further information.
        bool	                IsTempo() const             { return (status == META_EVENT) && (byte1 == META_TEMPO); }
        /// Returns *true* if the message is a data end (i.e. end of track) meta-message.
        bool	                IsDataEnd() const           { return (status == META_EVENT) && (byte1 == META_END_OF_TRACK); }
        /// Returns *true* if the message is a time Signature meta-message. You can then call
        /// GetTimeSigNumerator() and GetTimeSigDenominator() for further information.
        bool	                IsTimeSig() const           { return (status == META_EVENT) && (byte1 == META_TIMESIG); }
        /// Returns *true* if the message is a key signature meta-message. You can then call
        /// GetKeySigSharpFlats() and GetKeySigMajorMinor() for further information.
        bool	                IsKeySig() const            { return (status == META_EVENT) && (byte1 == META_KEYSIG); }
        /// Returns *true* if the message is a beat marker message (this is a dummy meta-message used by the MIDISequencer
        /// class to mark the metronome clicks).
        bool 	                IsBeatMarker() const        { return (status == META_EVENT) && (byte1 == META_BEAT_MARKER); }

        /// Returns *true* if the status is not a valid MIDI status.
        bool	                IsNoOp() const              { return (status & 0xF0) == 0; }
        //@}

        ///@name The 'Set' methods
        //@{


        /// Sets all bits of the status byte of the message (i.e., for channel messages, the type and the
        /// channel).
        void	                SetStatus(unsigned char s)	        { status = s; }
        /// Sets just the lower 4 bits of the status byte without changing the upper 4 bits.
        void	                SetChannel(unsigned char s)         { status = (unsigned char)((status & 0xf0) | s); }
        /// Sets just the upper 4 bits of the status byte without changing the lower 4 bits.
        void	                SetType(unsigned char s)            { status = (unsigned char)((status & 0x0f) | s); }
        /// Sets the raw value of the data byte 1.
        void	                SetByte1(unsigned char b)		    { byte1 = b; }
        /// Sets the raw value of the data byte 2.
        void	                SetByte2(unsigned char b)		    { byte2 = b; }
        /// Sets the raw value of the data byte 3.
        void	                SetByte3(unsigned char b)		    { byte3 = b; }
        /// Sets the note number for note on, note off, and polyphonic aftertouch messages.
        void	                SetNote(unsigned char n) 		    { byte1 = n; }
        /// Sets the velocity for note on, note off and polyphonic aftertouch messages.
        void	                SetVelocity(unsigned char v) 	    { byte2 = v; }
        /// Sets the controller number for a control change message.
        void	                SetController(unsigned char c) 	    { byte1 = c; }
        /// Sets the controller value for a control change message.
        void	                SetControllerValue(unsigned char v) { byte2 = v; }
        /// Sets the program number for a program change message.
        void	                SetProgramValue(unsigned char v)    { byte1 = v; }
        /// Sets the bender value (a signed 14 bit value) for a bender message.
        void	                SetBenderValue(short v);
        /// Sets the meta message type for a meta-event message.
        void	                SetMetaType(unsigned char t)        { byte1 = t; }
        /// Sets the meta value for a meta-event message.
        void	                SetMetaValue(unsigned short v);
        /// Makes the message a note on message with given channel, note and velocity. Frees the sysex pointer.
        void	                SetNoteOn(unsigned char chan, unsigned char note, unsigned char vel);
        /// Makes the message a note off message with given channel, note and velocity. Frees the sysex pointer.
        void	                SetNoteOff(unsigned char chan, unsigned char note, unsigned char vel);
        /// Makes the message a polyphonic aftertouch message with given channel, note and pressure.
        /// Frees the sysex pointer.
        void	                SetPolyPressure(unsigned char chan, unsigned char note, unsigned char pres);
        /// Makes the message a control change message with given channel, controller and value.
        /// Frees the sysex pointer.
        void	                SetControlChange(unsigned char chan, unsigned char ctrl, unsigned char val);
        /// Makes the message a volume change (control = 0x07) message with given channel and value.
        /// Frees the sysex pointer.
        void                    SetVolumeChange (unsigned char chan, unsigned char val)
                                            { SetControlChange( chan, C_MAIN_VOLUME, val); }
        /// Makes the message a pan change (control = 0x0A) message with given channel and value.
        /// Frees the sysex pointer.
        void                    SetPanChange (unsigned char chan, unsigned char val)
                                            { SetControlChange( chan, C_PAN, val); }
        /// Makes the message a program change message with given channel and program. Frees the sysex pointer.
        void	                SetProgramChange(unsigned char chan, unsigned char prog);
        /// Makes the message a channel pressure message with given channel and pressure. Frees the sysex pointer.
        void	                SetChannelPressure(unsigned char chan, unsigned char pres);
        /// Makes the message a pitch bend message with given channel and value (unsigned 14 bit).
        /// Frees the sysex pointer.
        void	                SetPitchBend( unsigned char chan, short val );
        /// Makes the message a channel mode message (i.e. a control change with a specific control number.
        /// Channel mode messages include:
        /// + All sound off         (type = C_ALL_SOUND_OFF)
        /// + Reset all controllers (type = C_RESET)
        /// + Local control         (type = C_LOCAL, value = 0x7f/0 for on/off)
        /// + All notes off         (type = C_ALL_NOTES_OFF, \see SetAllNotesOff())
        /// + Omni mode off         (type = C_OMNI_OFF)
        /// + Omni mode on          (type = C_OMNI_ON)
        /// + Mono mode on          (type = C_MONO, value = number of channels to respond, 0 for all channels)
        /// + Poly mode on          (type = C_POLY)
        void	                SetChannelMode(unsigned char chan, unsigned char type, unsigned char val = 0)
                                                    { SetControlChange(chan, type, val); }
        /// Makes the message a all notes off message with given channel
        void	                SetAllNotesOff(unsigned char chan)
                                                    { SetControlChange(chan, C_ALL_NOTES_OFF, 0); }
        /// Makes the message a system exclusive message with given MIDISystemExclusive object. The eventual old
        /// sysex is freed and the MIDISystemExclusive is copied, so the message owns its object.
        void	                SetSysEx(const MIDISystemExclusive* se);
        /// Makes the message a MIDI time code message with given field (3 bits) and value (4 bits). Frees the sysex
        /// pointer.
        void	                SetMTC(unsigned char field, unsigned char val);
        /// Makes the message a song position system message with given position (14 bits). Frees the sysex pointer.
        void	                SetSongPosition(short pos);
        /// Makes the message a song select system message with given song. Frees the sysex pointer.
        void	                SetSongSelect(unsigned char sng);
        /// Makes the message a one-byte system message with given status (type must be a valid MIDI system message
        /// status byte. Sets other bytes to 0 and frees the sysex pointer. This is useful mainly for messages which
        /// haven't data bytes, for others there are more appropriate methods.
        void	                SetSystemMessage(unsigned char type);
        /// Makes the message a meta-message with given type and data value. The two bytes of data are
        /// given separately.
        void	                SetMetaEvent(unsigned char type, unsigned char v1, unsigned char v2);
        /// Makes the message a meta-message with given type and data value. You can use this for
        /// Sequence number (16 bit value) and Channel Prefix (8 bit value) messages.
        void	                SetMetaEvent(unsigned char type, unsigned short val);
        /// Makes the message a text meta-message with given type; text is a C string containing the ascii
        /// characters and is stored in the sysex object (the eventual old pointer is freed). You can use
        /// this for Generic text, Copyright, Track name, Instrument name, Lyric, Marker and Cue point
        /// messages.
        void	                SetText(const char* text, unsigned char type = META_GENERIC_TEXT);
        /// Makes the message a data end (i.e. end of track) meta-message.
        void	                SetDataEnd()                        { SetMetaEvent(META_END_OF_TRACK, 0); }
        /// Makes the message a tempo change meta-message with given tempo (in bpm). The tempo is stored in the
        /// sysex object as a 3 byte value according to the MIDIfile format. The eventual old pointer is freed.
        void	                SetTempo(double tempo_bpm);
        /// Makes the message a SMPTE offset meta-message with given data. The bytes are stored in the
        /// sysex object.(the eventual old pointer is freed).
        void                    SetSMPTEOffset(unsigned char hour, unsigned char min, unsigned char sec,
                                               unsigned char frame,unsigned char subframe);
        /// Makes the message a time signature meta-message with given time (numerator and denominator).
        void	                SetTimeSig(unsigned char num, unsigned char den,
                                           unsigned char clocks_per_metronome = 0, unsigned char num_32_per_quarter = 8);
        /// Makes the message a key signature meta-message with given accidents and mode \see GetKeySigSharpFlats(),
        /// GetKeySigMajorMinor().
        void                    SetKeySig( signed char sharp_flats, unsigned char major_minor )
                                                                    { SetMetaEvent(META_KEYSIG, sharp_flats, major_minor); }
        /// Makes the message a beat marker meta-message.
        /// \see IsBeatMarker().
        void	                SetBeatMarker()                     { SetMetaEvent(META_BEAT_MARKER, 0); }

        /// The same of Clear(), makes the message a non valid message which will be ignored
        void	                SetNoOp()                           { Clear(); }
        /// Allocates a MIDISystemExclusive object, with a buffer of given max size. The buffer
        /// is initially empty and can be accessed with GetSysEx(). An eventual old object is freed.
        void                    AllocateSysEx(unsigned int len);
        /// Copies the given MIDISystemExclusive object into the message without changing other bytes.
        /// An eventual old object is freed.
        void                    CopySysEx(const MIDISystemExclusive* se);
        /// The compare operator
        friend bool             operator== (const MIDIMessage &m1, const MIDIMessage &m2);

    protected:

        unsigned char	        status;     ///< The status byte.
        unsigned char	        byte1;      ///< 1st data byte.
        unsigned char	        byte2;      ///< 2nd data byte.
        unsigned char	        byte3;		///< 3rd data byte (only used for some meta-events).
        MIDISystemExclusive*    sysex;      ///< The sysex pointer.
};

// todo: Add a Reset message? (status = 0xff)

/* ********************************************************************************************/
/*                   C L A S S   M I D I T i m e d M e s s a g e                              */
/* ********************************************************************************************/

typedef unsigned long MIDIClockTime;
const MIDIClockTime TIME_INFINITE = 0xffffffff;


///
/// The MIDITimedMessage class inherits from the MIDIMessage and represents a message associated with a
/// specific MIDIClockTime (i.e. the number of MIDI ticks from the start. The MIDITrack class stores a
/// vector of ordered MIDITimedMessage items, and they are used for playing, writing and loading MIDI files.
///
class 	MIDITimedMessage : public MIDIMessage {
    public:

      ///@name The Constructors, Destructor and Initializing methods
        //@{

        /// Creates a a MIDIMessage object which holds no value. The message will be ignored in playing.
                                MIDITimedMessage();
        /// Copy constructors. \see MIDIMessage::(MIDIMessage()
                                MIDITimedMessage(const MIDITimedMessage &msg);
                                MIDITimedMessage(const MIDIMessage &msg);
        /// Destructor
                                ~MIDITimedMessage();
        /// Resets the message and frees the MIDISystemExclusive. The message becomes non valid.
        void	                Clear();

        void                    SetNoOp()                       { Clear(); }

        /// The equal operators \see MIDIMessage::operator=()
        const MIDITimedMessage &operator= (const MIDITimedMessage &msg);
        const MIDITimedMessage &operator= (const MIDIMessage &msg);

        /// Returns the MIDIClockTime associated with the event.
        MIDIClockTime	        GetTime() const                 { return time; }

        /// Sets the MIDIClockTime associated with the event.
        void	                SetTime(MIDIClockTime t)        { time = t; }
        /// Adds the given amount to the associated time.
        void                    AddTime(MIDIClockTime t)        { time += t; }
        /// Subtracts the given amount from the associated time (if _t_ is greater set the time to 0).
        void                    SubTime(MIDIClockTime t)        { time = (t > time ? 0 : time - t); }

        /// Inherited from MIDIMessage.
        std::string             MsgToText() const;

        /// This function is used by the MIDITrack::InsertEvent() and MIDITrack::InsertNote() methods for ordering
        /// events when inserting. It compares events _m1_ and _m2_: the following tests are done in sequence:
        /// - if m1 (or m2) is a NoOp it's larger
        /// - if m1 (or m2) has lesser MIDI time it's smaller (sorts for increasing time)
        /// - if m1 (or m2) is an EndOfTrack event it's larger
        /// - if m1 (or m2) is a Meta event it is smaller (Meta go before channel messages)
        /// - if m1 (or m2) is a SysEx it is larger (Sysex go after channel messages)     // TODO: is this correct?
        /// - if m1 and m2 are both channel messages sort for ascending channel
        /// - if m1 (or m2) is not a note message it's smaller (non note go before notes)
        /// - if m1 (or m2) is a Note Off it's smaller (Note Off go before Note On)
        /// \returns **1** if _a_ < _b_ , **2** if _a_ > _b_, **0** if none of these (their order is indifferent)
        static int              CompareEventsForInsert (const MIDITimedMessage &m1, const MIDITimedMessage &m2);

        /// This function is used by the methods that search and insert events in the tracks to find events
        /// that are of the same kind and would normally be incompatible.
        /// It compares the events _m1_ and _m2_ and returns **true** if they have the same MIDI time and they are:
        /// - both NoOp
        /// - both Note On or Note Off with the same channel and the same note number
        /// - both Control Change with the same channel and the same control number
        /// - both channel messages (not notes or control) with the same channel and type
        /// - both MetaEvent with the same meta type
        /// - both non channel events (not Meta) with the same status
        /// If the input mode is set to INSMODE_REPLACE or INSMODE_INSERT_OR_REPLACE the functions
        /// MIDITrack::InsertEvent and MIDITrack::InsertNote search if at same MIDI time of the event to insert exists
        /// such an event and, if they find it, they replace the event, rather than inserting it.
        static bool             IsSameKind (const MIDITimedMessage &m1, const MIDITimedMessage &m2);

    protected:

        MIDIClockTime	        time;
};

#endif
