/*
 *  libjdksmidi-2004 C++ Class Library for MIDI
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
** Copyright 1986 to 1998 By J.D. Koftinoff Software, Ltd.
**
** All rights reserved.
**
** No one may duplicate this source code in any form for any reason
** without the written permission given by J.D. Koftinoff Software, Ltd.
**
*/
//
// Copyright (C) 2010 V.R.Madgazin
// www.vmgames.com vrm@vmgames.com
//

#include "../include/world.h"
#include "../include/sysex.h"

#ifndef DEBUG_MDSYSEX
# define DEBUG_MDSYSEX 0
#endif

#if DEBUG_MDSYSEX
# undef DBG
# define DBG(a) a
#endif

//#include <mss121/mss.h>
#include <iostream>


const unsigned char MIDISystemExclusive::GMReset_data[] = { 0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7 };
const unsigned char MIDISystemExclusive::GSReset_data[] = { 0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7 };
const unsigned char MIDISystemExclusive::XGReset_data[] = { 0xF0, 0x43, 0x10, 0x4C, 0x00, 0x00, 0x7E, 0x00, 0xF7 };


MIDISystemExclusive::MIDISystemExclusive( int size_ ) {
    buf=new uchar[size_];
    if( buf )
        max_len=size_;
    else
        max_len=0;
    //std::cout << "Allocati " << max_len << " bytes in " << &buf  << " da ctor1\n";
    cur_len=0;
    chk_sum=0;
    deletable=true;
}


MIDISystemExclusive::MIDISystemExclusive( const MIDISystemExclusive &e ) {
    buf = new unsigned char [e.max_len];
    max_len = e.max_len;
    //std::cout << "Allocati " << max_len << " bytes in " << &buf  << " da ctor2\n";
    cur_len = e.cur_len;
    chk_sum = e.chk_sum;
    deletable = true;

    for( int i=0; i<cur_len; ++i )
        buf[i] = e.buf[i];
}


MIDISystemExclusive::MIDISystemExclusive(unsigned char *buf_, int max_len_, int cur_len_, bool deletable_) {
    //std::cout << "Chiamato ctor3. Puntatore" << &buf  << " copiato\n";
    buf=buf_;
    max_len=max_len_;
    cur_len=cur_len_;
    chk_sum=0;
    deletable=deletable_;
}


MIDISystemExclusive::~MIDISystemExclusive() {
    if( deletable )
        delete [] buf;
    //std::cout << "Liberati " << max_len << " bytes in " << &buf  << " da dtor\n";
}


bool operator == ( const MIDISystemExclusive &e1, const MIDISystemExclusive &e2 ) {
    if ( e1.cur_len != e2.cur_len )
        return false;
    if ( e1.cur_len == 0 )
        return true;
    return memcmp( e1.buf, e2.buf, e1.cur_len ) == 0 ;
}
