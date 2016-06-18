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
//#include "tempo.h"
#include "sysex.h"

#include <string>



class 	MIDIMessage {
    public:
        ///@name The Constructors and Initializing methods
        //@{

        /// Creates a a MIDIMessage object which holds no value. The message will be ignored in playing.
                                MIDIMessage();
        /// The copy constructor. If the target message has a MIDISystemExclusive object it is duplicated,
        /// so every MIDIBigMessage has its own object.
                                MIDIMessage(const MIDIMessage &msg);
        /// Resets the message and frees the MIDISystemExclusive. The message becomes non valid and will
        /// be ignored.
        void	                Clear();
        /// Frees the MIDISystemExclusive without changing other bytes.
        void                    ClearSysEx();
        /// The equal operator. It primarily frees the old MIDISystemExclusive object if it was allocated,
        /// then duplicates the (eventual) new MIDISystemExclusive, so every MIDIBigMessage has its own object.
        const MIDIMessage&      operator= (const MIDIMessage &msg);

        //@}

        /// The destructor frees the eventual MIDISystemExclusive object
                                ~MIDIMessage();

        /// Create a human readable ascii string describing the message.  This is potentially unsafe as the 'txt'
        /// param must point to a buffer of at least 64 chars long.
        virtual char*	        MsgToText(char *txt) const;

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
        float	                GetTempo() const;
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
        /// GetController() and GetControllerValue() for further information.
        bool	                IsControlChange() const     { return ((status & 0xf0) == CONTROL_CHANGE); }
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
        /// Returns *true* if the message is a all notes off message (a control change with control >= 0x7A).  // TODO: revise this
        bool	                IsAllNotesOff() const       { return ((status & 0xf0) == CONTROL_CHANGE) &&
                                                                      (byte1 >= C_ALL_NOTES_OFF); }
        /// Returns *true* if the message is a system message (the status byte is 0xf0 or higher).
        bool	                IsSystemMessage() const     { return (status & 0xf0) == 0xf0; }
        /// Returns*true* if the message is a system exclusive message.
        bool	                IsSysEx() const             { return (status == SYSEX_START); }

        // now unused short	                GetSysExNum() const         { return (short)((byte3<<8) | byte2); }
        /// Returns true if the message is a midi time code message.
        bool	                IsMTC() const               { return (status == MTC); }
        /// Returns true if the message is a song position message.
        bool	                IsSongPosition() const      { return (status == SONG_POSITION); }
        /// Returns true if the message is a song select message.
        bool	                IsSongSelect() const        { return (status == SONG_SELECT); }
        /// Returns true if the message is a tune request message.
        bool 	                IsTuneRequest() const       { return (status == TUNE_REQUEST); }
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
        /// Allocates a MIDISystemExclusive object, with a buffer of given max size. The buffer
        /// is initially empty and can be accessed with GetSysEx(). An eventual old object is freed.
        void                    AllocateSysEx(unsigned int len);
        /// Copies the given MIDISystemExclusive object into the message without changing other bytes.
        /// An eventual old object is freed.
        void                    CopySysEx(const MIDISystemExclusive* se);
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

        void	                SetAllNotesOff(unsigned char chan, unsigned char type = C_ALL_NOTES_OFF);

        void	                SetLocal(unsigned char chan, unsigned char v);

        /// Makes the message a system exclusive message with given MIDISystemExclusive object. The eventual old
        /// sysex is freed and the MIDISystemExclusive is copied, so the message owns its object.
        void	                SetSysEx(const MIDISystemExclusive* se);
        /// Makes the message a MIDI time code message with given field (3 bits) and value (4 bits). Frees the sysex
        /// pointer.
        void	                SetMTC(unsigned char field, unsigned char val);
        /// Makes the message a song position system message with given position (14 bits). Frees the sysex pointer.
        void	                SetSongPosition( short pos );
        /// Makes the message a song select system message with given song. Frees the sysex pointer.
        void	                SetSongSelect(unsigned char sng);
        /// Makes the message a tune request system message. Frees the sysex pointer.
        void	                SetTuneRequest();
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
        void	                SetTempo(float tempo_bpm);
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
        friend bool             operator== (const MIDIMessage &m1, const MIDIMessage &m2);

    protected:

