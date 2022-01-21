/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2010 V.R.Madgazin
 *   www.vmgames.com vrm@vmgames.com
 *   Copyright (C) 2021, 2022  Nicola Cassetta
 *   https://github.com/ncassetta/NiCMidi
 *
 *   This file is part of NiCMidi.
 *
 *   NiCMidi is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   NiCMidi is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with NiCMidi.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "../include/sysex.h"
#include <iostream>     // for debug


const unsigned char MIDISystemExclusive::GMReset_data[] = { 0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7 };
const unsigned char MIDISystemExclusive::GSReset_data[] = { 0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7 };
const unsigned char MIDISystemExclusive::XGReset_data[] = { 0xF0, 0x43, 0x10, 0x4C, 0x00, 0x00, 0x7E, 0x00, 0xF7 };


MIDISystemExclusive::MIDISystemExclusive(unsigned int len) : chk_sum(0) {
    if (len != 0)
        buffer.reserve(len);
}


MIDISystemExclusive::MIDISystemExclusive(const MIDISystemExclusive &se) : buffer(se.buffer), chk_sum(se.chk_sum) {
}


MIDISystemExclusive::MIDISystemExclusive(const unsigned char *buf, unsigned int len) : chk_sum(0) {
    for (unsigned int i = 0; i < len; i++)
        buffer.push_back(buf[i]);
}


MIDISystemExclusive& MIDISystemExclusive::operator= (const MIDISystemExclusive& se) {
    chk_sum = se.chk_sum;
    buffer = se.buffer;
    return *this;
}


bool MIDISystemExclusive::operator== (const MIDISystemExclusive &se) const {
    if (buffer != se.buffer)
        return false;
    if (chk_sum != se.chk_sum)
        return false;
    return true;
}


bool MIDISystemExclusive::IsGMReset() const {
    return memcmp(buffer.data(), GMReset_data, buffer.size()) == 0 ;
}


bool MIDISystemExclusive::IsGSReset() const {
    return memcmp(buffer.data(), GSReset_data, buffer.size()) == 0;
}


bool MIDISystemExclusive::IsXGReset() const {
    return memcmp(buffer.data(), XGReset_data, buffer.size()) == 0;
}
