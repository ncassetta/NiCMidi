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


#include "../include/world.h"
#include "../include/midi.h"


const char* get_chan_msg_name(unsigned char status) {
    static const char* chan_msg_names[16] = {
        "ERROR 0x00    ",		// 0x00
        "ERROR 0x10    ",		// 0x10
        "ERROR 0x20    ",		// 0x20
        "ERROR 0x30    ",       // 0x30
        "ERROR 0x40    ",       // 0x40
        "ERROR 0x50    ",       // 0x50
        "ERROR 0x60    ",   	// 0x60
        "ERROR 0x70    ",    	// 0x70
        "NOTE OFF      ",		// 0x80
        "NOTE ON       ",		// 0x90
        "POLY PRESSURE ",		// 0xa0
        "CTRL CHANGE   ",		// 0xb0
        "PROG CHANGE   ",		// 0xc0
        "CHAN PRESSURE ",		// 0xd0
        "PITCH BEND    ",		// 0xe0
        "SYSTEM        "}; 		// 0xf0
    return chan_msg_names[status >> 4];
}

const char* get_chan_mode_name(unsigned char status) {
    static const char* chan_mode_names[8] = {
        "ALL SOUND OFF ",       // 0x78
        "RESET ALL CONTROLLERS",// 0x79
        "LOCAL ON/OFF  ",       // 0x7a
        "ALL NOTES OFF ",       // 0x7b
        "OMNI OFF      ",       // 0x7c
        "OMNI ON       ",       // 0x7d
        "MONO ON       ",       // 0x7e
        "POLY ON       "};      // 0x7f
    return chan_mode_names[status - C_ALL_SOUND_OFF];
}


const char* sys_msg_names[16] = {
    "SYSEX    ",		// 0xf0
    "MTC      ",		// 0xf1
    "SONG POS ",		// 0xf2
    "SONG SEL ",		// 0xf3
    "ERROR    ",		// 0xf4
    "ERROR    ",		// 0xf5
    "TUNE REQ ",		// 0xf6
    "SYSEX END",		// 0xf7
    "CLOCK    ",		// 0xf8
    "MEAS END ",		// 0xf9
    "START    ",		// 0xfa
    "CONTINUE ",		// 0xfb
    "STOP     ",        // 0xfc
    "ERROR    ",        // 0xfd
    "SENSE    ",		// 0xfe
    "META EV  "		    // 0xff
};


const char* get_meta_name(unsigned char b) {
    static const char* meta_msg_names[18] {
        "SEQUENCE NUMBER ",	    // 0x00,
        "GENERIC TEXT    ",     // 0x01,
        "COPYRIGHT       ",		// 0x02,
        "INSTRUMENT NAME ",	    // 0x03,
        "TRACK NAME      ",		// 0x04,
        "LYRIC TEXT      ",		// 0x05,
        "MARKER TEXT     ",	    // 0x06,
        "CUE TEXT        ",		// 0x07,
        "OUTPUT CABLE    ",     // 0x21,
        "TRACK LOOP      ",     // 0x2E,
        "END OF TRACK    ",     // 0x2F,
        "TEMPO           ",     // 0x51,
        "SMPTE           ",     // 0x54,
        "TIMESIG         ",     // 0x58,
        "KEYSIG          ",     // 0x59,
        "BEAT MARKER     ",     // 0x7e,
        "NO OPERATION    ",     // 0x7f
        "META ERROR      "      // others
    };
    if (b < 0x08)
        return meta_msg_names[b];
    switch(b) {
        case 0x21:
            return meta_msg_names[8];
        case 0x2E:
            return meta_msg_names[9];
        case 0x2F:
            return meta_msg_names[10];
        case 0x51:
            return meta_msg_names[11];
        case 0x54:
            return meta_msg_names[12];
        case 0x58:
            return meta_msg_names[13];
        case 0x59:
            return meta_msg_names[14];
        case 0x7E:
            return meta_msg_names[15];
        case 0x7F:
            return meta_msg_names[16];
        default:
            return meta_msg_names[17];
    }
}




  const signed char lut_msglen[16] =
  {
    0,0,0,0,0,0,0,0,
    3,	// 0x80=note off, 3 bytes
    3,	// 0x90=note on, 3 bytes
    3, 	// 0xa0=poly pressure, 3 bytes
    3,	// 0xb0=control change, 3 bytes
    2,	// 0xc0=program change, 2 bytes
    2,	// 0xd0=channel pressure, 2 bytes
    3,	// 0xe0=pitch bend, 3 bytes
    -1	// 0xf0=other things. may vary.
  };

  const signed char lut_sysmsglen[16] =
  {
    -1,	// 0xf0=sysex start. may vary
    2,	// 0xf1=MIDI Time Code. 2 bytes
    3,	// 0xf2=MIDI Song position. 3 bytes
    2,	// 0xf3=MIDI Song Select. 2 bytes.
    0,	// 0xf4=undefined
    0,	// 0xf5=undefined
    1,	// 0xf6=TUNE Request
    0,	// 0xf7=sysex end.
    1,	// 0xf8=timing clock. 1 byte
    1,	// 0xf9=proposed measure end?
    1,	// 0xfa=start. 1 byte
    1,	// 0xfb=continue. 1 byte
    1,	// 0xfc=stop. 1 byte
    0,	// 0xfd=undefined
    1,	// 0xfe=active sensing. 1 byte
    -1	// 0xff= not reset, but a META-EVENT, which may vary
  };


  const bool	lut_is_white[12] =
  {
//
//	C C# D D# E F F# G G# A A# B
//
    1,0, 1,0, 1,1,0, 1,0, 1,0, 1
  };