        unsigned char	        status;
        unsigned char	        byte1;
        unsigned char	        byte2;
        unsigned char	        byte3;		// byte 3 is only used for meta-events
        MIDISystemExclusive*    sysex;
};


/*********** NOW UNUSED
///
/// The MIDIMessage class is the base class which can hold a single MIDI Message consisting
/// of a status byte plus three data bytes, without timestamp nor SysEx. It has children classes
/// MIDIBigMessage, MIDITimedMessage and MIDITimedBigMessage.
///
class 	MIDIBigMessage {
    public:
        ///@name The Constructors and Initializing methods
        //@{

        /// Creates a a MIDIMessage object which holds no values.
                                MIDIBigMessage();
        /// The copy constructor. If the target message has a MIDISystemExclusive object it is duplicated,
        /// so every MIDIBigMessage has its own object.
                                MIDIBigMessage(const MIDIBigMessage &m);

        /// The destructor frees the eventual MIDISystemExclusive object
                                ~MIDIBigMessage();
        /// is a tempo change meta-message, returns the tempo value in 1/32 bpm
        unsigned short	        GetTempo32() const          { return GetMetaValue(); }

        /// These return a pointer to the MIDISystemExclusive object (0 if it is not allocated).
        MIDISystemExclusive*    GetSysEx()                  { return sysex; }
        const MIDISystemExclusive*GetSysEx() const          { return sysex; }



        /// Makes the message a polyphonic aftertouch message with given channel, note and pressure.
        void	                SetSysEx();

};

*/

/* ********************************************************************************************/
/*                     C L A S S   M I D I B i g M e s s a g e                                */
/* ********************************************************************************************/

/************ NOW UNUSED **********************************

///
/// The MIDIBigMessage inherits from MIDIMessage and adds the capability of storing
/// a dynamically allocated MIDISystemExclusive object. This is useful either for sysex messages
/// and for meta-event text events which needs to store text data. If it does not need to store a
/// sysex, the MIDISystemExclusive is not allocated.
///
class MIDIBigMessage : public MIDIMessage {
    public:
        ///@name Constructors/Assignment operators/Copiers
        //@{


        /// Creates a a MIDIBigMessage object which holds no values.
                                MIDIBigMessage() : sysex(0) {}
        /// The copy constructor. If the target message has a MIDISystemExclusive object it is duplicated,
        /// so every MIDIBigMessage has its own object.
                                MIDIBigMessage(const MIDIBigMessage &m);
        /// Copies only the MIDIMessage bytes. If the MIDIBigMessage has a MIDISystemExclusive, it is freed.
                                MIDIBigMessage(const MIDIMessage &m);

        /// The destructor frees the eventual MIDISystemExclusive object
                                ~MIDIBigMessage();
        /// Resets the message and frees the MIDISystemExclusive.
        void	                Clear();
        /// Frees the MIDISystemExclusive without changing other bytes.
        void                    ClearSysEx();
        /// The equal operator. It primarily frees the old MIDISystemExclusive object if it was allocated,
        /// then duplicates the (eventual) new MIDISystemExclusive, so every MIDIBigMessage has its own object.
        const MIDIBigMessage&   operator= (const MIDIBigMessage &m);
        /// Frees the old MIDISystemExclusive object if it was allocated and copies the MIDIMessage.
        const MIDIBigMessage&     operator = (const MIDIMessage &m);

      //
      // 'Get' methods
      //

        /// These return a pointer to the MIDISystemExclusive object (0 if it is not allocated).
        MIDISystemExclusive*      GetSysEx()              { return sysex; }
        const MIDISystemExclusive*GetSysEx() const        { return sysex; }


      //
      // 'Set' methods
      //

      void                      SetSysEx( MIDISystemExclusive *e );
        // this sets the sysex pointer to e (which bust be already allocated, and will be freed by the
        // dtor). It can be used also for meta messages which point to text. It doesn't set the status
        // byte to sysex: this is done by tehe SetSysEx() method


     //friend bool                operator== ( const MIDIBigMessage &m1, const MIDIBigMessage &m2 );


      protected:

      MIDISystemExclusive*      sysex;
};

*/

/* ********************************************************************************************/
/*                   C L A S S   M I D I T i m e d M e s s a g e                              */
/* ********************************************************************************************/


