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
/*
** Copyright 2016 By N. Cassetta
** myjdkmidi library
** see header for changes against jdksmidi
*/

#include "../include/filewrite.h"



MIDIFileWriteStream::MIDIFileWriteStream(const char *fname) : begin(0), del(true) {
    outfs = new std::ofstream(fname, std::ios::out | std::ios::binary);
    if (outfs->fail()) {
        delete outfs;
        outfs = 0;
    }
}


MIDIFileWriteStream::MIDIFileWriteStream(std::ostream* ofs) :
    outfs(ofs), begin(ofs->tellp()), del(false) {}


MIDIFileWriteStream::~MIDIFileWriteStream() {
    if (outfs && del)
        delete outfs;
}


long MIDIFileWriteStream::Seek(long pos, int whence) {
    std::streamoff offs = pos;

    switch (whence) {
        case SEEK_SET:
            outfs->seekp(begin+offs, std::ios_base::beg);
            break;
        case SEEK_CUR:
            outfs->seekp(offs, std::ios_base::cur);
            break;
        case SEEK_END:
            outfs->seekp(offs, std::ios_base::end);
            break;
    }
    return outfs->good();
}


int MIDIFileWriteStream::WriteChar(int c) {
    if (outfs) {
        outfs->put(c);
        return outfs->good() ? 0 : -1;
    }
    return -1;
}


bool MIDIFileWriteStream::IsValid() {
    return outfs != 0;
}











MIDIFileWriter::MIDIFileWriter(MIDIFileWriteStream *out_stream_) : out_stream(out_stream_) {
    file_length = 0;
    error = 0;
    track_length = 0;
    track_time = 0;
    running_status = 0;
    track_position = 0;
}


MIDIFileWriter::~MIDIFileWriter() {
}


void MIDIFileWriter::Error(char *s) {
    // NULL method; can override.
    error = true;
}


void MIDIFileWriter::WriteShort(unsigned short c) {
    WriteCharacter((unsigned char)((c >> 8) & 0xff));
    WriteCharacter((unsigned char)((c & 0xff)));
}


void MIDIFileWriter::Write3Char(long c) {
    WriteCharacter((unsigned char)((c >> 16) & 0xff));
    WriteCharacter((unsigned char)((c >> 8) & 0xff));
    WriteCharacter((unsigned char)((c & 0xff)));
}


void MIDIFileWriter::WriteLong(unsigned long c) {
    WriteCharacter((unsigned char)((c >> 24) & 0xff));
    WriteCharacter((unsigned char)((c >> 16) & 0xff));
    WriteCharacter((unsigned char)((c >> 8) & 0xff));
    WriteCharacter((unsigned char)((c & 0xff)));
}


void MIDIFileWriter::WriteFileHeader(int format, int ntrks, int division) {
    WriteCharacter((unsigned char) 'M');
    WriteCharacter((unsigned char) 'T');
    WriteCharacter((unsigned char) 'h');
    WriteCharacter((unsigned char) 'd');
    WriteLong(6);
    WriteShort((short)format);
    WriteShort((short)ntrks);
    WriteShort((short)division);
    file_length = 4 + 4 + 6;
}


void MIDIFileWriter::WriteTrackHeader(unsigned long length) {
    track_position = file_length;
    track_length = 0;
    track_time = 0;
    running_status = 0;

    WriteCharacter((unsigned char) 'M');
    WriteCharacter((unsigned char) 'T');
    WriteCharacter((unsigned char) 'r');
    WriteCharacter((unsigned char) 'k');

    WriteLong(length);
    file_length += 8;
    within_track = true;
}


int	MIDIFileWriter::WriteVariableNum(unsigned long n) {
    register unsigned long buffer;
    short cnt = 0;

    buffer = n & 0x7f;
    while((n >>= 7) > 0) {
        buffer <<= 8;
        buffer |= 0x80;
        buffer += (n & 0x7f);
    }

    while(true) {
      WriteCharacter((unsigned char) (buffer & 0xff));
      cnt++;
      if(buffer & 0x80)
            buffer >>= 8;
      else
            break;
    }
    return cnt;
}


void MIDIFileWriter::WriteDeltaTime(unsigned long abs_time) {
    long dtime = abs_time - track_time;
    if(dtime < 0) {
//		Error( "Events out of order" );
      dtime = 0;
    }

    IncrementCounters(WriteVariableNum(dtime));
    track_time = abs_time;
}

