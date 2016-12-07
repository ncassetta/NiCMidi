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
** Copyright 2016 By N. Cassetta
** myjdkmidi library
** see header for changes against jdksmidi
** avoided a warning
*/

#include "../include/filewritemultitrack.h"



MIDIFileWriteMultiTrack::MIDIFileWriteMultiTrack(const MIDIMultiTrack *mlt_, MIDIFileWriteStream *strm_) :
    multitrack( mlt_ ), writer( strm_ ) {}


MIDIFileWriteMultiTrack::~MIDIFileWriteMultiTrack() {}


bool MIDIFileWriteMultiTrack::Write(int num_tracks, int division) {
    bool f = true;

    if(!PreWrite())
        return false;

    // first, write the header.
    writer.WriteFileHeader(num_tracks > 1 ? 1 : 0, num_tracks, division);

    // now write each track
    for(int i = 0; i < num_tracks; ++i) {
        if (writer.ErrorOccurred()) {
            f = false;
            break;
        }
        const MIDITrack *t = multitrack->GetTrack(i);
        writer.WriteTrackHeader(0);     // will be rewritten later
        if (t) {
            for(unsigned int event_num = 0; event_num < t->GetNumEvents(); ++event_num) {
                const MIDITimedMessage *ev = t->GetEventAddress(event_num);
                if(ev && !ev->IsNoOp()) {
                    if (!ev->IsDataEnd()) {
                        writer.WriteEvent(*ev);
                        if(writer.ErrorOccurred()) {
                            f = false;
                            break;
                        }
                    }
                }
            }
        }
        writer.WriteEndOfTrack(0);
        writer.RewriteTrackLength();
    }

    if(!PostWrite())
        return false;

    return f;
}


bool MIDIFileWriteMultiTrack::PreWrite() {
    return true;
}


bool MIDIFileWriteMultiTrack::PostWrite() {
    return true;
}



bool WriteMIDIFile(const char* filename, int format, const MIDIMultiTrack* tracks, bool strip) {
    MIDIMultiTrack tmp_tracks(1, tracks->GetClksPerBeat());
    switch (format) {
        case 0: {
            MIDIClockTime max_end_time = 0;
            const MIDITrack* cur_track;
            for (unsigned int i = 0; i < tracks->GetNumTracks(); i++) {
                cur_track = tracks->GetTrack(i);
                if (cur_track->GetEndTime() > max_end_time)
                    max_end_time = cur_track->GetEndTime();
                for (unsigned int j = 0; j < cur_track->GetNumEvents(); j++)
                    tmp_tracks.InsertEvent (0, cur_track->GetEvent(j), INSMODE_INSERT);
            }
            tmp_tracks.GetTrack(0)->SetEndTime(max_end_time);
            break;
        }
        case 1:
            tmp_tracks = *tracks;
            if (strip) {
                for (unsigned int i = 1; i < tmp_tracks.GetNumTracks(); i++) {
                    if (tmp_tracks.GetTrack(i)->IsEmpty()) {
                        tmp_tracks.DeleteTrack(i);
                        i--;
                    }
                }
            }
            break;
        default:
            return false;       // TODO: allow format 2 3
    }
    MIDIFileWriteStream write_stream (filename);
    if (!write_stream.IsValid())
        return false;
    MIDIFileWriteMultiTrack writer (&tmp_tracks, &write_stream);
    return writer.Write(tmp_tracks.GetNumTracks(), tmp_tracks.GetClksPerBeat());
}


bool WriteMIDIFile(const std::string& filename, int format, const MIDIMultiTrack* tracks, bool strip) {
    return WriteMIDIFile(filename.c_str(), format, tracks, strip);
}
