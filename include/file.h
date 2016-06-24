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
//
// Copyright (C) 2010 V.R.Madgazin
// www.vmgames.com vrm@vmgames.com
//
/*
** Copyright 2016 By N. Cassetta
** myjdkmidi library
**
** CHECKED with jdksmidi.  NO CHANGES:
*/

#ifndef _JDKMIDI_FILE_H
#define _JDKMIDI_FILE_H

#include "midi.h"
#include "msg.h"


///
/// This file contains basic definitions for reading and writing MIDI files
///
const unsigned long _MThd=OSTYPE('M','T','h','d');
const unsigned long  _MTrk=OSTYPE('M','T','r','k');


struct MIDIFileChunk {
    unsigned long id;
    unsigned long length;
};


struct MIDIFileHeader {
    MIDIFileHeader() : format(0), ntrks(0), division(0) {}
    short format;
    short ntrks;
    short division;
};


#endif

