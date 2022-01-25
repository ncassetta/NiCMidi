/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
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


#include "../include/filewrite.h"



MIDIFileWriter::MIDIFileWriter(std::ostream *out_stream_) : error(0), within_track(0), file_length(0), track_length(0),
    track_time(0), track_position(0), running_status(0),  out_stream(out_stream_)
{}


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
            msg.GetSysEx()->GetBuffer(),
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
            }
        }
    }
}


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
