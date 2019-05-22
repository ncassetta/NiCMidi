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

#include "../include/filereadmultitrack.h"




MIDIFileReadMultiTrack::MIDIFileReadMultiTrack (MIDIMultiTrack *mlttrk) :
    multitrack(mlttrk), cur_track(-1) {}



// TODO: modified when I modified MIDI Messages names: is it right?
void MIDIFileReadMultiTrack::mf_sysex(MIDIClockTime time, const MIDISystemExclusive &ex) {
    MIDITimedMessage msg;

    msg.SetSysEx(&ex);
    msg.SetTime(time);

    multitrack->InsertEvent(cur_track, msg, INSMODE_INSERT);
}


void MIDIFileReadMultiTrack::mf_arbitrary(MIDIClockTime time, int len, unsigned char *data) {
    // ignore arbitrary byte strings
}


void MIDIFileReadMultiTrack::mf_metamisc(MIDIClockTime time, int a, int b, unsigned char *ch) {
    // ignore miscellaneous meta events
}


// This works for all meta events with no sysex attached
void MIDIFileReadMultiTrack::mf_meta16( MIDIClockTime time, int type, int b1, int b2 ) {
    // ignore sequence number events NO! now it reads them
    MIDITimedMessage msg;

    msg.SetMetaEvent(type, b1, b2);
    msg.SetTime(time);
    multitrack->InsertEvent(cur_track, msg, INSMODE_INSERT);
}


void MIDIFileReadMultiTrack::mf_smpte( MIDIClockTime time, int h, int m, int s, int f, int sf ) {
    // ignore smpte events NO! now it reads them
    MIDITimedMessage msg;

    msg.SetSMPTEOffset(h, m, s, f, sf);
    msg.SetTime(time);
    multitrack->InsertEvent(cur_track, msg, INSMODE_INSERT);
}


void MIDIFileReadMultiTrack::mf_timesig(MIDIClockTime time, int num, int denom_power,
                                        int clks_per_metro, int notated_32nd_per_quarter) {
    MIDITimedMessage msg;

    int denom= 1 << denom_power;

    msg.SetTimeSig((unsigned char)num, (unsigned char)denom,
                   (unsigned char)clks_per_metro, (unsigned char)notated_32nd_per_quarter);
    msg.SetTime(time);
    multitrack->InsertEvent(cur_track, msg, INSMODE_INSERT);
}


void MIDIFileReadMultiTrack::mf_tempo(MIDIClockTime time, int m1, int m2, int m3) {
    MIDITimedMessage msg;
    msg.SetMetaEvent(META_TEMPO, 0);
    msg.AllocateSysEx(3);
    msg.GetSysEx()->PutSysByte(m1);
    msg.GetSysEx()->PutSysByte(m2);
    msg.GetSysEx()->PutSysByte(m3);
    //msg.SetTempo32( static_cast<unsigned short>(tempo_bpm_times_32) );
    msg.SetTime(time);

    multitrack->InsertEvent(cur_track, msg, INSMODE_INSERT);
  }


void MIDIFileReadMultiTrack::mf_keysig(MIDIClockTime time, int c, int v ) {
    MIDITimedMessage msg;

    msg.SetKeySig( (unsigned char)c, (unsigned char)v );
    msg.SetTime( time );

    multitrack->InsertEvent(cur_track, msg, INSMODE_INSERT);
}


void MIDIFileReadMultiTrack::mf_sqspecific( MIDIClockTime time, int, unsigned char * ) {
    // ignore any sequencer specific messages
}


void MIDIFileReadMultiTrack::mf_text( MIDIClockTime time, int type, int len, unsigned char *s ) {
    MIDITimedMessage msg;

    msg.SetStatus(META_EVENT);
    msg.SetMetaType((unsigned char)type); // remember - MF_*_TEXT* id codes match META_*_TEXT codes
    msg.SetTime(time);
    msg.AllocateSysEx(len);

    for( int i=0; i<len; ++i )
        msg.GetSysEx()->PutSysByte( s[i] );

    multitrack->InsertEvent(cur_track, msg, INSMODE_INSERT);
}


void MIDIFileReadMultiTrack::mf_eot( MIDIClockTime time ){
    // modified by me for using SetEndTime function
    if( cur_track != -1 && cur_track < (signed)multitrack->GetNumTracks() ) {
        MIDITrack *t = multitrack->GetTrack( cur_track );

        if( t )
            t->SetEndTime(time);
    }
}


void MIDIFileReadMultiTrack::mf_error(const char* err) {}


void MIDIFileReadMultiTrack::mf_starttrack (int trk) {
    cur_track = trk;
}


void MIDIFileReadMultiTrack::mf_endtrack (int trk) {
    cur_track = -1;
}


void MIDIFileReadMultiTrack::mf_header (int format_, int ntrks_, int division_) {
    header.format = format_;
    header.ntrks = ntrks_;
    header.division = division_;

    multitrack->SetClksPerBeat( header.division );
    if (header.format == 0)                        // this is modified by me
        // split format 0 files into separate tracks, one for each channel,
        multitrack->Clear(17);         //
    else
        multitrack->Clear(header.ntrks); //
}


void MIDIFileReadMultiTrack::ChanMessage( const MIDITimedMessage &msg ) {
    if( header.format == 0 || cur_track==0 ) {
        // split format 0 files into separate tracks, one for each channel,
        // keep track 0 for tempo and meta-events

        multitrack->InsertEvent(msg.GetChannel() + 1, msg, INSMODE_INSERT);
    }
    else
        multitrack->InsertEvent(cur_track, msg, INSMODE_INSERT);
}


MIDIFileHeader& GetMIDIFileHeader(const char* filename) {
    static MIDIFileHeader header;
    std::ifstream read_stream (filename, std::ios::in | std::ios::binary);

    header = MIDIFileHeader();
    if (read_stream.fail())
        return header;

    MIDIFileReader reader (&read_stream, 0);        // no need for a EventHandler
    if (reader.ReadHeader()) {
        header.format = reader.GetFormat();
        header.ntrks = reader.GetNumberTracks();
        header.division = reader.GetDivision();
        header.filename = filename;
    }
    return header;
}


MIDIFileHeader& GetMIDIFileHeader(const std::string& filename) {
    return GetMIDIFileHeader(filename.c_str());
}


bool LoadMIDIFile(const char* filename, MIDIMultiTrack* tracks, MIDIFileHeader* const h) {
    std::ifstream read_stream (filename, std::ios::in | std::ios::binary);
    if (read_stream.fail())
        return false;

    MIDIFileReadMultiTrack track_loader (tracks);
    MIDIFileReader reader (&read_stream, &track_loader);
    bool ret = reader.Parse();
    if (ret && h && reader.ReadHeader()) {
        h->format = reader.GetFormat();
        h->ntrks = reader.GetNumberTracks();
        h->division = reader.GetDivision();
        h->filename = filename;
    }
    return ret;

}


bool LoadMIDIFile(const std::string& filename, MIDIMultiTrack* tracks, MIDIFileHeader* const h) {
    return LoadMIDIFile(filename.c_str(), tracks, h);
}

