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

//
// MODIFIED by N. Cassetta  ncassetta@tiscali.it
//

#include <cstring>      // for strlen()
#include "../include/msg.h"


////////////////////////////////////////////////////////////////////////////
//                              class MIDIMessage                         //
////////////////////////////////////////////////////////////////////////////

bool MIDIMessage::use_note_onv0 = false;


//
// constructors
//

MIDIMessage::MIDIMessage() : status(0), byte1(0), byte2(0) , byte3(0), sysex(0)
{}


MIDIMessage::MIDIMessage(const MIDIMessage &msg) :
    status(msg.status), byte1(msg.byte1), byte2(msg.byte2), byte3(msg.byte3), sysex(0) {
    if(msg.sysex)
        sysex = new MIDISystemExclusive(*msg.sysex);
}


MIDIMessage::~MIDIMessage() {
    ClearSysEx();
}


void MIDIMessage::Clear() {
    status = byte1 = byte2 = byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::ClearSysEx() {
    if (sysex) {
        delete sysex;
        sysex = 0;
    }
}

//
// operator =
//

const MIDIMessage& MIDIMessage::operator= (const MIDIMessage &msg) {
    if (this != &msg) {
        status = msg.status,
        byte1 = msg.byte1;
        byte2 = msg.byte2;
        byte3 = msg.byte3;
        ClearSysEx();
        if(msg.sysex)
            sysex = new MIDISystemExclusive(*msg.sysex);
    }
    return *this;
}


//
// Query methods
//

char MIDIMessage::GetLength() const {
    if((status & 0xf0) == 0xf0)
        return sys_msg_len[status - 0xf0];
    else
        return chan_msg_len[status >> 4];
}


double MIDIMessage::GetTempo() const {
    double tempo_bpm = 60.0 * 1.0e6 / GetInternalTempo();
    return tempo_bpm;
}


unsigned long MIDIMessage::GetInternalTempo() const {
    unsigned long microsecs_per_beat = sysex->GetData(0);
    microsecs_per_beat <<= 8;
    microsecs_per_beat += sysex->GetData(1);
    microsecs_per_beat <<= 8;
    microsecs_per_beat += sysex->GetData(2);
    return microsecs_per_beat;
}


std::string MIDIMessage::GetText() const {
    std::string s;
    for (int i = 0; i < sysex->GetLength(); i++)
        s += sysex->GetData(i);
    return s;
}

//
// Set methods
//

void MIDIMessage::SetBenderValue(short v) {
    short x = (short)(v + 8192);
    byte1 = (unsigned char)(x & 0x7f);
    byte2 = (unsigned char)((x >> 7) & 0x7f);
}


void MIDIMessage::SetMetaValue(unsigned short v) {
    byte2 = (unsigned char)(v & 0xff);
    byte3 = (unsigned char)((v >> 8) & 0xff);
}


void MIDIMessage::SetNoteOn(unsigned char chan, unsigned char note, unsigned char vel) {
    status = (unsigned char)(chan | NOTE_ON);
    byte1 = note;
    byte2 = vel;
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetNoteOff(unsigned char chan, unsigned char note, unsigned char vel) {
    if (use_note_onv0) {
        status = (unsigned char)(chan | NOTE_ON);
        byte2 = 0;      // vel ignored
    }
    else {
        status = (unsigned char)(chan | NOTE_OFF);
        byte2 = vel;
    }
    byte1 = note;
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetPolyPressure(unsigned char chan, unsigned char note, unsigned char pres) {
    status = (unsigned char)(chan | POLY_PRESSURE);
    byte1 = note;
    byte2 = pres;
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetControlChange(unsigned char chan, unsigned char ctrl, unsigned char val) {
    status = (unsigned char)(chan | CONTROL_CHANGE);
    byte1 = ctrl;
    byte2 = val;
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetProgramChange(unsigned char chan, unsigned char prog) {
    status = (unsigned char)(chan | PROGRAM_CHANGE);
    byte1 = prog;
    byte2 = 0;
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetChannelPressure(unsigned char chan, unsigned char pres) {
    status = (unsigned char)(chan | CHANNEL_PRESSURE);
    byte1 = pres;
    byte2 = 0;
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetPitchBend(unsigned char chan, short val) {
    status = (unsigned char)(chan | PITCH_BEND);
    val += (short)0x2000;	                    // center value
    byte1 = (unsigned char)(val & 0x7f);        // 7 bit bytes
    byte2 = (unsigned char)((val >> 7) & 0x7f);
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetSysEx(const MIDISystemExclusive* se) {
    status=SYSEX_START;
    byte1 = 0;
    byte2 = 0;
    byte3 = 0;
    CopySysEx(se);
}


void MIDIMessage::SetMTC(unsigned char field, unsigned char val) {
    status = MTC;
    byte1 = (unsigned char)((field << 4) | val);
    byte2 = 0;
    byte3 = 0;
    ClearSysEx();
  }


void MIDIMessage::SetSongPosition(short pos) {
    status = SONG_POSITION;
    byte1 = (unsigned char)(pos & 0x7f);
    byte2 = (unsigned char)((pos >> 7) & 0x7f);
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetSongSelect(unsigned char sng) {
    status = SONG_SELECT;
    byte1 = sng;
    byte2 = 0;
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetSystemMessage(unsigned char type) {
    status = type;
    byte1 = 0;
    byte2 = 0;
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetMetaEvent(unsigned char type, unsigned char v1, unsigned char v2) {
    status = META_EVENT;
    byte1 = type;
    byte2 = v1;
    byte3 = v2;
    ClearSysEx();
}


void MIDIMessage::SetMetaEvent(unsigned char type, unsigned short val) {
    status = META_EVENT;
    byte1 = type;
    byte2 = (unsigned char)(val & 0xff);
    byte3 = (unsigned char)((val >> 8) & 0xff);
    ClearSysEx();
}


void MIDIMessage::SetText(const char* text, unsigned char type) {
    SetMetaEvent(type, 0);
    AllocateSysEx(strlen(text));
    for(unsigned int i = 0; i < strlen(text); ++i)
        sysex->PutSysByte(text[i]);
}


void MIDIMessage::SetTempo(double tempo_bpm) {
    SetMetaEvent(META_TEMPO, 0);
    AllocateSysEx(3);
    unsigned long microsecs_per_beat = (unsigned long)(60.0 * 1.0e6 / tempo_bpm);
            // microseconds in a minute / bpm
    sysex->PutSysByte((microsecs_per_beat >> 16) & 0xff);
    sysex->PutSysByte((microsecs_per_beat >> 8) & 0xff);
    sysex->PutSysByte(microsecs_per_beat & 0xff);
}


void MIDIMessage::SetSMPTEOffset(unsigned char hour, unsigned char min, unsigned char sec,
                                 unsigned char frame,unsigned char subframe) {
    SetMetaEvent(META_SMPTE, 0);
    AllocateSysEx(5);
    sysex->PutSysByte(hour);
    sysex->PutSysByte(min);
    sysex->PutSysByte(sec);
    sysex->PutSysByte(frame);
    sysex->PutSysByte(subframe);
}


void MIDIMessage::SetTimeSig(unsigned char num, unsigned char den,
                             unsigned char clocks_per_metronome, unsigned char num_32_per_quarter) {
    SetMetaEvent(META_TIMESIG, num, den);             // now den is in byte3
    AllocateSysEx(4);
    unsigned char power_of_2 = 0;
    while (den > 1) {
        power_of_2++;
        den >>= 1;
    }
    if (clocks_per_metronome == 0)      // assume as metronome the numerator value
        clocks_per_metronome = 24 * 4 / byte3;
    sysex->PutSysByte(num);
    sysex->PutSysByte(power_of_2);
    sysex->PutSysByte(clocks_per_metronome);
    sysex->PutSysByte(num_32_per_quarter);
}


void MIDIMessage::SetBeatMarker() {
    status = STATUS_SERVICE;
    byte1 = BEAT_MARKER_VAL;
    byte2 = 0;
    byte3 = 0;
    ClearSysEx();
}


//
// MsgToText()
//

std::string MIDIMessage::MsgToText () const {
    char buf[256];
    std::string txt;

    // Meta Events
    if (IsMetaEvent()) {
        sprintf (buf, "%s ", get_sys_msg_name(status));
        txt += buf;
        txt += get_meta_msg_name(byte1);            // type of meta

        switch (byte1) {

            case META_SEQUENCE_NUMBER:          // 2 byte meta events
                sprintf (buf, "Data %02X  ", ((int)byte2 >> 8) + byte3);
                break;

            case META_GENERIC_TEXT:             // text meta events
            case META_COPYRIGHT:
            case META_TRACK_NAME:
            case META_INSTRUMENT_NAME:
            case META_LYRIC_TEXT:
            case META_MARKER_TEXT:
            case META_CUE_TEXT:
            case META_PROGRAM_NAME:
            case META_DEVICE_NAME:
            case META_GENERIC_TEXT_A:
            case META_GENERIC_TEXT_B:
            case META_GENERIC_TEXT_C:
            case META_GENERIC_TEXT_D:
            case META_GENERIC_TEXT_E:
            case META_GENERIC_TEXT_F:
                sprintf(buf, "\"%s\"", GetText().c_str());
                if (strlen(buf) > 40)
                    buf[40] = 0;
                break;

            case META_CHANNEL_PREFIX:       // 1 byte meta events
            case META_OUTPUT_CABLE:
                sprintf(buf, "Data %2d", byte2);
                break;
            //META_TRACK_LOOP         = 0x2E, I found no documentation for this

            case META_END_OF_TRACK:         // 0 byte meta event
                *buf = 0;
                break;

            case META_TEMPO:
                sprintf(buf, "BpM  %3.2f", GetTempo());
                break;

            case META_SMPTE:
                sprintf(buf, "Data %02d %02d %02d %02d %02d",
                        sysex->GetData(0), sysex->GetData(1), sysex->GetData(2),
                        sysex->GetData(3), sysex->GetData(4));
                break;

            case META_TIMESIG:
                sprintf(buf, "Time %d/%d (Other data %02d %02d)",
                        GetTimeSigNumerator(), GetTimeSigDenominator(),
                        sysex->GetData(2), sysex->GetData(3));
                break;

            case META_KEYSIG:
                sprintf(buf, "Key %s", KeyName(byte2, byte3));
                break;

            default:
                *buf = 0;
                break;
        }
        txt += buf;
    }

    // System Exclusive Events
    else if (IsSysEx()) {
        sprintf (buf, "%s ", get_sys_msg_name(status));
        txt += buf;
        if (GetSysEx()->IsGMReset())
            sprintf(buf, "GM Reset");
        else if (GetSysEx()->IsGSReset())
            sprintf(buf, "GS Reset");
        else if (GetSysEx()->IsXGReset())
            sprintf(buf, "XG Reset");
        else
            sprintf (buf, "(length: %d)", GetSysEx()->GetLength());
        txt += buf;
    }

    // Channel Events
    else {

        sprintf (buf, "Ch %2d     ", (int) GetChannel() + 1);
        txt += buf;

        if (IsChannelMode()) {
            sprintf (buf, "%s ", get_chan_mode_msg_name(GetController()));
            txt += buf;
            if (GetType() == C_LOCAL)
                txt += (byte1 ? " On" : " Off");
        }
        else {
            sprintf (buf, "%s  ", get_chan_msg_name(GetType()));
            txt += buf;
            switch (status & 0xf0) {
                case NOTE_ON:
                    if (GetVelocity() == 0) // velocity = 0: Note off
                        sprintf (buf, "Note %3d  Vel  %3d    (Note Off)  ", (int) byte1, (int) byte2);
                    else
                        sprintf (buf, "Note %3d  Vel  %3d  ", (int) byte1, (int) byte2);
                    break;

                case NOTE_OFF:
                    sprintf (buf, "Note %3d  Vel  %3d  ", (int) byte1, (int) byte2 );
                    break;

                case POLY_PRESSURE:
                    sprintf (buf, "Note %3d  Pres %3d  ", (int) byte1, (int) byte2 );
                    break;

                case CONTROL_CHANGE:
                    sprintf (buf, "Ctrl %3d  Val  %3d  ", ( int ) byte1, ( int ) byte2 );
                    break;

                case PROGRAM_CHANGE:
                    sprintf (buf, "Prog %3d  ", (int) byte1);
                    break;

                case CHANNEL_PRESSURE:
                    sprintf (buf, "Pres %3d  ", (int) byte1 );
                    break;

                case PITCH_BEND:
                    sprintf (buf, "Val %5d  ", (int) GetBenderValue() );
                    break;
            }
            txt += buf;
        }
    }
    return txt;
}


void MIDIMessage::AllocateSysEx(unsigned int len) {
    ClearSysEx();
    sysex = new MIDISystemExclusive(len);
}


void MIDIMessage::CopySysEx(const MIDISystemExclusive* se) {
    ClearSysEx();
    if (se)
        sysex = new MIDISystemExclusive(*se);
}


bool operator== (const MIDIMessage &m1, const MIDIMessage &m2) {
    if (m1.status != m2.status ||
        m1.byte1 != m2.byte1 ||
        m1.byte2 != m2.byte2 ||
        m1.byte3 != m2.byte3)
        return false;
    if ((m1.sysex == 0 && m2.sysex != 0) ||
        (m1.sysex != 0 && m2.sysex == 0))
        return false;
    if (m1.sysex == 0 && m2.sysex == 0)
        return true;
    return (*m1.sysex == *m2.sysex);
}


////////////////////////////////////////////////////////////////////////////
//                         class MIDITimedMessage                         //
////////////////////////////////////////////////////////////////////////////

//
// Constructors
//

MIDITimedMessage::MIDITimedMessage() : time(0) {}


MIDITimedMessage::MIDITimedMessage(const MIDITimedMessage &msg)
    : MIDIMessage(msg), time(msg.time)
{}


MIDITimedMessage::MIDITimedMessage(const MIDIMessage &msg)
    : MIDIMessage(msg), time(0)
{}


MIDITimedMessage::~MIDITimedMessage()
{}


void MIDITimedMessage::Clear() {
    time = 0;
    MIDIMessage::Clear();
}

//
// operator =
//

const MIDITimedMessage &MIDITimedMessage::operator= (const MIDITimedMessage &msg) {
    time = msg.time;
    MIDIMessage::operator= (msg);
    return *this;
}


const MIDITimedMessage &MIDITimedMessage::operator= (const MIDIMessage &msg) {
    time = 0;
    MIDIMessage::operator= (msg);
    return *this;
}

//
// MsgToText()
//

std::string MIDITimedMessage::MsgToText() const {
    char buf[256];
    std::string txt;

    sprintf (buf, "%8ld : ", GetTime());
    txt += buf;
    txt += MIDIMessage::MsgToText();
    return txt;
}


int CompareEventsForInsert (const MIDITimedMessage &m1, const MIDITimedMessage &m2) {
    bool n1 = m1.IsNoOp();
    bool n2 = m2.IsNoOp();
    // NOP's always are larger.

    if ( n1 && n2 )
        return 0; // same, do not care.
    if ( n1 )
        return 1; // m1 is larger
    if ( n2 )
        return 2; // m2 is larger

    if ( m1.GetTime() > m2.GetTime() )
        return 1; // m1 is larger

    if ( m2.GetTime() > m1.GetTime() )
        return 2; // m2 is larger

    n1 = m1.IsDataEnd();
    n2 = m2.IsDataEnd();
    // EndOfTrack are larger
    if ( n1 && n2 )
        return 0; // same
    if ( n1 )
        return 1; // m1 is larger
    if ( n2 )
        return 2; // m2 is larger

    n1 = m1.IsMetaEvent();
    n2 = m2.IsMetaEvent();
    // Meta events go before other events
    if (n1 && n2)
        return 0; // same
    if (n1)
        return 2; // m2 is larger
    if (n2)
        return 1;
    // m1 is larger

    n1 = m1.IsSysEx();
    n2 = m2.IsSysEx();
    // System exclusive are larger
    if ( n1 && n2 )
        return 0; // same
    if ( n1 )
        return 1; // m1 is larger
    if ( n2 )
        return 2; // m2 is larger

    if ( m1.IsChannelMsg() && m2.IsChannelMsg() )
    {
        if ( m1.GetChannel() != m2.GetChannel() )
            return m1.GetChannel() < m2.GetChannel();

        else
        {
            n1 = ! m1.IsNote();
            n2 = ! m2.IsNote();
            if (n1 && n2)
                return 0; // same
            if (n1)
                return 2; // m2 is larger
            if (n2)
                return 1; // m1 is larger

            n1 = m1.IsNoteOn();
            n2 = m2.IsNoteOn();
            if (n1 && n2)
                return 0; // same
            if (n1)
                return 1; // m1 is larger
            if (n2)
                return 2; // m2 is larger
        }
    }

    return 0;
}

//
// Friend functions for comparing
//

bool IsSameKind (const MIDITimedMessage &m1, const MIDITimedMessage &m2) {
    if (m1.IsNoOp() && m2.IsNoOp())
        return true;

    if (m1.GetTime() != m2.GetTime())
        return false;

    if (m1.IsChannelMsg() && m2.IsChannelMsg() &&
        m1.GetChannel() == m2.GetChannel()) {
        if (m1.GetType() != m2.GetType())
            return false;
        if (m1.IsNoteOn() && m2.IsNoteOn() && m1.GetNote() != m2.GetNote())
            return false;
        if (m1.IsNoteOff() && m2.IsNoteOff() && m1.GetNote() != m2.GetNote())
            return false;
        if (m1.IsControlChange() && m2.IsControlChange() && m1.GetController() != m2.GetController())
            return false;
        return true;
    }
    if (m1.IsMetaEvent() && m2.IsMetaEvent()) {
        if (m1.GetMetaType() == m2.GetMetaType())
            return true;
        return false;
    }
    else if (m1.GetStatus() == m2.GetStatus())
        return true;
    return false;
}


bool operator== (const MIDITimedMessage &m1, const MIDITimedMessage &m2) {
    if (m1.GetTime() != m2.GetTime())
        return false;
    return ((MIDIMessage)m1 == (MIDIMessage)m2);
}

