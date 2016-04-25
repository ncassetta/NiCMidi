/*
 *
 *  SLIGHTLY MODIFIED BY NICOLA CASSETTA
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

// LASCIATO IL VECCHIO FILE: MESSAGGI DI 4 BYTE


#ifndef _JDKMIDI_MSG_H
#define _JDKMIDI_MSG_H

#include "midi.h"
#include "tempo.h"
#include "sysex.h"


class 	MIDIMessage {
    public:

      //
      // The Constructors
      //

                                MIDIMessage();
                                MIDIMessage( const MIDIMessage &m );

        void	                Clear();
        void	                Copy( const MIDIMessage & m );  // TODO: DELETE THIS!

        char *	                MsgToText( char *txt ) const;

      //
      // The equal operator
      //

        const MIDIMessage&      operator= ( const MIDIMessage &m );

      //
      // The Query methods.
      //

        char	                GetLength() const;
        unsigned char	        GetStatus() const	        { return (unsigned char)status;	}
        unsigned char	        GetChannel() const          { return (unsigned char)(status&0x0f);	}
        unsigned char	        GetType() const             { return (unsigned char)(status&0xf0);	}
        unsigned char	        GetMetaType() const	        { return byte1;	}
        unsigned char	        GetByte1() const	        { return byte1;	}
        unsigned char	        GetByte2() const	        { return byte2;	}
        unsigned char	        GetByte3() const	        { return byte3;	}

        unsigned char	        GetNote() const		        { return byte1;	}
        unsigned char	        GetVelocity() const	        { return byte2;	}
        unsigned char           GetChannelPressure() const  { return byte1; }
        unsigned char	        GetPGValue() const	        { return byte1;	}
        unsigned char	        GetController() const	    { return byte1; }
        unsigned char	        GetControllerValue() const  { return byte2;	}
        short	                GetBenderValue() const
                                        { return (short)(((byte2<<7) | byte1)-8192); }
        unsigned short	        GetMetaValue()	const
                                        { return (unsigned short)((byte3<<8) | byte2); }
        unsigned char           GetTimeSigNumerator() const { return byte2; }
        unsigned char           GetTimeSigDenominator() const { return byte3; }
        signed char             GetKeySigSharpFlats() const { return (signed char)byte2; }
        unsigned char           GetKeySigMajorMinor() const { return byte3; }

        // GetTempo() returns the tempo value in 1/32 bpm
        unsigned short	        GetTempo32() const              { return GetMetaValue(); }
        unsigned short	        GetLoopNumber() const           { return GetMetaValue(); }


        bool	                IsChannelMsg() const
                                        { return (status>=0x80) && (status<0xf0); }
        bool	                IsNoteOn() const
                                        { return ((status&0xf0)==NOTE_ON) && byte2; }
        bool	                IsNoteOff() const
                                        { return ((status&0xf0)==NOTE_OFF) ||
                                          (((status&0xf0)==NOTE_ON) && byte2==0 ); }
        bool                    IsNote() const
                                        { return IsNoteOn() || IsNoteOff(); }
        bool	                IsPolyPressure() const
                                        { return ((status&0xf0)==POLY_PRESSURE ); }
        bool	                IsControlChange() const
                                        { return ((status&0xf0)==CONTROL_CHANGE); }
        bool	                IsProgramChange() const
                                        { return ((status&0xf0)==PROGRAM_CHANGE); }
        bool                    IsVolumeChange() const
                                        { return IsControlChange() && GetController() == C_MAIN_VOLUME; }
        bool                    IsPedalOn() const
                                        { return IsControlChange() && GetController() == C_DAMPER &&
                                                 GetControllerValue() & 0x40; }
        bool                    IsPedalOff() const
                                        { return IsControlChange() && GetController() == C_DAMPER &&
                                                 !( GetControllerValue() & 0x40 ); }
        bool                    IsPanChange() const
                                        { return IsControlChange() && GetController() == C_PAN; }
        bool	                IsChannelPressure() const
                                        { return ((status&0xf0)==CHANNEL_PRESSURE); }
        bool	                IsPitchBend() const
                                        { return ((status&0xf0)==PITCH_BEND); }
        bool	                IsSystemMessage() const
                                        { return (status&0xf0)==0xf0; }
        bool	                IsSysEx() const
                                        { return (status==SYSEX_START); }
        short	                GetSysExNum() const
                                        { return (short)((byte3<<8) | byte2); }
        bool	                IsMTC() const               { return (status==MTC); }
        bool	                IsSongPosition() const      { return (status==SONG_POSITION); }
        bool	                IsSongSelect() const        { return (status==SONG_SELECT); }
        bool 	                IsTuneRequest() const       { return (status==TUNE_REQUEST); }
        bool	                IsMetaEvent() const         { return (status==META_EVENT); }
        bool 	                IsTextEvent() const
                                        { return (status==META_EVENT) && (byte1>=0x1 && byte1<=0xf); }
        bool	                IsAllNotesOff() const
                                        { return ((status&0xf0)==CONTROL_CHANGE ) &&
                                          (byte1>=C_ALL_NOTES_OFF); }
        bool	                IsNoOp() const
                                        { return (status==META_EVENT) &&
                                          (byte1==META_NO_OPERATION); }
        bool	                IsTempo() const
                                        { return (status==META_EVENT) &&
                                          (byte1==META_TEMPO); }
        bool	                IsDataEnd() const
                                        { return (status==META_EVENT) &&
                                          (byte1==META_DATA_END ); }
        bool	                IsTimeSig() const
                                        { return (status==META_EVENT) &&
                                          (byte1==META_TIMESIG ); }
        bool	                IsKeySig() const
                                        { return (status==META_EVENT) &&
                                          (byte1==META_KEYSIG ); }
        bool 	                IsBeatMarker() const
                                        { return (status==META_EVENT) &&
                                          (byte1==META_BEAT_MARKER); }
        bool 	                IsTextMarker() const
                                        { return (status==META_EVENT) &&
                                          (byte1==META_MARKER_TEXT); }

      //
      // The 'Set' methods
      //

        void	                SetStatus( unsigned char s )	{ status=s; }
        void	                SetChannel( unsigned char s )
                                        {status=(unsigned char)((status&0xf0)|s);}
        void	                SetType( unsigned char s )
                                        {status=(unsigned char)((status&0x0f)|s);}
        void	                SetByte1( unsigned char b )		{ byte1=b; }
        void	                SetByte2( unsigned char b )		{ byte2=b; }
        void	                SetByte3( unsigned char b )		{ byte3=b; }
        void	                SetNote( unsigned char n ) 		{ byte1=n; }
        void	                SetVelocity(unsigned char v) 	{ byte2=v; }
        void	                SetProgramValue(unsigned char v){ byte1=v; }
        void	                SetController(unsigned char c) 	{ byte1=c; }
        void	                SetControllerValue(unsigned char v ) { byte2=v; }
        void	                SetBenderValue( short v);
        void	                SetMetaType( unsigned char t )  { byte1 = t; }
        void	                SetMetaValue( unsigned short v );
        void	                SetNoteOn( unsigned char chan, unsigned char note, unsigned char vel );
        void	                SetNoteOff( unsigned char chan, unsigned char note, unsigned char vel );
        void	                SetPolyPressure( unsigned char chan, unsigned char note, unsigned char pres );
        void	                SetControlChange( unsigned char chan, unsigned char ctrl, unsigned char val );
        void                    SetVolumeChange ( unsigned char chan, unsigned char val )
                                            { SetControlChange( chan, C_MAIN_VOLUME, val); }
        void                    SetPanChange ( unsigned char chan, unsigned char val )
                                            { SetControlChange( chan, C_PAN, val); }
        void	                SetProgramChange( unsigned char chan, unsigned char val );
        void	                SetChannelPressure( unsigned char chan, unsigned char val );
        void	                SetPitchBend( unsigned char chan, short val );
        void	                SetPitchBend( unsigned char chan, unsigned char low, unsigned char high );
        void	                SetSysEx();
        void	                SetMTC( unsigned char field, unsigned char v );
        void	                SetSongPosition( short pos );
        void	                SetSongSelect(unsigned char sng);
        void	                SetTuneRequest();
        void	                SetMetaEvent( unsigned char type, unsigned char v1, unsigned char v2 );
        void	                SetMetaEvent( unsigned char type, unsigned short v );
        void	                SetAllNotesOff( unsigned char chan, unsigned char type=C_ALL_NOTES_OFF );
        void	                SetLocal( unsigned char chan, unsigned char v );
        void	                SetNoOp();
        void	                SetTempo32( unsigned short tempo_times_32 );
        void	                SetText( unsigned short text_num, unsigned char type=META_GENERIC_TEXT );
        void	                SetDataEnd();
        void	                SetTimeSig( unsigned char numerator, unsigned char denominator );
        void                    SetKeySig( signed char sharp_flats, unsigned char major_minor );
        void	                SetBeatMarker();

// WARNING! when using these on a MIDIBigMessage child class we must always call ClearSysEx() or Clear() or
// we could get a memory leak (sysex pointer not freed)

        friend bool             operator== ( const MIDIMessage &m1, const MIDIMessage &m2 );

    protected:

        static	const char* 	chan_msg_name[16];
        static	const char* 	sys_msg_name[16];

        unsigned char	        status;
        unsigned char	        byte1;
        unsigned char	        byte2;
        unsigned char	        byte3;		// byte 3 is only used for meta-events and to
                                            // round out the structure size to 32 bits

};



/* ********************************************************************************************/
/*                     C L A S S   M I D I B i g M e s s a g e                                */
/* ********************************************************************************************/