/******************** NOW UNUSED


class 	MIDITimedMessage : public MIDIMessage
    {
    public:
        ///@name Constructors/assignment operators/Copiers
        //@{


        /// The default constructor
                                MIDITimedMessage();
                                MIDITimedMessage( const MIDITimedMessage &m );
                                MIDITimedMessage( const MIDIMessage &m );

        void	                Clear();

      //
      // operator =
      //

        const MIDITimedMessage& operator = ( const MIDITimedMessage & m );
        const MIDITimedMessage& operator = ( const MIDIMessage & m );

      //
      // 'Get' methods
      //

        MIDIClockTime	        GetTime() const     { return time; }

      //
      // 'Set' methods
      //

        void	                SetTime( MIDIClockTime t )  { time = t; }

      //
      // Compare method for sorting. Not just comparing time.
      //

        static int 	            CompareEvents(
                                    const MIDITimedMessage &a,
                                    const MIDITimedMessage &b
                                );

        friend bool             operator== ( const MIDITimedMessage &m1, const MIDITimedMessage &m2 );

    protected:

        MIDIClockTime           time;
};

*/

/* ********************************************************************************************/
/*                   C L A S S   M I D I T i m e d M e s s a g e                              */
/* ********************************************************************************************/

///
/// The MIDITimedBigMessage inherits ftom a MIDIBigMessage and adds the features of a MIDITimedMessage.
/// I didn't use multiple inheritance to avoid increasing of memory usage.
/// This is the most used type of message, the one which is stored in MIDITrack class and is used for playing,
/// writing and loading MIDI files.
///
class 	MIDITimedMessage : public MIDIMessage {
    public:

      //
      // Constructors
      //

                                MIDITimedMessage();
                                MIDITimedMessage(const MIDITimedMessage &msg);
                                MIDITimedMessage(const MIDIMessage &msg);
      //
      // Destructor
      //

                                ~MIDITimedMessage();

        void	                Clear();

      //
      // operator =
      //

        const MIDITimedMessage &operator= (const MIDITimedMessage &msg);
        const MIDITimedMessage &operator= (const MIDIMessage &msg);

      //
      // 'Get' methods
      //

      MIDIClockTime	            GetTime() const             { return time; }

      //
      // 'Set' methods
      //

      void	                    SetTime(MIDIClockTime t)    { time = t; }
      void                      AddTime(MIDIClockTime t);
      void                      SubTime(MIDIClockTime t);

      //
      // Compare method, for sorting. Not just comparing time.
      //

      /* OLD FUNCTION, should be unused
      static int 	CompareEvents(
        const MIDITimedBigMessage &a,
        const MIDITimedBigMessage &b
        );
      */


    // This function is used by the MIDITrack::InsertEvent() and MIDITrack::InsertNote() methods for ordering
    // events when inserting. It compares events _a_ and _b_. _a_: the following tests are done in sequence:
    // if a (or b) is a NoOp it's larger
    // if a (or b) has lesser MIDI time it's smaller (sorts for increasing time)
    // if a (or b) is an EndOfTrack event it's larger
    // if a (or b) is a Meta event it is smaller (Meta go before channel messages)
    // if a (or b) is a SysEx it is larger (Sysex go after channel messages)
    // if a and b are both channel messages sort for ascending channel
    // if a (or b) is not a note message it's smaller (non note go before notes)
    // if a (or b) is a Note Off it's smaller (Note Off go before Note On)
    static int CompareEventsForInsert (
        const MIDITimedMessage &m1,
        const MIDITimedMessage &m2
    );

    // This function is used by the methods that search and insert events in the tracks to find events
    // that are of the same kind and would normally be incompatible.
    // It compares the events *a* and *b* and returns **true** if they have the same MIDI time and they are:
    // - both NoOp
    // - both Note On or Note Off with the same channel and the same note number
    // - both Control Change with the same channel and the same control number
    // - both channel messages (not notes or control) with the same channel and type
    // - both MetaEvent with the same meta type
    // - both non channel events (not Meta) with the same status
    // If the input mode is set to INSMODE_REPLACE or INSMODE_INSERT_OR_REPLACE the functions
    // MIDITrack::InsertEvent and MIDITrack::InsertNote search if at same MIDI time of the event to insert exists
    // such an event and, if they find it, they replace the event, rather than inserting it.
    static bool IsSameKind (
        const MIDITimedMessage &m1,
        const MIDITimedMessage &m2
    );

