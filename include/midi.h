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


/// \file
/// Contains the MIDI values enumerations (to have readable values instead of hexadecimal values) and some utility function.


#ifndef _JDKMIDI_MIDI_H
#define _JDKMIDI_MIDI_H


/// \addtogroup GLOBALS
//@{
/// A type which can hold a time in MIDI ticks.
/// The MIDI tick is the basis for MIDI clocking: a quarter note is assigned a number of MIDI ticks, and a
/// MIDITimedMessage objetìct has its time set in MIDI ticks.
typedef unsigned long MIDIClockTime;
/// An infinite time.
const MIDIClockTime TIME_INFINITE = 0xffffffff;
//@}


/// \ingroup MIDIENUM
/// Channel_status.
/// These are the type values for a MIDI channel message. For a channel message (between 0x80 ... 0xe0)
/// only the upper four bits of the status byte are affected (and lower four bits represent the channel);
/// all status bytes between 0xf0 ... 0xff are considered system messages.
enum {
    NOTE_OFF	        =0x80,  ///< Note off
    NOTE_ON		        =0x90,  ///< Note on
    POLY_PRESSURE	    =0xa0,  ///< Polyphonic (aftertouch) pressure
    CONTROL_CHANGE	    =0xb0,  ///< Control change
    PROGRAM_CHANGE	    =0xc0,  ///< Program (patch) change
    CHANNEL_PRESSURE    =0xd0,  ///< Channel (afertouch) pressure
    PITCH_BEND	        =0xe0,  ///< Pitch bend
    SYSEX_START	        =0xf0,  ///< Start of a sysex
    MTC		            =0xf1,  ///< MIDI Time Code
    SONG_POSITION	    =0xf2,  ///< Song Position pointer
    SONG_SELECT	        =0xf3,  ///< Song Select Pointer
    TUNE_REQUEST	    =0xf6,  ///< Tune request
    SYSEX_END	        =0xf7,  ///< End of a sysex
    RESET		        =0xff,	///< 0xff never used as reset in a MIDIMessage
    META_EVENT	        =0xff	///< Meta event
};


/// \ingroup MIDIENUM
/// MIDI Real Time Messages.
/// Used for quick one-byte messages intended to be sent during playing.
enum {
    RT_TIMING_CLOCK	    =0xf8,  ///< MIDI Real time clock
    RT_MEASURE_END	    =0xf9,	///< Proposed measure end byte UNUSED
    RT_START		    =0xfa,  ///< Sequencer start
    RT_CONTINUE	        =0xfb,  ///< Sequencer continue
    RT_STOP		        =0xfc,  ///< Sequencer stop
    RT_ACTIVE_SENSE	    =0xfe   ///< MIDI Active sensing
};


/// \ingroup MIDIENUM
/// GM Controller Numbers.
/// General MIDI standardized controller numbers (stored in the first data byte of a
/// Control Change message); the last (between 0x78 ... 0x7f) are the channel mode
/// messages.
enum {
    C_LSB		        =0x20,	///< add this to a non-switch controller to access the LSB.
    C_GM_BANK	        =0x00,	///< General Midi bank select
    C_MODULATION	    =0x01,	///< modulation
    C_BREATH	        =0x02,	///< breath controller
    C_FOOT		        =0x04,	///< foot controller
    C_PORTA_TIME	    =0x05,	///< portamento time
    C_DATA_ENTRY	    =0x06,	///< data entry value
    C_MAIN_VOLUME 	    =0x07,	///< main volume control
    C_BALANCE	        =0x08,	///< balance control
    C_PAN		        =0x0a,	///< panpot stereo control
    C_EXPRESSION	    =0x0b,	///< expression control
    C_GENERAL_1	        =0x10,	///< general purpose controller 1
    C_GENERAL_2	        =0x11,	///< general purpose controller 2
    C_GENERAL_3	        =0x12,	///< general purpose controller 3
    C_GENERAL_4	        =0x13,	///< general purpose controller 4

    C_DAMPER	        =0x40,	///< hold pedal (sustain)
    C_PORTA		        =0x41,	///< portamento switch
    C_SOSTENUTO	        =0x42,	///< sostenuto switch
    C_SOFT_PEDAL	    =0x43,	///< soft pedal
    C_HOLD_2	        =0x45,	///< hold pedal 2