class MIDIBigMessage : public MIDIMessage {
    public:
      //
      // Constructors
      //

                                MIDIBigMessage();
                                MIDIBigMessage( const MIDIBigMessage &m );
                                MIDIBigMessage( const MIDIMessage &m );
      //
      // Destructor
      //
                                ~MIDIBigMessage();

      void	                    Clear();
      void                      ClearSysEx();

      void	                    Copy( const MIDIBigMessage &m );
      void	                    Copy( const MIDIMessage &m );

      //
      // operator =
      //

      const MIDIBigMessage&     operator = ( const MIDIBigMessage &m );
      const MIDIBigMessage&     operator = ( const MIDIMessage &m );

      //
      // 'Get' methods
      //

      MIDISystemExclusive*      GetSysEx()              { return sysex; }
      const MIDISystemExclusive*GetSysEx() const        { return sysex; }


      //
      // 'Set' methods
      //

      void                      SetSysEx( MIDISystemExclusive *e );
        // this sets the sysex pointer to e (which bust be already allocated, and will be freed by the
        // dtor). It can be used also for meta messages which point to text. It doesn't set the status
        // byte to sysex: this is done by tehe SetSysEx() method


     friend bool                operator== ( const MIDIBigMessage &m1, const MIDIBigMessage &m2 );


