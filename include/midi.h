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
**	Copyright 1986 to 1998 By J.D. Koftinoff Software, Ltd.
**
**	All rights reserved.
**
**	No one may duplicate this source code in any form for any reason
**	without the written permission given by J.D. Koftinoff Software, Ltd.
**
*/


#ifndef _JDKMIDI_MIDI_H
#define _JDKMIDI_MIDI_H

#include "world.h"


//
// MIDI Status bytes
//

  enum
    {
      NOTE_OFF	        =0x80,
      NOTE_ON		    =0x90,
      POLY_PRESSURE	    =0xa0,
      CONTROL_CHANGE	=0xb0,
      PROGRAM_CHANGE	=0xc0,
      CHANNEL_PRESSURE  =0xd0,
      PITCH_BEND	    =0xe0,
      SYSEX_START	    =0xf0,
      MTC		        =0xf1,
      SONG_POSITION	    =0xf2,
      SONG_SELECT	    =0xf3,
      TUNE_REQUEST	    =0xf6,
      SYSEX_END	        =0xf7,
      RESET		        =0xff,	// 0xff never used as reset in a MIDIMessage
      META_EVENT	    =0xff	// 0xff is for non MIDI messages
    };



  //
  // MIDI Real Time Messages
  //

  enum
    {
      TIMING_CLOCK	    =0xf8,
      MEASURE_END	    =0xf9,	// proposed measure end byte
      START		        =0xfa,
      CONTINUE	        =0xfb,
      STOP		        =0xfc,
      ACTIVE_SENSE	    =0xfe
    };


  //
  // Controller Numbers
  //

  enum
    {
      C_LSB		        =0x20,	// add this to a non-switch controller
      // to access the LSB.

      C_GM_BANK	        =0x00,	// general midi bank select
      C_MODULATION	    =0x01,	// modulation
      C_BREATH	        =0x02,	// breath controller
      C_FOOT		    =0x04,	// foot controller
      C_PORTA_TIME	    =0x05,	// portamento time
      C_DATA_ENTRY	    =0x06,	// data entry value
      C_MAIN_VOLUME 	=0x07,	// main volume control
      C_BALANCE	        =0x08,	// balance control
      C_PAN		        =0x0a,	// panpot stereo control
      C_EXPRESSION	    =0x0b,	// expression control
      C_GENERAL_1	    =0x10,	// general purpose controller 1
      C_GENERAL_2	    =0x11,	// general purpose controller 2
      C_GENERAL_3	    =0x12,	// general purpose controller 3
      C_GENERAL_4	    =0x13,	// general purpose controller 4

      C_DAMPER	        =0x40,	// hold pedal (sustain)
      C_PORTA		    =0x41,	// portamento switch
      C_SOSTENUTO	    =0x42,	// sostenuto switch
      C_SOFT_PEDAL	    =0x43,	// soft pedal
      C_HOLD_2	        =0x45,	// hold pedal 2

      C_GENERAL_5	    =0x50,	// general purpose controller 5
      C_GENERAL_6	    =0x51,	// general purpose controller 6
      C_GENERAL_7	    =0x52,	// general purpose controller 7
      C_GENERAL_8	    =0x53,	// general purpose controller 8

      C_EFFECT_DEPTH	=0x5b,	// external effects depth
      C_TREMELO_DEPTH	=0x5c,	// tremelo depth
      C_CHORUS_DEPTH	=0x5d,	// chorus depth
      C_CELESTE_DEPTH	=0x5e,	// celeste (detune) depth
      C_PHASER_DEPTH	=0x5f,	// phaser effect depth

      C_DATA_INC	    =0x60,	// increment data value
      C_DATA_DEC	    =0x61,	// decrement data value

      C_NONRPN_LSB	    =0x62,	// non registered parameter LSB
      C_NONRPN_MSB	    =0x63,	// non registered parameter MSB

      C_RPN_LSB	        =0x64,	// registered parameter LSB
      C_RPN_MSB	        =0x65,	// registered parameter MSB



      C_RESET		    =0x79,	// reset all controllers

      C_LOCAL		    =0x79,	// local control on/off
      C_ALL_NOTES_OFF	=0x7a,	// all notes off
      C_OMNI_OFF	    =0x7c,	// omni off, all notes off
      C_OMNI_ON	        =0x7d,	// omni on, all notes off
      C_MONO		    =0x7e,	// mono on, all notes off
      C_POLY		    =0x7f	// poly on, all notes off
    };


  //
  // Registered Parameter Numbers:
  //

  enum
    {
      RPN_BEND_WIDTH	=0x00,	// bender sensitivity
      RPN_FINE_TUNE	    =0x01,	// fine tuning
      RPN_COARSE_TUNE   =0x02	// coarse tuning
    };



  ///
  /// META Event types (stored in first data byte if status==META_EVENT).
  /// These types are the same as MIDIFile meta-events; when the data length
  /// is <= 2 bytes, data are stored in bytes 2 and 3 of the MIDIMessage,
  /// otherwise in the sysex object. So the format of the meta-events in a
  /// MIDIMessage class will be different than the standard MIDIFile
  /// meta-events.
  ///

  enum
    {
        /// This meta event defines the pattern number of a Type 2 MIDI file
        /// or the number of a sequence in a Type 0 or Type 1 MIDI file.
        /// Should always have a delta time of 0 and come before all MIDI
        /// Channel Events and non-zero delta time events. The data length
        /// is 2 bytes.
      META_SEQUENCE_NUMBER	= 0x00,
        /// Text events used for embedding ascii text in the file. They have
        /// variable data length (stored in the sysex object).
      META_GENERIC_TEXT	    = 0x01,
      META_COPYRIGHT		= 0x02,
      META_TRACK_NAME	    = 0x03,
      META_INSTRUMENT_NAME	= 0x04,
      META_LYRIC_TEXT		= 0x05,
      META_MARKER_TEXT	    = 0x06,
      META_CUE_TEXT		    = 0x07,
      META_PROGRAM_NAME     = 0x08,
      META_DEVICE_NAME      = 0x09,
      META_GENERIC_TEXT_A   = 0x0A,
      META_GENERIC_TEXT_B   = 0x0B,
      META_GENERIC_TEXT_C   = 0x0C,
      META_GENERIC_TEXT_D   = 0x0D,
      META_GENERIC_TEXT_E   = 0x0E,
      META_GENERIC_TEXT_F   = 0x0F,
        /// This meta event associates a MIDI channel with following meta events.
        /// Its effect is terminated by another MIDI Channel Prefix event or any non-Meta event.
        /// It is often used before an Instrument Name Event to specify which channel
        /// an instrument name represents. The data length is 1 byte.
      META_CHANNEL_PREFIX   = 0x20,
        /// This meta event may be used in multiport environments to associate
        /// a track with a specific port. The data length is 1 byte.
      META_OUTPUT_CABLE       = 0x21,
      //META_TRACK_LOOP         = 0x2E, I found no documentation for this
        /// This meta event is the end of track marker. Data length is 0 byte.
      META_END_OF_TRACK       = 0x2F,
        /// This meta event denotes a tempo change and has a length of 3 bytes.
        /// The data is a 3-byte integer, the number of microseconds for a quarter
        /// note. The MIDIMessage::GetTempo() method converts it into the usual
        /// bpm value (a float).
    META_TEMPO                = 0x51,
        /// This meta event specifies the initial SMPTE offset of the beginning of playback. It
        /// has 5 data bytes (stored in the sysex object) which denote hours, minutes, second
        /// frames and subframes of the SMPTE. Currently the SMPTE object doesn't support an
        /// offset.
        // TODO: add offset to SMPTE
    META_SMPTE              = 0x54,
        /// This meta event specifies a musical time signature change. It has 4 data bytes
        /// (stored in the sysex object) which denote the time numerator, the denominator power
        /// of two (1->2, 2->4, 3->8 etc), the metronome note(24 = quarter, 12 = eigth, 36 dotted
        /// quarter etc) and the number of 32th for a quarter note (usually 8, but you are allowed
        /// to change this). The MIDIMessage::GetTimeSigNumerator() and MidiMessage::GetTimeSigDenominator()
        /// methods give you the timesig numerator and denominator.
        /// Currently the MIDISequencer object ignores third byte and always assumes the metronome note
        /// as the timesig denominator.
        // TODO: add this
    META_TIMESIG            = 0x58,
        /// This meta event specifies a musical key signature change. It has 2 data bytes:
        /// the 1st is a signed char denoting the number of accidents (-7 = 7 flats, 0 =
        /// no accidents, +7 = 7 sharps), the second is the mode (0 = major, 1 = minor).
    META_KEYSIG             = 0x59,
        /// This is a dummy, non-MIDI type used internally by the class MIDISequencer to mark
        /// the metronome clicks (a BEAT_MARKER message will be output at every click).
        // TODO: should be better not to use a not MIDI type. Perhaps we could change the message status byte?
    META_BEAT_MARKER        = 0x7e,
        /// This meta event is used to specify information specific to a hardware or
        /// software sequencer. The first Data byte (or three bytes if the first byte is 0)
        /// specifies the manufacturer's ID and the following bytes contain information
        /// specified by the manufacturer. Currently is ignored by the library.
    META_SEQUENCER_SPECIFIC = 0x7F
};





extern const signed char	lut_msglen[16];
extern const signed char	lut_sysmsglen[16];
extern const bool	        lut_is_white[12];


extern const char*          chan_msg_name[16];
extern const char*          sys_msg_name[16];
const char*                 get_meta_name(unsigned char b);

  //
  // Message Length functions. Not valid for Meta-events (0xff)
  //

  inline	signed char	GetMessageLength( unsigned char stat )
    {
      return lut_msglen[stat>>4];
    }

  inline	signed char	GetSystemMessageLength( unsigned char stat )
    {
      return lut_sysmsglen[stat-0xf0];
    }


  //
  // Piano key color methods
  //

  inline	bool	IsNoteWhite( unsigned char note )
    {
      return lut_is_white[ note%12 ];
    }

  inline	bool	IsNoteBlack( unsigned char note )
    {
      return !lut_is_white[ note%12 ];
    }


  //
  // Note # to standard octave conversion
  //

  inline	int	GetNoteOctave( unsigned char note )
    { return (note/12)-1;	}








#endif

