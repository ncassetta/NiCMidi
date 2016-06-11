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


#include "../include/world.h"
#include "../include/sysex.h"
#include "../include/msg.h"


/* ********************************************************************************************/
/*                         C L A S S   M I D I M e s s a g e                                  */
/* ********************************************************************************************/




/* OLD VERSION OF MY PREVIOUS FILE
char *	MIDIMessage::MsgToText( char *txt ) const {
    char buf[64];
    int len=GetLength();

    *txt=0;

    if( IsAllNotesOff() ) {
        sprintf( buf, "Ch %2d  All Notes Off  (ctrl=%3d)", (int)GetChannel()+1, (int)byte1 );
        strcat( txt, buf );
    }
    else {
        int type=(status&0xf0)>>4;

      //
      // if it is a note on with vel=0, call it a NOTE OFF
      //
        if( type==9 && byte2==0 )
            type=8;
        if( type!=0xf ) {
            sprintf( buf, "Ch %2d  ", (int)GetChannel()+1 );
            strcat( txt, buf  );
      }
      strcat( txt, chan_msg_name[type] );

      if( status>=0xf0 ) {
            strcat( txt, sys_msg_name[status-0xf0] );
            if( len>1 ) {
                sprintf( buf,"%02x", (int)byte1 );
                strcat( txt, buf );
            }

            if( len>2 ) {
                sprintf( buf,",%02x", (int)byte2 );
                strcat( txt, buf );
            }

            if( len>3 ) {
                sprintf( buf,",%02x", (int)byte3 );
                strcat( txt, buf );
            }
        }

      else {
            char *endtxt = txt+strlen(txt);
            switch( status&0xf0 ) {
                case NOTE_ON:
                    if( byte2==0 )
                        sprintf( endtxt, "Note %3d", (int)byte1 );
                else
                    sprintf( endtxt, "Note %3d  Vel  %3d  ", (int)byte1, (int)byte2 );
                break;
                case NOTE_OFF:
                    sprintf( endtxt, "Note %3d  Vel  %3d  ", (int)byte1, (int)byte2 );
                break;
                case POLY_PRESSURE:
                    sprintf( endtxt, "Note %3d  Pres %3d  ", (int)byte1, (int)byte2 );
                    break;
                case CONTROL_CHANGE:
                    sprintf( endtxt, "Ctrl %3d  Val  %3d  ", (int)byte1, (int)byte2 );
                    break;
                case PROGRAM_CHANGE:
                    sprintf( endtxt, "PG   %3d  ", (int)byte1 );
                    break;
                case CHANNEL_PRESSURE:
                    sprintf( endtxt, "Pres %3d  ", (int)byte1 );
                    break;
                case PITCH_BEND:
                    sprintf( endtxt, "Val %5d", (int)GetBenderValue() );
                    break;
            }
        }
    }

    //
    // pad the rest with spaces
    //

    {
        int len=strlen(txt);
        char *p=txt+len;

        while( len<45 ) {
            *p++=' ';
            ++len;
        }
        *p++='\0';
    }

    return txt;
}
*/


