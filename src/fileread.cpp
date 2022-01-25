/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2010 V.R.Madgazin
 *   www.vmgames.com vrm@vmgames.com
 *   Copyright (C) 2021  Nicola Cassetta
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


#include "../include/fileread.h"

#include <iostream>     // only for debug! TODO: delete this after debug

// Standard MIDI-File Format Spec. 1.1, page 9 of 18:
// "Sysex events and meta events cancel any running status which was in effect.
// Running status does not apply to and may not be used for these messages."

// TODO: decide which way is right for this flag and fix it - The standard midi file format specs are (were?) unclear
#define MIDIFRD_ALLOW_STATUS_ACROSS_META_EVENT 0 // correct value is 0 !



void MIDIFileEventHandler::ChanMessage(const MIDITimedMessage &msg) {
    switch(msg.GetStatus() & 0xf0) {
        case NOTE_OFF:
            mf_note_off(msg);
            break;
        case NOTE_ON:
            if(msg.GetVelocity() == 0)
                mf_note_off(msg);
            else
                mf_note_on(msg);
            break;
        case POLY_PRESSURE:
            mf_poly_after(msg);
            break;
        case CONTROL_CHANGE:
            if(msg.GetByte2() > C_ALL_NOTES_OFF)
                mf_system_mode(msg);
            else
                mf_control(msg);
            break;
        case PROGRAM_CHANGE:
            mf_program(msg);
            break;
        case CHANNEL_PRESSURE:
            mf_chan_after(msg);
            break;
        case PITCH_BEND:
            mf_bender(msg);
            break;
    }
}


void MIDIFileEventHandler::MetaEvent(MIDIClockTime time, int type, int leng, unsigned char* m) {
    switch  (type) {
        case META_SEQUENCE_NUMBER:
            mf_meta16(time, type, m[0], m[1]);
            break;
        case META_GENERIC_TEXT:
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

            // These are all text events

            m[leng] = 0;      // make sure string ends in NULL
            mf_text(time, type, leng, m);
            break;
        case META_CHANNEL_PREFIX:
        case META_OUTPUT_CABLE:
        //case META_TRACK_LOOP:         does it exists?
            mf_meta16(time, type, m[1], 0);
            break;
        case META_END_OF_TRACK:       // End of Track
            mf_eot(time);
            break;
        case META_TEMPO:              // Set Tempo
            mf_tempo(time, m[0], m[1], m[2]);
            break;
        case META_SMPTE:
            mf_smpte(time, m[0], m[1], m[2], m[3], m[4]);
            break;
        case META_TIMESIG:
            mf_timesig(time, m[0], m[1], m[2], m[3]);
            break;
        case META_KEYSIG: {
                char c = m[0];
                mf_keysig(time, c,m[1]);
            }
            break;
        case META_SEQUENCER_SPECIFIC:
            mf_sqspecific(time, leng, m);
            break;
        default:
            mf_metamisc(time, type, leng, m);
            break;
    }
}




MIDIFileReader::MIDIFileReader(std::istream *ist, MIDIFileEventHandler *ev_h, unsigned long max_msg_len_) :
    no_merge(0), cur_time(0), skip_init(1), to_be_read(0), cur_track(0), abort_parse(0),
    max_msg_len(max_msg_len_), msg_index(0), in_stream(ist), event_handler(ev_h) {
    the_msg = new unsigned char[max_msg_len];
}


MIDIFileReader::~MIDIFileReader() {
    delete[] the_msg;
}


void MIDIFileReader::mf_error(const char *e) {
    event_handler->mf_error(e);
    abort_parse = true;
}


bool MIDIFileReader::Parse() {
    int n;

    n = ReadHeader();
    if(n <= 0) {
        mf_error( "No Tracks" );
        return false;
    }
    for(cur_track = 0; cur_track < n; cur_track++) {
        ReadTrack();
        if( abort_parse )
            return false;
    }
    return true;
}


int MIDIFileReader::ReadMT(unsigned long type, int skip) {
    unsigned long read = 0;
    int c;

    read = EGetC()*0x1000000 + EGetC()*0x10000 + EGetC()*0x100 + EGetC();

    if(type != read) {
        if(skip) {
            do {
                read <<= 8;
                c = EGetC();
                read |= c;
                if(read == type)
                    return true;
                if(abort_parse)
                    return false;
            } while(c!=-1);
        }
        mf_error( "Error looking for chunk type" );
        return false;
    }
    else
        return true;
}


int MIDIFileReader::ReadHeader() {
    int the_format;
    int ntrks;
    int division;

    if(ReadMT(_MThd, skip_init) == 0xffff)
        return 0;

    if(abort_parse)
        return 0;

    to_be_read = Read32Bit();
    the_format = Read16Bit();
    ntrks = Read16Bit();
    division = Read16Bit();

    if(abort_parse)
        return 0;

    header.format = the_format;
    header.ntrks = ntrks;
    header.division = division;

    event_handler->mf_header(the_format, ntrks, division);

    std::cout << "MIDI File: format " << the_format <<" tracks " << ntrks
              << " division " << division << std::endl;

    while(to_be_read > 0)
        EGetC();

    return ntrks;
}

//
// read a track chunk
//