/* DELETE THIS FUNCTION
void MIDIFileWriter::WriteEvent(const MIDITimedMessage &m) {
    ENTER( "void    MIDIFileWriter::WriteEvent()" );

    if(m.IsNoOp())
        return;

    if(m.IsMetaEvent()) {
      // TO DO: add more meta events.

        if(m.IsTempo()) {
            unsigned long tempo = (60000000 / m.GetTempo32()) * 32;
            WriteTempo(m.GetTime(), tempo);
            return;
        }
        if(m.IsDataEnd()) {
            WriteEndOfTrack(m.GetTime());
            return;
        }
        if(m.IsKeySig()) {
            WriteKeySignature(m.GetTime(), m.GetKeySigSharpFlats(), m.GetKeySigMajorMinor());
            return;
        }
        return;	// all other marks are ignored.
    }
    else {
        short len = m.GetLength();

        WriteDeltaTime(m.GetTime());
        if(m.GetStatus() != running_status) {
            running_status = m.GetStatus();
            WriteCharacter((unsigned char) running_status);
            IncrementCounters(1);
        }
        if(len > 1) {
            WriteCharacter((unsigned char) m.GetByte1());
            IncrementCounters(1);
        }
        if(len > 2) {
            WriteCharacter((unsigned char) m.GetByte2());
            IncrementCounters(1);
        }
    }
}
*/



void MIDIFileWriter::WriteEvent(const MIDITimedMessage &msg) {
    if(msg.IsNoOp())
        return;

    if(msg.IsChannelMsg())
        WriteChannelEvent(msg);
    if(msg.IsSysEx())
        WriteSysExEvent(msg);
    if(msg.IsMetaEvent()) {
      // if this meta-event has a sysex buffer attached, this
      // buffer contains the raw midi file meta data
      // (writes text, tempo and timesig)
        if(msg.GetSysEx()) {
            WriteMetaEvent(
            msg.GetTime(),
            msg.GetMetaType(),
            msg.GetSysEx()->GetBuf(),
            msg.GetSysEx()->GetLength());
        }
        else {
        // otherwise, it is a type of meta that doesnt have
        // data...
            if(msg.IsDataEnd())
                WriteEndOfTrack(msg.GetTime());

            else {
                unsigned char buf[2];
                buf[0] = msg.GetByte2();
                buf[1] = msg.GetByte3();
                int len;
                switch (msg.GetMetaType()) {
                    case META_SEQUENCE_NUMBER:
                    case META_KEYSIG:
                        len = 2;
                        break;
                    case META_CHANNEL_PREFIX:
                    case META_OUTPUT_CABLE:
                    //case META_TRACK_LOOP: dropped
                        len = 1;
                        break;
                    default:        // this should not happen
                        len = 0;
                }
                WriteMetaEvent(
                    msg.GetTime(), msg.GetMetaType(), buf, len);
                //if(msg.IsKeySig())
                //WriteKeySignature( msg.GetTime(), msg.GetKeySigSharpFlats(), msg.GetKeySigMajorMinor() );
            }
        }
    }
}

/* OLD FUNCTION
void MIDIFileWriter::WriteEvent(const MIDITimedMessage &m) {
    if(m.IsNoOp())
        return;

    if(m.IsMetaEvent()) {
      // if this meta-event has a sysex buffer attached, this
      // buffer contains the raw midi file meta data

        if(m.GetSysEx()) {
            WriteMetaEvent(
            m.GetTime(),
            m.GetMetaType(),
            m.GetSysEx()->GetBuf(),
            m.GetSysEx()->GetLength());
        }
        else {
        // otherwise, it is a type of meta that doesnt have
        // data...
            if(m.IsTempo()) {
                unsigned long tempo = (60000000 / m.GetTempo32()) * 32;
                WriteTempo(m.GetTime(), tempo);
            }
            else if(m.IsDataEnd()) {
                WriteEndOfTrack(m.GetTime());
            }
            else if(m.IsKeySig())
                WriteKeySignature( m.GetTime(), m.GetKeySigSharpFlats(), m.GetKeySigMajorMinor() );
      }

    }
    else {
        short len = m.GetLength();

        if(m.IsSysEx() && m.GetSysEx()) {
            WriteEvent( m.GetTime(), m.GetSysEx() );
        }
        else if(len > 0) {
            WriteDeltaTime(m.GetTime());
            if(m.GetStatus() != running_status) {
                running_status = m.GetStatus();
                WriteCharacter((unsigned char)running_status);
                IncrementCounters(1);
            }
            if(len > 1) {
                WriteCharacter((unsigned char) m.GetByte1());
                IncrementCounters(1);
            }
            if(len > 2) {
                WriteCharacter((unsigned char) m.GetByte2());
                IncrementCounters(1);
            }
        }
    }
}
*/

void MIDIFileWriter::WriteChannelEvent(const MIDITimedMessage &msg) {
    short len = msg.GetLength();

    if(len > 0) {
        WriteDeltaTime(msg.GetTime());
        if(msg.GetStatus() != running_status) {
            running_status = msg.GetStatus();
            WriteCharacter((unsigned char)running_status);
            IncrementCounters(1);
        }
        if(len > 1) {
            WriteCharacter((unsigned char) msg.GetByte1());
            IncrementCounters(1);
        }
        if(len > 2) {
            WriteCharacter((unsigned char) msg.GetByte2());
            IncrementCounters(1);
        }
    }
}