char* MIDIMessage::MsgToText (char *txt) const
{
    char buf[256];
    int len = GetLength();
    *txt = 0;

    if (IsMetaEvent()) {        // all Meta Events

        sprintf (buf, "%s ", sys_msg_name[status - 0xF0]);
        strcat (txt, buf);
        strcat (txt, get_meta_name(byte1) );    // type of meta

        if (len > 0) {                          // meta data
            sprintf (buf, "Data %02X  ", (int) byte2);
            strcat (txt, buf);
        }
        if (len > 1) {
            sprintf (buf, "%02X  ", (int) byte3);
            strcat (txt, buf);
        }
        return txt;
    }

    if (IsSysEx()) {            // all System Exclusive Events
        sprintf (buf, "%s ", sys_msg_name[status - 0xF0]);
        strcat (txt, buf);
        return txt;
    }

    // else all Channel Events

    if (IsAllNotesOff()) {
        sprintf (buf, "Ch %2d  All Notes Off  Type %3d  Mode %3d ", (int)GetChannel()+1, (int)byte1, (int)byte2);
        strcat (txt, buf);
        return txt;
    }

    int type = (status & 0xF0) >> 4;

    sprintf (buf, "Ch %2d     ", (int) GetChannel() + 1);
    strcat (txt, buf);
    sprintf (buf, "%s  ", chan_msg_name[type]);
    strcat (txt, buf);

    char *endtxt = txt + strlen (txt);

    switch (status & 0xf0) {
        case NOTE_ON:
            if (GetVelocity() == 0) // velocity = 0: Note off
                sprintf (endtxt, "Note %3d  Vel  %3d    (Note Off)  ", (int) byte1, (int) byte2);
        else
            sprintf (endtxt, "Note %3d  Vel  %3d  ", (int) byte1, (int) byte2);
        break;

    case NOTE_OFF:
        sprintf (endtxt, "Note %3d  Vel  %3d  ", (int) byte1, (int) byte2 );
        break;

    case POLY_PRESSURE:
        sprintf (endtxt, "Note %3d  Pres %3d  ", (int) byte1, (int) byte2 );
        break;

    case CONTROL_CHANGE:
        if ( IsAllNotesOff() )
            sprintf ( endtxt, "Ctrl %3d  Val  %3d  (All Notes Off)  ", ( int ) byte1, ( int ) byte2 );
        else
            sprintf ( endtxt, "Ctrl %3d  Val  %3d  ", ( int ) byte1, ( int ) byte2 );
        break;

    case PROGRAM_CHANGE:
        sprintf (endtxt, "PG   %3d  ", (int) byte1);
        break;

    case CHANNEL_PRESSURE:
        sprintf (endtxt, "Pres %3d  ", (int) byte1 );
        break;

    case PITCH_BEND:
        sprintf (endtxt, "Val %5d  ", (int) GetBenderValue() );
        break;
    }

    return txt;
}

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
    status = msg.status,
    byte1 = msg.byte1;
    byte2 = msg.byte2;
    byte3 = msg.byte3;
    ClearSysEx();
    if(msg.sysex)
        sysex = new MIDISystemExclusive(*msg.sysex);
    return *this;
}

//
// destructor
//

MIDIMessage::~MIDIMessage() {
    ClearSysEx();
}

//
// Query methods
//