    C_GENERAL_5	        =0x50,	///< general purpose controller 5
    C_GENERAL_6	        =0x51,	///< general purpose controller 6
    C_GENERAL_7	        =0x52,	///< general purpose controller 7
    C_GENERAL_8	        =0x53,	///< general purpose controller 8

    C_EFFECT_DEPTH	    =0x5b,	///< external effects depth
    C_TREMELO_DEPTH	    =0x5c,	///< tremelo depth
    C_CHORUS_DEPTH	    =0x5d,	///< chorus depth
    C_CELESTE_DEPTH	    =0x5e,	///< celeste (detune) depth
    C_PHASER_DEPTH	    =0x5f,	///< phaser effect depth

    C_DATA_INC	        =0x60,	///< increment data value
    C_DATA_DEC	        =0x61,	///< decrement data value

    C_NRPN_LSB	        =0x62,	///< non registered parameter LSB
    C_NRPN_MSB	        =0x63,	///< non registered parameter MSB
    C_RPN_LSB	        =0x64,	///< registered parameter LSB
    C_RPN_MSB	        =0x65,	///< registered parameter MSB

    C_ALL_SOUND_OFF     =0x78,  ///< all sound off
    C_RESET		        =0x79,	///< reset all controllers
    C_LOCAL		        =0x7a,	///< local control on/off
    C_ALL_NOTES_OFF	    =0x7b,	///< all notes off
    C_OMNI_OFF	        =0x7c,	///< omni off, all notes off
    C_OMNI_ON	        =0x7d,	///< omni on, all notes off
    C_MONO		        =0x7e,	///< mono on, all notes off
    C_POLY		        =0x7f	///< poly on, all notes off
};


/// \ingroup MIDIENUM
/// Registered Parameter Numbers.
/// Used by the GS standard in a RPN Control Change message
enum {
    RPN_BEND_WIDTH	    =0x00,	///< bender sensitivity
    RPN_FINE_TUNE	    =0x01,	///< fine tuning
    RPN_COARSE_TUNE     =0x02	///< coarse tuning
};


/// \ingroup MIDIENUM
/// META Event types.
/// This is the byte 1 (after the status) of a message with status 0xff, and these types are the same as
/// MIDIFile meta-events. When the data length is <= 2 bytes, data are stored in bytes 2 and 3 of the
/// MIDIMessage, otherwise in the sysex object. So the format of the meta-events in a MIDIMessage class
/// will be different than the standard MIDIFile meta-events.
enum {
    /// Defines the pattern number of a Type 2 MIDI file or the number of a sequence in a Type 0
    /// or Type 1 MIDI file.\ Should always have a delta time of 0 and come before all MIDI Channel
    /// Events and non-zero delta time events. The data length is 2 bytes.
    META_SEQUENCE_NUMBER	= 0x00,
    /// This and the following are used for embedding ascii text in a MIDI file. They have variable data length
    /// (stored in the sysex object).
    META_GENERIC_TEXT	    = 0x01,
    META_COPYRIGHT		    = 0x02, ///< Text: copyright
    META_TRACK_NAME	        = 0x03, ///< Text: track name
    META_INSTRUMENT_NAME	= 0x04, ///< Text: instrument name
    META_LYRIC_TEXT		    = 0x05, ///< Text: lyric
    META_MARKER_TEXT	    = 0x06, ///< Text: marker
    META_CUE_TEXT		    = 0x07, ///< Text: cue point
    META_PROGRAM_NAME       = 0x08, ///< Text: program name
    META_DEVICE_NAME        = 0x09, ///< Text: device name
    META_GENERIC_TEXT_A     = 0x0A, ///< Text: generic a
    META_GENERIC_TEXT_B     = 0x0B, ///< Text: generic b
    META_GENERIC_TEXT_C     = 0x0C, ///< Text: generic c
    META_GENERIC_TEXT_D     = 0x0D, ///< Text: generic d
    META_GENERIC_TEXT_E     = 0x0E, ///< Text: generic e
    META_GENERIC_TEXT_F     = 0x0F, ///< Text: generic f
    /// Associates a MIDI channel with following meta events\. Its effect is terminated by another
    /// MIDI Channel Prefix event or any non-Meta event. It is often used before an Instrument Name
    /// Event to specify which channel an instrument name represents. The data length is 1 byte.
    META_CHANNEL_PREFIX     = 0x20,
    /// This may be used in multiport environments to associate a track with a specific port\.
    /// The data length is 1 byte.
    META_OUTPUT_CABLE       = 0x21,
    //META_TRACK_LOOP         = 0x2E, I found no documentation for this
    /// The end of track marker in a MIDI file (also used in the MIDITrack object)\. Data length is 0 byte.
    META_END_OF_TRACK       = 0x2F,
    /// Specifies a tempo change and has a length of 3 bytes\. The data is a 3-byte integer,
    /// the number of microseconds for a quarter note. The MIDIMessage::GetTempo() method converts
    /// it into the usual bpm value (a double).
    META_TEMPO              = 0x51,
    /// Specifies the initial SMPTE offset of the beginning of playback\. It has 5 data bytes (stored
    /// in the sysex object) which denote hours, minutes, second frames and subframes of the SMPTE.
    META_SMPTE              = 0x54,
    /// Specifies a musical time signature change\. It has 4 data bytes (stored in the sysex object)
    /// which denote the time numerator, the denominator power of two (1->2, 2->4, 3->8 etc), the metronome
    /// note(24 = quarter, 12 = eigth, 36 dotted quarter etc) and the number of 32th for a quarter note
    ///(usually 8, but you are allowed to change this). The MIDIMessage::GetTimeSigNumerator() and
    /// MidiMessage::GetTimeSigDenominator() methods give you the timesig numerator and denominator.
    META_TIMESIG            = 0x58,
    /// Specifies a musical key signature change\. It has 2 data bytes: the 1st is a signed char denoting
    /// the number of accidents (-7 = 7 flats, 0 = no accidents, +7 = 7 sharps), the second is the mode
    /// (0 = major, 1 = minor).
    META_KEYSIG             = 0x59,
    /// Used to give informations specific to a hardware or software sequencer\. The first Data byte
    /// (or three bytes if the first byte is 0) specifies the manufacturer's ID and the following bytes
    /// contain information specified by the manufacturer. Currently is ignored by the library.
    META_SEQUENCER_SPECIFIC = 0x7F
};