    //friend bool operator == ( const MIDITimedBigMessage &m1, const MIDITimedBigMessage &m2 );


    // replaced by operator==
    //static bool BitwiseEqual(const MIDITimedBigMessage& m1, const MIDITimedBigMessage& m2);

        virtual char*           MsgToText(char* txt) const;

    protected:

        MIDIClockTime	time;
};

#endif






/*
class MIDIMessage
{
public:


    /// If the message is a normal system exclusive marker, IsSysExN() will return true.
    /// \note Sysex messages are not stored in the MIDIMessage object. \see MIDIBigMessage
    bool IsSysExN() const   // Normal SysEx Event
    {
        return ( service_num == NOT_SERVICE) &&
               ( status == SYSEX_START_N );
    }


    bool IsSysExURT() const // Universal Real Time System Exclusive message, URT sysex
    {
        return IsSysExN() && ( byte1 == 0x7F );
    }


    int GetSysExURTdevID() const // return Device ID code for URT sysex
    {
        return byte2;
    }

    int GetSysExURTsubID() const // return Sub ID code for URT sysex
    {
        return byte3;
    }

    bool IsSysExA() const // Authorization SysEx Event
    {
        return ( service_num == NOT_SERVICE) &&
               ( status == SYSEX_START_A );
    }

    // TODO@VRM note to Jeff:
    // code with old fun IsSysEx() need to rewrite manually, because now it's two func: IsSysExN() and IsSysExA()
    bool IsSystemExclusive() const
    {
        return ( IsSysExN() || IsSysExA() );
    }





    /// If the message is a NoOp (not MIDI) message, IsNoOp() will return true.
    bool IsNoOp() const
    {
        return ( service_num == SERVICE_NO_OPERATION );
    }

    /// If the message is a Channel Prefix message, IsChannelPrefix() will return true.
    bool IsChannelPrefix() const
    {
        return ( service_num == NOT_SERVICE) &&
               ( status == META_EVENT ) &&
               ( byte1 == META_CHANNEL_PREFIX );
    }



    /// Returns the tempo value in 1/32 bpm
    unsigned long GetTempo32() const;

    // GetTempo() returns the original midifile tempo value (microseconds per beat)
    unsigned long GetTempo() const;







    void SetMetaType ( unsigned char t )
    {
        byte1 = t;
    }

    void SetMetaValue ( unsigned short v );

    void SetNoteOn ( unsigned char chan, unsigned char note, unsigned char vel );

    void SetNoteOff ( unsigned char chan, unsigned char note, unsigned char vel );

    void SetPolyPressure ( unsigned char chan, unsigned char note, unsigned char pres );

    void SetControlChange ( unsigned char chan, unsigned char ctrl, unsigned char val );

    // set panorama control in chan: pan = -1. for lefmost, 0. for centre, +1. for rightmost
    void SetPan( unsigned char chan, double pan );

    double GetPan();

    void SetProgramChange ( unsigned char chan, unsigned char val );

    void SetChannelPressure ( unsigned char chan, unsigned char val );

    void SetPitchBend ( unsigned char chan, short val );

    void SetPitchBend ( unsigned char chan, unsigned char low, unsigned char high );

    void SetSysEx( unsigned char type ); // type = SYSEX_START or SYSEX_START_A

    void SetMTC ( unsigned char field, unsigned char v );

    void SetSongPosition ( short pos );

    void SetSongSelect ( unsigned char sng );

    void SetTuneRequest();

    void SetMetaEvent ( unsigned char type, unsigned char v1, unsigned char v2 );

    void SetMetaEvent ( unsigned char type, unsigned short v );

    void SetAllNotesOff ( unsigned char chan, unsigned char type = C_ALL_NOTES_OFF, unsigned char mode = 0);

    void SetLocal ( unsigned char chan, unsigned char v );

    void SetNoOp()
    {
        Clear();
        service_num = SERVICE_NO_OPERATION;
    }

    void SetTempo ( unsigned long tempo ); // If no tempo is define, 120 beats per minute is assumed.

    void SetTempo32 ( unsigned long tempo_times_32 );

    void SetText ( unsigned short text_num, unsigned char type = META_GENERIC_TEXT );

    void SetDataEnd();
    void SetEndOfTrack()
    {
        SetDataEnd();
    }

    void SetTimeSig (
        unsigned char numerator = 4,
        unsigned char denominator_power = 2,
        unsigned char midi_clocks_per_metronome = 24,
        unsigned char num_32nd_per_midi_quarter_note = 8 );

    void SetKeySig ( signed char sharp_flats, unsigned char major_minor );

    void SetBeatMarker();


    friend bool operator == ( const MIDIMessage &m1, const MIDIMessage &m2 );

    //@}

protected:

    static const char * chan_msg_name[16]; ///< Simple ascii text strings describing each channel message type (0x8X to 0xeX)
    static const char * sys_msg_name[16]; ///< Simple ascii text strings describing each system message type (0xf0 to 0xff)
    static const char * service_msg_name[];

    unsigned int service_num; // if service_num != NOT_SERVICE than event used for internal service

    unsigned char status; // type of events and channel for Channel events, type of SysEx events
    unsigned char byte1; // type of Meta events, not used for SysExURT events
    unsigned char byte2; // Meta events or SysExURT events first data byte (#1)
    unsigned char byte3;
    unsigned char byte4;
    unsigned char byte5;
    unsigned char byte6; // Meta events or SysExURT events last data byte (#5)
    unsigned char data_length; // number of data bytes in Meta events or SysExURT events (0...5)
};
*/