void MIDIFileWriter::WriteSysExEvent(const MIDITimedMessage &msg) {
    WriteDeltaTime(msg.GetTime());

    int len = msg.GetSysEx()->GetLength();
    WriteCharacter((unsigned char)SYSEX_START);
    IncrementCounters(WriteVariableNum(len - 1));

    for(int i = 1; i < len; i++) 	// skip the initial 0xF0
        WriteCharacter((unsigned char)(msg.GetSysEx()->GetData(i)));
    IncrementCounters(len);
    running_status = 0;
}

/*   NOT USED
void MIDIFileWriter::WriteTextEvent(unsigned long time, unsigned short text_type, const char *text) {
    ENTER( "void	MIDIFileWriter::WriteEvent()" );

    WriteDeltaTime(time);

    WriteCharacter((unsigned char) 0xff);		    // META-Event
    WriteCharacter((unsigned char) text_type);	    // Text event type

    IncrementCounters(2);

    long len = strlen(text);

    IncrementCounters(WriteVariableNum(len));

    while(*text)
        WriteCharacter((unsigned char) *text++);
    IncrementCounters(len);
    running_status = 0;
}
*/

void MIDIFileWriter::WriteMetaEvent(unsigned long time, unsigned char type, const unsigned char *data, long length) {
    WriteDeltaTime(time);
    WriteCharacter((unsigned char) 0xff);   // META-Event
    WriteCharacter((unsigned char) type);	// Meta-event type

    IncrementCounters(2);

    IncrementCounters(WriteVariableNum(length));

    for(int i = 0; i < length; i++)
        WriteCharacter((unsigned char) data[i]);
    IncrementCounters(length);
    running_status = 0;
}

/*   NOT USED
void MIDIFileWriter::WriteTempo(unsigned long time, long tempo) {
    ENTER( "void	MIDIFileWriter::WriteTempo()" );

    WriteDeltaTime(time);
    WriteCharacter((unsigned char) 0xff);   // Meta-Event
    WriteCharacter((unsigned char) 0x51);   // Tempo event
    WriteCharacter((unsigned char) 0x03);	// length of event

    Write3Char(tempo);
    IncrementCounters(6);
    running_status = 0;
}


void MIDIFileWriter::WriteKeySignature(unsigned long time, char sharp_flat, char minor) {
    ENTER( "void	MIDIFileWriter::WriteKeySignature()" );

    WriteDeltaTime(time);
    WriteCharacter((unsigned char) 0xff);		// Meta-Event
    WriteCharacter((unsigned char) 0x59);		// Key Sig
    WriteCharacter((unsigned char) 0x02);		// length of event
    WriteCharacter((unsigned char) sharp_flat);	// - for flats, + for sharps
    WriteCharacter((unsigned char) minor);	    // 1 if minor key
    IncrementCounters(5);
    running_status = 0;
}


void MIDIFileWriter::WriteTimeSignature(
    unsigned long time,
    char numerator,
    char denominator_power,
    char midi_clocks_per_metronome,
    char num_32nd_per_midi_quarter_note) {
    ENTER( "void	MIDIFileWriter::WriteTimeSignature()" );

    WriteDeltaTime(time);
    WriteCharacter((unsigned char) 0xff);		// Meta-Event
    WriteCharacter((unsigned char) 0x58);		// time signature
    WriteCharacter((unsigned char) 0x04);		// length of event
    WriteCharacter((unsigned char) numerator);
    WriteCharacter((unsigned char) denominator_power);
    WriteCharacter((unsigned char) midi_clocks_per_metronome);
    WriteCharacter((unsigned char) num_32nd_per_midi_quarter_note);
    IncrementCounters(7);
    running_status = 0;
}
*/


void MIDIFileWriter::WriteEndOfTrack(unsigned long time)  {
    if(within_track == true) {
        if(time ==  0)
            time = track_time;
        WriteDeltaTime(time);
        WriteCharacter((unsigned char) 0xff);		// Meta-Event
        WriteCharacter((unsigned char) 0x2f);		// End of track
        WriteCharacter((unsigned char) 0x00);		// length of event
        IncrementCounters(3);
        within_track = false;
        running_status = 0;
    }
}


void MIDIFileWriter::RewriteTrackLength() {
    // go back and patch in the tracks length into the track chunk
    // header, now that we know the proper value.
    // then make sure we go back to the end of the file

    Seek(track_position + 4);
    WriteLong(track_length);
    Seek(track_position + 8 + track_length);
}