      protected:

      MIDISystemExclusive*      sysex;
};



/* ********************************************************************************************/
/*                   C L A S S   M I D I T i m e d M e s s a g e                              */
/* ********************************************************************************************/


class 	MIDITimedMessage : public MIDIMessage
    {
    public:

      //
      // Constructors
      //

                                MIDITimedMessage();
                                MIDITimedMessage( const MIDITimedMessage &m );
                                MIDITimedMessage( const MIDIMessage &m );

        void	                Clear();
        void	                Copy( const MIDITimedMessage &m );

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



/* ********************************************************************************************/
/*                   C L A S S   M I D I T i m e d B i g M e s s a g e                        */
/* ********************************************************************************************/


class 	MIDITimedBigMessage : public MIDIBigMessage {
    public:

      //
      // Constructors
      //

                                MIDITimedBigMessage();
                                MIDITimedBigMessage( const MIDITimedBigMessage &m );
                                MIDITimedBigMessage( const MIDIBigMessage &m );
                                MIDITimedBigMessage( const MIDITimedMessage &m );
                                MIDITimedBigMessage( const MIDIMessage &m );
      //
      // Destructor
      //

                                ~MIDITimedBigMessage();

        void	                Clear();
        void	                Copy( const MIDITimedBigMessage &m );
        void	                Copy( const MIDITimedMessage &m );

      //
      // operator =
      //

        const MIDITimedBigMessage&operator = ( const MIDITimedBigMessage & m );
        const MIDITimedBigMessage &operator = ( const MIDITimedMessage & m );
        const MIDITimedBigMessage &operator = ( const MIDIMessage & m );

      //
      // 'Get' methods
      //

      MIDIClockTime	            GetTime() const         { return time; }

      //
      // 'Set' methods
      //

      void	                    SetTime( MIDIClockTime t ) { time = t; }
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
        const MIDITimedBigMessage &a,
        const MIDITimedBigMessage &b
    );

    // This function is used by the methods that search and insert events in the tracks to find events
    // that are of the same kind and would normally be incompatible.
    // It compares the events *a* and *b* and returns **true** if they have the same MIDI time and they are:
    // - both NoOp
    // - both Note On or Note Off with the same channel and the same note number
    // - both Control Change with the same channel and the same control nunber
    // - both channel messages (not notes or control) with the same channel and type
    // - both MetaEvent with the same meta type
    // - both non channel events (not Meta) with the same status
    // If the input mode is set to INSMODE_REPLACE or INSMODE_INSERT_OR_REPLACE the functions
    // MIDITrack::InsertEvent and MIDITrack::InsertNote search if at same MIDI time of the event to insert exists
    // such an event and, if they find it, they replace the event, rather than inserting it.
    static bool IsSameKind (
        const MIDITimedBigMessage &a,
        const MIDITimedBigMessage &b
    );

    //friend bool operator == ( const MIDITimedBigMessage &m1, const MIDITimedBigMessage &m2 );


    // replaced by operator==
    //static bool BitwiseEqual(const MIDITimedBigMessage& m1, const MIDITimedBigMessage& m2);


    protected:

        MIDIClockTime	time;
};

#endif