/*
class MIDIBigMessage : public MIDIMessage
{
public:

    ///@name Constructors/assignment operators/Copiers
    //@{

    MIDIBigMessage();

    MIDIBigMessage ( const MIDIBigMessage &m );

    MIDIBigMessage ( const MIDIMessage &m );

    MIDIBigMessage ( const MIDIMessage &m, const MIDISystemExclusive *e );

    const MIDIBigMessage &operator = ( const MIDIBigMessage &m );

    const MIDIBigMessage &operator = ( const MIDIMessage &m );


    void Copy ( const MIDIBigMessage &m )
    {
        *this = m;
    }

    void Copy ( const MIDIMessage &m )
    {
        *this = m;
    }

    void CopySysEx ( const MIDISystemExclusive *e );

    //@}


    void Clear();

    void ClearSysEx();


    ///
    /// destructor
    ///

    ~MIDIBigMessage();

    void SetNoOp()
    {
        Clear();
        MIDIMessage::SetNoOp();
    }

    MIDISystemExclusive *GetSysEx()
    {
        return sysex;
    }


    const MIDISystemExclusive *GetSysEx() const
    {
        return sysex;
    }


    std::string GetSysExString() const
    {
        const unsigned char *buf = GetSysEx()->GetBuf();
        int len = GetSysEx()->GetLengthSE();
        std::string str;
        for (int i = 0; i < len; ++i)
            str.push_back( (char) buf[i] );
        return str;
    }

    friend bool operator == ( const MIDIBigMessage &m1, const MIDIBigMessage &m2 );

protected:

    MIDISystemExclusive *sysex;
};


///
/// The MIDITimedMessage inherits from a MIDIMessage and adds the capability of storing
/// a MIDI time clock.
///

class MIDITimedMessage : public MIDIMessage
{
public:

    //
    // Constructors
    //

    MIDITimedMessage();

    MIDITimedMessage ( const MIDITimedMessage &m );

    MIDITimedMessage ( const MIDIMessage &m );

    void Clear();

    void Copy ( const MIDITimedMessage &m )
    {
        *this = m;
    }

    //
    // operator =
    //

    const MIDITimedMessage &operator = ( const MIDITimedMessage & m );

    const MIDITimedMessage &operator = ( const MIDIMessage & m );

    //
    // 'Get' methods
    //

    /// Returns the MIDI time clock of the message.
    MIDIClockTime GetTime() const
    {
        return time;
    }

    //
    // 'Set' methods
    //

    /// Sets the MIDI time clock of the message.
    void SetTime ( MIDIClockTime t )
    {
        time = t;
    }


    //
    // Compare method for sorting. Not just comparing time.
    //

    static int  CompareEvents (
        const MIDITimedMessage &a,
        const MIDITimedMessage &b
    );

    friend bool operator == ( const MIDITimedMessage &m1, const MIDITimedMessage &m2 );

protected:


    MIDIClockTime time;
};




///
/// The MIDITimedBigMessage inherits ftom a MIDIBigMessage and adds the features of a MIDITimedMessage.
/// As said in MIDIMessage doc, Multiple Inheritance was not safe at the time the library was written,
/// so it isn't used here (maybe in the future).
/// This is the most used type of message, the one which is stored in MIDITrack class and is used for playing,
/// writing and loading MIDI files.
///

class MIDITimedBigMessage : public MIDIBigMessage
{
public:

    //
    // Constructors
    //

    MIDITimedBigMessage();

    MIDITimedBigMessage ( const MIDITimedBigMessage &m );

    MIDITimedBigMessage ( const MIDIBigMessage &m );

    MIDITimedBigMessage ( const MIDITimedMessage &m );

    MIDITimedBigMessage ( const MIDIMessage &m );

    MIDITimedBigMessage ( const MIDITimedMessage &m, const MIDISystemExclusive *e );

    void Clear();

    void Copy ( const MIDITimedBigMessage &m )
    {
        *this = m;
    }

    void Copy ( const MIDITimedMessage &m )
    {
        *this = m;
    }

    //
    // operator =
    //

    const MIDITimedBigMessage &operator = ( const MIDITimedBigMessage & m );

    const MIDITimedBigMessage &operator = ( const MIDITimedMessage & m );

    const MIDITimedBigMessage &operator = ( const MIDIMessage & m );

    //
    // 'Get' methods
    //

    MIDIClockTime GetTime() const
    {
        return time;
    }

    //
    // 'Set' methods
    //

    void SetTime ( MIDIClockTime t )
    {
        time = t;
    }

    //
    // Compare method, for sorting. Not just comparing time.
    //

    /// This is the older version of the function
    static int CompareEvents (
        const MIDITimedBigMessage &a,
        const MIDITimedBigMessage &b
    );

    /// This function is used by the MIDITrack::InsertEvent() and MIDITrack::InsertNote() methods for ordering
    /// events when inserting. It compares events _a_ and _b_. _a_: the following tests are done in sequence:
    /// + if _a_ (or _b_) is a NoOp it's larger
    /// + if _a_ (or _b_) has lesser MIDI time it's smaller (sorts for increasing time)
    /// + if _a_ (or _b_) is an EndOfTrack event it's larger
    /// + if _a_ (or _b_) is a Meta event it is smaller (Meta go before channel messages)
    /// + if _a_ (or _b_) is a SysEx it is larger (Sysex go after channel messages)
    /// + if _a_ and _b_ are both channel messages sort for ascending channel
    /// + if _a_ (or _b_) is not a note message it's smaller (non note go before notes)
    /// + if _a_ (or _b_) is a Note Off it's smaller (Note Off go before Note On)
    /// @returns **1** if _a_ < _b_ , **2** if _a_ > _b_, **0** if none of these (their order is indifferent)
    static int CompareEventsForInsert (
        const MIDITimedBigMessage &a,
        const MIDITimedBigMessage &b
    );

    /// This function is used by the methods that search and insert events in the tracks to find events
    /// that are of the same kind and would normally be incompatible.
    /// It compares the events *a* and *b* and returns **true** if they have the same MIDI time and they are:
    /// - both NoOp
    /// - both Note On or Note Off with the same channel and the same note number
    /// - both Control Change with the same channel and the same control nunber
    /// - both channel messages (not notes or control) with the same channel and type
    /// - both MetaEvent with the same meta type
    /// - both non channel events (not Meta) with the same status
    /// If the input mode is set to INSMODE_REPLACE or INSMODE_INSERT_OR_REPLACE the functions
    /// MIDITrack::InsertEvent and MIDITrack::InsertNote search if at same MIDI time of the event to insert exists
    /// such an event and, if they find it, they replace the event, rather than inserting it.
    static bool IsSameKind (
        const MIDITimedBigMessage &a,
        const MIDITimedBigMessage &b
    );

    friend bool operator == ( const MIDITimedBigMessage &m1, const MIDITimedBigMessage &m2 );

protected:
    MIDIClockTime time;
};

*/