char MIDIMessage::GetLength() const {
    if((status & 0xf0) == 0xf0)
        return GetSystemMessageLength(status);
    else
        return GetMessageLength(status);
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


void MIDIMessage::AllocateSysEx(unsigned int len) {
    ClearSysEx();
    sysex = new MIDISystemExclusive(len);
}


void MIDIMessage::CopySysEx(const MIDISystemExclusive* se) {
    ClearSysEx();
    if (se)
        sysex = new MIDISystemExclusive(*se);
}


void MIDIMessage::SetNoteOn(unsigned char chan, unsigned char note, unsigned char vel) {
    status = (unsigned char)(chan | NOTE_ON);
    byte1 = note;
    byte2 = vel;
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetNoteOff(unsigned char chan, unsigned char note, unsigned char vel) {
    status = (unsigned char)(chan | NOTE_OFF);
    byte1 = note;
    byte2 = vel;
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


void MIDIMessage::SetTuneRequest() {
    status = TUNE_REQUEST;
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


void MIDIMessage::SetMetaEvent(unsigned char type, unsigned short v) {
    status = META_EVENT;
    byte1 = type;
    byte2 = (unsigned char)(v & 0xff);
    byte3 = (unsigned char)((v >> 8) & 0xff);
    ClearSysEx();
}


void MIDIMessage::SetAllNotesOff(unsigned char chan, unsigned char type) {
    status = (unsigned char)(chan | CONTROL_CHANGE);
    byte1 = type;
    byte2 = 0x7f;
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetLocal(unsigned char chan, unsigned char v) {
    status = (unsigned char)(chan | CONTROL_CHANGE);
    byte1 = C_LOCAL;
    byte2 = v;
    byte3 = 0;
    ClearSysEx();
}


void MIDIMessage::SetTempo32(unsigned short tempo_times_32) {
    SetMetaEvent(META_TEMPO, tempo_times_32);
}


void MIDIMessage::SetText(const char* text, unsigned char type) {
    SetMetaEvent(type, 0);
    sysex = new MIDISystemExclusive(strlen(text));
    for(int i = 0; i < strlen(text); ++i)
        sysex->PutSysByte(text[i]);
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
    return (*m1.sysex == *m2.sysex);
}


/* ********************************************************************************************/
/*                     C L A S S   M I D I B i g M e s s a g e                                */
/* ********************************************************************************************/




/*

MIDIBigMessage::MIDIBigMessage(const MIDIBigMessage &m) : MIDIMessage(m), sysex(0) {
    if(m.sysex)
        sysex = new MIDISystemExclusive( *m.sysex );
}


MIDIBigMessage::MIDIBigMessage( const MIDIMessage &m ) :
    MIDIMessage( m ), sysex( 0 )   {}


void MIDIBigMessage::Clear() {
    if(sysex) {
        delete sysex;
        sysex = 0;
    }
    MIDIMessage::Clear();
}


void MIDIBigMessage::ClearSysEx() {
    if (sysex) {
        delete sysex;
        sysex = 0;
    }
}



//
// destructors
//

MIDIBigMessage::~MIDIBigMessage() {
    if(sysex) {
        delete sysex;
        sysex = 0;
    }
}

//
// operator =
//

const MIDIBigMessage &MIDIBigMessage::operator = (const MIDIBigMessage &m) {
    if (sysex) {
        delete sysex;
        sysex = 0;
    }
    if(m.sysex)
        sysex = new MIDISystemExclusive( *m.sysex );
    MIDIMessage::operator= (m);
    return *this;
}


const MIDIBigMessage &MIDIBigMessage::operator= (const MIDIMessage &m) {
    if (sysex) {
        delete sysex;
        sysex = 0;
    }
    MIDIMessage::operator= (m);
    return *this;
}
*/
/* ********************************************************************************************/
/*                   C L A S S   M I D I T i m e d M e s s a g e                              */
/* ********************************************************************************************/
/*
//
// Constructors
//

MIDITimedMessage::MIDITimedMessage() : time(0) {}


MIDITimedMessage::MIDITimedMessage(const MIDITimedMessage &msg) : MIDIMessage(msg), time(msg.time)
{}


MIDITimedMessage::MIDITimedMessage(const MIDIMessage &msg) : MIDIMessage(msg), time(0) {}


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


bool operator== (const MIDITimedMessage &m1, const MIDITimedMessage &m2) {
    if (m1.time != m2.time)
        return false;
    return ((MIDIMessage)m1) == ( (MIDIMessage) m2 );
}
*/

/* ********************************************************************************************/
/*                   C L A S S   M I D I T i m e d B i g M e s s a g e                        */
/* ********************************************************************************************/


char* MIDITimedMessage::MsgToText(char* txt) const {
    char msgbuf[1024];
    int c;

    c = sprintf (txt, "%8ld : %s", GetTime(), MIDIMessage::MsgToText (msgbuf));
    if (IsSysEx()) {
        if (GetSysEx()->IsGMReset())
            sprintf(txt + c, "GM Reset\n");
        else if (GetSysEx()->IsGSReset())
            sprintf(txt + c, "GS Reset\n");
        else if (GetSysEx()->IsXGReset())
            sprintf(txt + c, "XG Reset\n");
        else
            sprintf (txt + c, "(length: %d)\n", GetSysEx()->GetLength());
    }
    else
        sprintf(txt + c, "\n");
    return txt;
}


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


void MIDITimedMessage::AddTime(MIDIClockTime t) {
    time += t;
}


void MIDITimedMessage::SubTime(MIDIClockTime t) {
    time = (t > time ? 0 : time - t);
}



/* Replaced by operator==
bool MIDITimedBigMessage::BitwiseEqual (const MIDITimedBigMessage& m1, const MIDITimedBigMessage& m2) {
    if (m1.status != m2.status) return false;
    if (m1.byte1 != m2.byte1) return false;
    if (m1.byte2 != m2.byte2) return false;
    if (m1.byte3 != m2.byte3) return false;
    if (m1.sysex == 0 && m2.sysex == 0) return true;
    if (m1.sysex->GetLength() != m2.sysex->GetLength()) return false;
    for (int i = 0; i < m1.sysex->GetLength(); i++)
        if (m1.sysex->GetData(i) != m2.sysex->GetData(i)) return false;
    return true;
}
*/

// can be two different SYSEX at same time? This forbids it!!!



int  MIDITimedMessage::CompareEventsForInsert (const MIDITimedMessage &m1, const MIDITimedMessage &m2) {
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


bool  MIDITimedMessage::IsSameKind (const MIDITimedMessage &m1, const MIDITimedMessage &m2) {
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




/* THESE OLD COMPARISON FUNCTIONS SHOULD BE UNUSED


int  MIDITimedMessage::CompareEvents (
    const MIDITimedMessage &m1,
    const MIDITimedMessage &m2
)
{
    bool n1 = m1.IsNoOp();
    bool n2 = m2.IsNoOp();
    // NOP's always are larger.

    if ( n1 && n2 )
        return 0; // same, do not care.

    if ( n2 )
        return 2; // m2 is larger

    if ( n1 )
        return 1; // m1 is larger

    if ( m1.GetTime() > m2.GetTime() )
        return 1; // m1 is larger

    if ( m2.GetTime() > m1.GetTime() )
        return 2; // m2 is larger

    // if times are the same, a note off is always larger

    if ( m1.byte1 == m2.byte1
            && m1.GetStatus() == NOTE_ON
            && ( ( m2.GetStatus() == NOTE_ON && m2.byte2 == 0 ) || ( m2.GetStatus() == NOTE_OFF ) )
       )
        return 2; // m2 is larger

    if ( m1.byte1 == m2.byte1
            && m2.GetStatus() == NOTE_ON
            && ( ( m1.GetStatus() == NOTE_ON && m1.byte2 == 0 ) || ( m1.GetStatus() == NOTE_OFF ) )
       )
        return 1; // m1 is larger

    return 0;  // both are equal.
}


int  MIDITimedBigMessage::CompareEvents (
    const MIDITimedBigMessage &m1,
    const MIDITimedBigMessage &m2
)
{
    bool n1 = m1.IsNoOp();
    bool n2 = m2.IsNoOp();
    // NOP's always are larger.

    if ( n1 && n2 )
        return 0; // same, do not care.

    if ( n2 )
        return 2; // m2 is larger

    if ( n1 )
        return 1; // m1 is larger

    if ( m1.GetTime() > m2.GetTime() )
        return 1; // m1 is larger

    if ( m2.GetTime() > m1.GetTime() )
        return 2; // m2 is larger

    // if times are the same, a note off is always larger

    if ( m1.byte1 == m2.byte1
            && m1.GetStatus() == NOTE_ON
            && ( ( m2.GetStatus() == NOTE_ON && m2.byte2 == 0 ) || ( m2.GetStatus() == NOTE_OFF ) )
       )
        return 2; // m2 is larger

    if ( m1.byte1 == m2.byte1
            && m2.GetStatus() == NOTE_ON
            && ( ( m1.GetStatus() == NOTE_ON && m1.byte2 == 0 ) || ( m1.GetStatus() == NOTE_OFF ) )
       )
        return 1; // m1 is larger

    return 0;  // both are equal.
}

*/

/*
namespace jdksmidi
{









int MIDIMessage::GetLengthMSG() const
{
    if ( IsMetaEvent() ) // for all Meta Events
    {
        return data_length;
    }

    else if ( IsSystemMessage() ) // for all System Exclusive Events
    {
        return GetSystemMessageLength ( status );
    }

    else // for all Channel Events
    {
        return GetMessageLength ( status );
    }
}


unsigned long MIDIMessage::GetTempo32() const
{
    // tempo is in microseconds per beat
    unsigned long tempo = GetTempo();
    if ( tempo == 0 )
        tempo = 1;
    /*
        // calculate beats per second by
        double beats_per_second = 1e6 / ( double ) tempo;// 1 million microseconds per second
        double beats_per_minute = beats_per_second * 60.;
        unsigned long tempo_bpm_times_32 = (unsigned long) ( 0.5 + beats_per_minute * 32. );
    */  /*
    unsigned long tempo_bpm_times_32 = (unsigned long) ( 0.5 + (32*60*1e6) / ( double ) tempo );
    return tempo_bpm_times_32;
}

unsigned long MIDIMessage::GetTempo() const
{
    return MIDIFile::To32Bit ( 0, byte2, byte3, byte4 );
}

void MIDIMessage::SetBenderValue ( short v )
{
    short x = ( short ) ( v + 8192 );
    byte1 = ( unsigned char ) ( x & 0x7f );
    byte2 = ( unsigned char ) ( ( x >> 7 ) & 0x7f );
}

void MIDIMessage::SetMetaValue ( unsigned short v )
{
    byte2 = ( unsigned char ) ( v & 0xff );
    byte3 = ( unsigned char ) ( ( v >> 8 ) & 0xff );
}

void MIDIMessage::SetNoteOn ( unsigned char chan, unsigned char note, unsigned char vel )
{
    Clear();
    status = ( unsigned char ) ( chan | NOTE_ON );
    byte1 = note;
    byte2 = vel;
}



double MIDIMessage::GetPan() const
{
    int val = GetControllerValue(); // 0 = leftmost, 64 = centre, 127 = rightmost
    if (val == 127) val = 128;
    return (val-64)/64.;
}

void MIDIMessage::SetPan( unsigned char chan, double pan )
{
    //     leftmost  centre   rightmost
    //  pan = -1 ...    0 ...    +1
    // ipan =  0 ... 8192 ... 16384
    int ipan = jdks_float2int( 8192. * (pan + 1.) );
    if ( ipan > 16383 ) ipan = 16383;

    int pan_msb = ipan / 128;
//  int pan_lsb = ipan % 128;

    SetControlChange( chan, C_PAN, pan_msb );
//  Russian (windows 1251):
//  к сожалению любое pan_lsb сбрасывает панораму в центр при проигрывании midi
//  и через MediaPlayer и даже через Timidity, поэтому не делаем установку lsb
//  English:
//  unfortunately any pan_lsb drops panorama to the center with the playback of midi file
//  through MediaPlayer and even through Timidity; therefore we do not make the set of lsb
//  SetControlChange( chan, C_PAN + C_LSB, pan_lsb ); // don't work...
}


void MIDIMessage::SetPitchBend ( unsigned char chan, short val )
{
    Clear();
    status = ( unsigned char ) ( chan | PITCH_BEND );
    val += ( short ) 0x2000; // center value
    byte1 = ( unsigned char ) ( val & 0x7f ); // 7 bit bytes
    byte2 = ( unsigned char ) ( ( val >> 7 ) & 0x7f );
}

void MIDIMessage::SetPitchBend ( unsigned char chan, unsigned char low, unsigned char high )
{
    Clear();
    status = ( unsigned char ) ( chan | PITCH_BEND );
    byte1 = ( unsigned char ) ( low );
    byte2 = ( unsigned char ) ( high );
}

void MIDIMessage::SetSysEx( unsigned char type )
{
    Clear();
    status = type; // SYSEX_START or SYSEX_START_A
}

void MIDIMessage::SetMTC ( unsigned char field, unsigned char v )
{
    Clear();
    status = MTC;
    byte1 = ( unsigned char ) ( ( field << 4 ) | v );
}

void MIDIMessage::SetSongPosition ( short pos )
{
    Clear();
    status = SONG_POSITION;
    byte1 = ( unsigned char ) ( pos & 0x7f );
    byte2 = ( unsigned char ) ( ( pos >> 7 ) & 0x7f );
}

void MIDIMessage::SetSongSelect ( unsigned char sng )
{
    Clear();
    status = SONG_SELECT;
    byte1 = sng;
}

void MIDIMessage::SetTuneRequest()
{
    Clear();
    status = TUNE_REQUEST;
}

void MIDIMessage::SetMetaEvent ( unsigned char type, unsigned char v1, unsigned char v2 )
{
    Clear();
    status = META_EVENT;
    byte1 = type;
    byte2 = v1;
    byte3 = v2;
}

void MIDIMessage::SetMetaEvent ( unsigned char type, unsigned short v )
{
    unsigned char v1 = ( unsigned char ) ( v & 0xff );
    unsigned char v2 = ( unsigned char ) ( ( v >> 8 ) & 0xff );
    SetMetaEvent ( type, v1, v2 );
}

void MIDIMessage::SetAllNotesOff (unsigned char chan, unsigned char type, unsigned char mode)
{
    Clear();
    status = ( unsigned char ) ( chan | CONTROL_CHANGE );
    byte1 = type;
    byte2 = mode;
//  byte2 = 0x7f; // was
}

void MIDIMessage::SetLocal ( unsigned char chan, unsigned char v )
{
    Clear();
    status = ( unsigned char ) ( chan | CONTROL_CHANGE );
    byte1 = C_LOCAL;
    byte2 = v;
}

void MIDIMessage::SetTempo ( unsigned long tempo )
{
    int a, b, c;
    c = tempo & 0xFF;
    b = (tempo >> 8) & 0xFF;
    a = (tempo >> 16) & 0xFF;
    SetMetaEvent ( META_TEMPO, a, b );
    SetByte4( c );
}

void MIDIMessage::SetTempo32 ( unsigned long tempo_times_32 )
{
    unsigned long tempo = (unsigned long) ( 0.5 + (32*60*1e6) / ( double ) tempo_times_32 );
    SetTempo ( tempo );
}

void MIDIMessage::SetText ( unsigned short text_num, unsigned char type )
{
    SetMetaEvent ( type, text_num );
}

void MIDIMessage::SetDataEnd()
{
    SetMetaEvent ( META_END_OF_TRACK, 0 );
}


void MIDIMessage::SetKeySig ( signed char sharp_flats, unsigned char major_minor )
{
    SetMetaEvent ( META_KEYSIG, sharp_flats, major_minor );
}



MIDIBigMessage::MIDIBigMessage()
    :
    sysex ( 0 )
{
}

MIDIBigMessage::MIDIBigMessage ( const MIDIBigMessage &m )
    :
    MIDIMessage ( m ),
    sysex ( 0 )
{
    if ( m.sysex )
    {
        sysex = new MIDISystemExclusive ( *m.sysex );
    }
}

MIDIBigMessage::MIDIBigMessage ( const MIDIMessage &m )
    :
    MIDIMessage ( m ),
    sysex ( 0 )
{
}

MIDIBigMessage::MIDIBigMessage ( const MIDIMessage &m, const MIDISystemExclusive *e )
    :
    MIDIMessage ( m ),
    sysex ( 0 )
{
    CopySysEx( e );
}

void MIDIBigMessage::Clear()
{
    ClearSysEx();
    MIDIMessage::Clear();
}

//
// destructors
//

MIDIBigMessage::~MIDIBigMessage()
{
    Clear();
}

//
// operator =
//

const MIDIBigMessage &MIDIBigMessage::operator = ( const MIDIBigMessage &m )
{
    delete sysex;
    sysex = 0;
    if ( m.sysex )
        sysex = new MIDISystemExclusive ( *m.sysex );

    MIDIMessage::operator = ( m );
    return *this;
}

const MIDIBigMessage &MIDIBigMessage::operator = ( const MIDIMessage &m )
{
    delete sysex;
    sysex = 0;
    MIDIMessage::operator = ( m );
    return *this;
}



//
// 'Set' methods
//

void MIDIBigMessage::CopySysEx ( const MIDISystemExclusive *e )
{
    ClearSysEx();
    if ( e )
    {
        sysex = new MIDISystemExclusive ( *e );
    }
}

#if 0
void MIDIBigMessage::SetSysEx ( MIDISystemExclusive *e )
{
    delete sysex;
    sysex = e;
}
#endif

void MIDIBigMessage::ClearSysEx()
{
    jdks_safe_delete_object( sysex );
}






//
// Constructors
//

MIDITimedMessage::MIDITimedMessage()
    : time ( 0 )
{
}

MIDITimedMessage::MIDITimedMessage ( const MIDITimedMessage &m )
    : MIDIMessage ( m ), time ( m.GetTime() )
{
}

MIDITimedMessage::MIDITimedMessage ( const MIDIMessage &m )
    : MIDIMessage ( m ), time ( 0 )
{
}

void MIDITimedMessage::Clear()
{
    time = 0;
    MIDIMessage::Clear();
}

//
// operator =
//

const MIDITimedMessage &MIDITimedMessage::operator = ( const MIDITimedMessage & m )
{
    time = m.GetTime();
    MIDIMessage::operator = ( m );
    return *this;
}

const MIDITimedMessage &MIDITimedMessage::operator = ( const MIDIMessage & m )
{
    time = 0;
    MIDIMessage::operator = ( m );
    return *this;
}

//
// Comparison functions
//

int  MIDITimedMessage::CompareEvents (
    const MIDITimedMessage &m1,
    const MIDITimedMessage &m2
)
{
    bool n1 = m1.IsNoOp();
    bool n2 = m2.IsNoOp();
    // NOP's always are larger.

    if ( n1 && n2 )
        return 0; // same, do not care.

    if ( n2 )
        return 2; // m2 is larger

    if ( n1 )
        return 1; // m1 is larger

    if ( m1.GetTime() > m2.GetTime() )
        return 1; // m1 is larger

    if ( m2.GetTime() > m1.GetTime() )
        return 2; // m2 is larger

    // if times are the same, a note off is always larger

    if ( m1.byte1 == m2.byte1
            && m1.GetStatus() == NOTE_ON
            && ( ( m2.GetStatus() == NOTE_ON && m2.byte2 == 0 ) || ( m2.GetStatus() == NOTE_OFF ) )
       )
        return 2; // m2 is larger

    if ( m1.byte1 == m2.byte1
            && m2.GetStatus() == NOTE_ON
            && ( ( m1.GetStatus() == NOTE_ON && m1.byte2 == 0 ) || ( m1.GetStatus() == NOTE_OFF ) )
       )
        return 1; // m1 is larger

    return 0;  // both are equal.
}


//
// Constructors
//

MIDITimedBigMessage::MIDITimedBigMessage()
    : time ( 0 )
{
}

MIDITimedBigMessage::MIDITimedBigMessage ( const MIDITimedBigMessage &m )
    : MIDIBigMessage ( m ),
      time ( m.GetTime() )
{
}

MIDITimedBigMessage::MIDITimedBigMessage ( const MIDIBigMessage &m )
    : MIDIBigMessage ( m ),
      time ( 0 )
{
}

MIDITimedBigMessage::MIDITimedBigMessage ( const MIDITimedMessage &m )
    : MIDIBigMessage ( m ),
      time ( m.GetTime() )
{
}

MIDITimedBigMessage::MIDITimedBigMessage ( const MIDIMessage &m )
    : MIDIBigMessage ( m ),
      time ( 0 )
{
}

MIDITimedBigMessage::MIDITimedBigMessage ( const MIDITimedMessage &m, const MIDISystemExclusive *e )
    : MIDIBigMessage ( m, e ),
      time ( m.GetTime() )
{
}

void MIDITimedBigMessage::Clear()
{
    time = 0;
    MIDIBigMessage::Clear();
}


//
// operator =
//

const MIDITimedBigMessage &MIDITimedBigMessage::operator = ( const MIDITimedBigMessage & m )
{
    time = m.GetTime();
    MIDIBigMessage::operator = ( m );
    return *this;
}

const MIDITimedBigMessage &MIDITimedBigMessage::operator = ( const MIDITimedMessage & m )
{
    time = m.GetTime();
    MIDIBigMessage::operator = ( m );
    return *this;
}

const MIDITimedMessage &MIDITimedBigMessage::operator = (const MIDIMessage &m) {
    time = 0;
    MIDIBigMessage::operator = ( m);
    return *this;
}



int  MIDITimedMessage::CompareEvents (
    const MIDITimedMessage &m1,
    const MIDITimedMessage &m2
)
{
    bool n1 = m1.IsNoOp();
    bool n2 = m2.IsNoOp();
    // NOP's always are larger.

    if ( n1 && n2 )
        return 0; // same, do not care.

    if ( n2 )
        return 2; // m2 is larger

    if ( n1 )
        return 1; // m1 is larger

    if ( m1.GetTime() > m2.GetTime() )
        return 1; // m1 is larger

    if ( m2.GetTime() > m1.GetTime() )
        return 2; // m2 is larger

    // if times are the same, a note off is always larger

    if ( m1.byte1 == m2.byte1
            && m1.GetStatus() == NOTE_ON
            && ( ( m2.GetStatus() == NOTE_ON && m2.byte2 == 0 ) || ( m2.GetStatus() == NOTE_OFF ) )
       )
        return 2; // m2 is larger

    if ( m1.byte1 == m2.byte1
            && m2.GetStatus() == NOTE_ON
            && ( ( m1.GetStatus() == NOTE_ON && m1.byte2 == 0 ) || ( m1.GetStatus() == NOTE_OFF ) )
       )
        return 1; // m1 is larger

    return 0;  // both are equal.
}

int  MIDITimedMessage::CompareEventsForInsert (
    const MIDITimedMessage &m1,
    const MIDITimedMessage &m2
)
{
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

    n1 = m1.IsEndOfTrack();
    n2 = m2.IsEndOfTrack();
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

    n1 = m1.IsSystemExclusive();
    n2 = m2.IsSystemExclusive();
    // System exclusive are larger
    if ( n1 && n2 )
        return 0; // same
    if ( n1 )
        return 1; // m1 is larger
    if ( n2 )
        return 2; // m2 is larger

    if ( m1.IsChannelEvent() && m2.IsChannelEvent() )
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

bool  MIDITimedMessage::IsSameKind (
    const MIDITimedMessage &m1,
    const MIDITimedMessage &m2
)
{
    if (m1.IsNoOp() && m2.IsNoOp())
        return true;

    if ( m1.GetTime() != m2.GetTime() )
        return false;

    if ( m1.IsChannelMsg() && m2.IsChannelMsg() &&
         m1.GetChannel() == m2.GetChannel() )
    {
        if ( m1.GetType() != m2.GetType() )
            return false;
        if ( m1.IsNoteOn() && m2.IsNoteOn() && m1.GetNote() != m2.GetNote() )
            return false;
        if ( m1.IsNoteOff() && m2.IsNoteOff() && m1.GetNote() != m2.GetNote() )
            return false;
        if ( m1.IsControlChange() && m2.IsControlChange() && m1.GetController() != m2.GetController() )
            return false;
        return true;
    }

    if ( m1.IsMetaEvent() && m2.IsMetaEvent() )
    {
        if ( m1.GetMetaType() == m2.GetMetaType() )
            return true;
        return false;
    }

    if ( m1.GetStatus() == m2.GetStatus() )
        return true;
    return false;
}


}

*/
