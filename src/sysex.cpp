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

#include "../include/sysex.h"

#include <iostream>


const unsigned char MIDISystemExclusive::GMReset_data[] = { 0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7 };
const unsigned char MIDISystemExclusive::GSReset_data[] = { 0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7 };
const unsigned char MIDISystemExclusive::XGReset_data[] = { 0xF0, 0x43, 0x10, 0x4C, 0x00, 0x00, 0x7E, 0x00, 0xF7 };


MIDISystemExclusive::MIDISystemExclusive(int size) : cur_len(0), chk_sum(0) {
    buffer = new unsigned char[size];
    if(buffer)
        max_len = size;
    else
        max_len = 0;
    //std::cout << "Sysex: allocated " << max_len << " bytes in " << &buffer  << " by ctor1\n";
}


MIDISystemExclusive::MIDISystemExclusive(const MIDISystemExclusive &se) :
    max_len(se.max_len), cur_len(se.cur_len), chk_sum(se.chk_sum) {
    buffer = new unsigned char [se.max_len];
    //std::cout << "Sysex: allocated " << max_len << " bytes in " << &buffer  << " by ctor2\n";
    memcpy(buffer, se.buffer, cur_len);
}


MIDISystemExclusive::MIDISystemExclusive(const unsigned char *buf, int max_len_, int cur_len_) :
    max_len(max_len_), cur_len(cur_len_), chk_sum(0) {
    buffer = new unsigned char [max_len_];
    //std::cout << "Sysex: allocated " << max_len << " bytes in " << &buffer  << " by ctor3\n";
    memcpy(buffer, buf, cur_len);
}


MIDISystemExclusive::~MIDISystemExclusive() {
    delete [] buffer;
    //std::cout << "Sysex: freed " << max_len << " bytes in " << &buf  << " by dtor\n";
}


MIDISystemExclusive& MIDISystemExclusive::operator= (const MIDISystemExclusive& se) {
    max_len = se.max_len;
    cur_len = se.cur_len;
    chk_sum = se.chk_sum;
    buffer = new unsigned char [max_len];
    //std::cout << "Sysex: allocated " << max_len << " bytes in " << &buffer  << " by operator=\n";
    memcpy(buffer, se.buffer, cur_len);
    return *this;
}

bool MIDISystemExclusive::operator == (const MIDISystemExclusive &se) const {
    if (cur_len != se.cur_len)
        return false;
    if (cur_len == 0)
        return true;
    return memcmp(buffer, se.buffer, cur_len) == 0 ;
}


bool MIDISystemExclusive::IsGMReset() const {
    if (cur_len != GMReset_len)
        return false;
    return memcmp(buffer, GMReset_data, cur_len) == 0 ;
}


bool MIDISystemExclusive::IsGSReset() const {
    if (cur_len != GSReset_len)
        return false;
    return memcmp(buffer, GSReset_data, cur_len) == 0;
}


bool MIDISystemExclusive::IsXGReset() const {
    if (cur_len != XGReset_len)
        return false;
    return memcmp(buffer, XGReset_data, cur_len) == 0;
}