void MIDIFileReader::ReadTrack() {
    //
    // This array is indexed by the high half of a status byte.  Its
    // value is either the number of bytes needed (1 or 2) for a channel
    // message, or 0 (meaning it's not  a channel message).
    //

    static char chantype[] =
      {
        0, 0, 0, 0, 0, 0, 0, 0,         // 0x00 through 0x70
        2, 2, 2, 2, 1, 1, 2, 0          // 0x80 through 0xf0
      };

    unsigned long lookfor, lng;
    int c, c1, type;
    int sysexcontinue = 0;  // 1 if last message was unfinished sysex
    int running = 0;        // 1 when running status used
    int status = 0;         // (possible running) status byte
    int needed;

    if(ReadMT(_MTrk, 0) == 0xffff)
        return;

    to_be_read = Read32Bit();
    cur_time = 0;

    event_handler->mf_starttrack(cur_track);

    while(to_be_read > 0 && !abort_parse) {
        unsigned long deltat = ReadVariableNum();

        event_handler->UpdateTime(deltat);
        cur_time += deltat;
        c = EGetC();

        if(c == -1)
            break;
        if(sysexcontinue && c != 0xf7)
            mf_error( "Error after expected continuation of SysEx" );
        if((c & 0x80) == 0) {
            if(status == 0)
                mf_error( "Unexpected Running Status" );
            running=1;
            needed = chantype[(status >> 4) & 0xf];
        }
        else {
            // TO DO: is running status to be cleared or not when meta event happens?

#if MIDIFRD_ALLOW_STATUS_ACROSS_META_EVENT
            if(c != 0xff) {
                status = c;
                running = 0;
                needed = 0;
            }

#else
            status = c;
            running = 0;
            needed = chantype[(status >> 4) & 0xf];
#endif
        }

        if(needed) {            // ie. is it a channel message?
            if(running)
                c1 = c;
            else
                c1 = EGetC();
            FormChanMessage((unsigned char)status, (unsigned char)c1, (unsigned char)((needed > 1) ? EGetC() : 0 ));
            continue;
        }

        switch(c) {
            case 0xff:              // meta-event
                type = EGetC();
                lng = ReadVariableNum();
                lookfor = to_be_read - lng;
                MsgInit();
                while(to_be_read > lookfor)
                    MsgAdd( EGetC() );
                event_handler->MetaEvent(cur_time, type, msg_index, the_msg);
                break;

            case 0xf0:              // start of sys-ex
                lng = ReadVariableNum();
                lookfor = to_be_read - lng;
                MsgInit();
                MsgAdd(0xf0);
                while( to_be_read>lookfor )
                    MsgAdd(c = EGetC());
                if(c == 0xf7 || no_merge == 0) {
          // make a sysex object out of the raw sysex data
                    MIDISystemExclusive ex(the_msg, msg_index);

          // give the sysex object to our event handler
                    event_handler->mf_sysex( cur_time, ex );
                }
                else
                    sysexcontinue = 1; // merge into next msg
                break;
            case 0xf7:              // sysex continuation or
            // arbitary stuff
                lng = ReadVariableNum();
                lookfor = to_be_read - lng;
                if(!sysexcontinue)
                    MsgInit();
                while(to_be_read > lookfor)
                    MsgAdd(c = EGetC());
                if(!sysexcontinue)
                    event_handler->mf_arbitrary(cur_time, msg_index, the_msg);
                else if(c == 0xf7) {
          // make a sysex object out of the raw sysex data
                    MIDISystemExclusive ex(the_msg, msg_index);
                    event_handler->mf_sysex(cur_time, ex);
                    sysexcontinue = 0;
                }
                break;
            default:
                BadByte(c);
                break;
        }
    }
    event_handler->mf_endtrack(cur_track);
    return;
}


unsigned long MIDIFileReader::ReadVariableNum() {
    unsigned long value;
    int c;

    c = EGetC();
    if(c == -1) {
        return 0;
    }
    value = c;
    if(c & 0x80) {
        value &= 0x7f;
        do {
            c = EGetC();
            value = (value << 7) + (c & 0x7f);
        } while(c & 0x80);
    }
    return value;
}


unsigned long MIDIFileReader::Read32Bit() {
    int c1, c2, c3, c4;

    c1 = EGetC();
    c2 = EGetC();
    c3 = EGetC();
    c4 = EGetC();
    return (  (unsigned long)c1 << 24) + ((unsigned long)c2 << 16)
            + ((unsigned long)c3 << 8) + ((unsigned long)c4 << 0 );
}


int MIDIFileReader::Read16Bit() {
    int c1, c2;

    c1 = EGetC();
    c2 = EGetC();
    return (((unsigned short)c1 << 8) + ((unsigned short)c2 << 0));
}


int MIDIFileReader::EGetC() {
    int c;

    c = in_stream->get();

    if(!in_stream->good()) {
        mf_error( "Unexpected Stream Error" );
        abort_parse = true;
        return -1;
    }
    --to_be_read;

    return (int)c;
}


void MIDIFileReader::MsgAdd(int a) {
    if(msg_index<max_msg_len)
        the_msg[msg_index++] = (unsigned char)a;
}


void MIDIFileReader::MsgInit() {
    msg_index = 0;
}


void MIDIFileReader::BadByte(int c) {
    mf_error( "Unexpected Byte" );
    abort_parse = true;
}


void MIDIFileReader::FormChanMessage(unsigned char st, unsigned char b1, unsigned char b2) {
    MIDITimedMessage m;

    m.SetStatus(st);
    m.SetByte1(b1);
    m.SetByte2(b2);
    m.SetTime(cur_time);
    if(st >= 0x80 && st < 0xf0 )
        event_handler->ChanMessage(m);
}