extern const signed char	chan_msg_len[16];
extern const signed char	sys_msg_len[16];
extern const bool           note_is_white[12];


/// \name Utility functions
//<{
/// Returns a readable name for the given channel message status.
const char*                 get_chan_msg_name(unsigned char status);
/// Returns a readable name for the given channel mode (Control change with
/// controller number between 0x78 ... 0x7f).
const char*                 get_chan_mode_msg_name(unsigned char number);
/// Returns a readable name for the given sys message status.
const char*                 get_sys_msg_name(unsigned char status);
/// Returns a readable name for the given meta message status.
const char*                 get_meta_msg_name(unsigned char type);

/// Returns **true** if the MIDI number of the note denotes a white key.
inline bool IsNoteWhite(unsigned char note) {
    return note_is_white[note % 12];
}

/// Returns **true** if the MIDI number of the note denotes a black key.
inline bool IsNoteBlack(unsigned char note) {
    return !note_is_white[note % 12];
}

/// MIDI note number to standard octave conversion.
inline int GetNoteOctave(unsigned char note) {
    return (note/12)-1;
}

/// Converts a MIDI key signature into a readable form.
/// \param sharp_flats the number of accidents as coded in a MIDI keysig mera messge
/// \param major_minor the mode as coded in a MIDI keysig messages
/// \param uppercase if true the key name (A, B, C, ...) is uppercase
/// \param space if true put a space between the key and the mode (es. A m)
/// \param use_Mm if true the mode is M or m, otherwise maj or min
char* KeyName (signed char sharp_flats, unsigned char major_minor, bool uppercase = true,
               bool space = false, bool use_Mm = true);
//<}


/// \name Default constants when initializing messages and classes.
//<{
const int MIDI_DEFAULT_TIMESIG_NUMERATOR = 4;       ///< Timesig numerator
const int MIDI_DEFAULT_TIMESIG_DENOMINATOR = 4;     ///< Timesig denominator
const double MIDI_DEFAULT_TEMPO = 120.0;            ///< Musical tempo
const int MIDI_DEFAULT_KEYSIG_KEY = 0;              ///< Keysig key (C)
const int MIDI_DEFAULT_KEYSIG_MODE = 0;             ///< Keysig mode (major)
//<}


#endif

