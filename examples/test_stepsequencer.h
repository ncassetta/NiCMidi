/*
 * Example using the classes MIDITrack and MIDIMultiTrack for
 * libJDKSmidi C++ MIDI Library.
 * A simple step sequencer: you can add, remove, edit MIDI
 * events and play and save your file (console app, no GUI!)
 *
 * Copyright (C) 2014-2020 N.Cassetta
 * ncassetta@tiscali.it
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 * if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


/* This is a very basic, and not comfortable, step sequencer, made for demonstrating
   editing capabilities of the jdksmidi library. It creates an AdvancedSequencer class instance,
   gets it MultiTrack, and allows the user to edit it.
   You can load and save MIDI files, play them, view the file content, edit the file.
   You can insert, delete or change these MIDI events: note, control (in particular volume and pan)
   patch and tempo. For changing an event, insert a new event (same note, control, patch, tempo) at
   same time position.
*/


#ifndef TEST_STEPSEQUENCER_H_INCLUDED
#define TEST_STEPSEQUENCER_H_INCLUDED

#include "../include/multitrack.h"


static const char helpstring1[] =
"\nGeneral commands:\n\
   load filename       : Loads a file into the sequencer\n\
   save [filename]     : Saves the file\n\
   play                : Starts playback from current time\n\
   stop                : Stops playback\n\
   rew                 : Rewinds to the beginning of the file\n\
   goto meas [beat]    : Moves current time to given meas and beat\n\
                         (numbered from 1)\n\
   dump [trk]          : Prints a dump of all midi events in the file\n\
                         (or in the track trk)\n\
   notify on/off       : Sets events notifying during playback on or off\n\
   < [n]               : Moves current time n steps backward\n\
                         (if omitted, one step)\n\
   > [n]               : Moves current time n steps forward (as above)\n\
   t<                  : Moves insert position to previous track\n\
   t>                  : Moves insert position to next track\n\
   step sss            : Sets the step length in MIDI clocks\n\
   help                : Prints this help screen\n\
   quit                : Exits\n";

static const char helpstring2[] =
"\nEvent commands:\n\
   note nn [len vel]   : Inserts a note event: nn note name, len length, vel velocity\n\
                         (remembers last note len and vel, so you can omit\n\
                          them, or only vel, if they are the same)\n\
   volume val          : Inserts a volume event at current position\n\
   pan val             : Inserts a pan event at current position\n\
   control nn val      : Inserts a control nn event at current position\n\
   program val         : Inserts a program event at current position\n\
   tempo val           : Inserts a tempo event at current position\n\
   time num den        : Inserts a timesig event at current position\n\
   note nn *, volume *, etc... (followed by an asterisk)\n\
                       : Deletes the event (event must be at current time and track)\n\
   Note names must be introduced as C5 (middle C), a#3, Bb6, etc.\n\
   (the note name can be lower or upper case)\n";


static const char helpstring3[] =
"\nEdit commands:\n\
   bb [tr1]            : Marks block begin. You can specify a track number (only this\n\
                         track wil be copied); if you omit it all tracks are copied\n\
   be                  : Marks block end\n\
   bcopy               : Copies the currently marked block into memory\n\
   bclear              : Clears all events in the currently marked block\n\
   bcut                : Deletes the currently marked block, shifting all subsequent\n\
                         events. This has no effect if a single track is marked\n\
   bpaste num [o]      : Pastes the currently marked block at current position, starting\n\
                         from track num\n\
                         (if you specify \"o\" overwrites events, otherwise inserts them)\n\
   \n\
   NOTE: when playing, the sequencer notifier will print beat messages,\n\
   messing up the program input prompt. You can turn it off with the ""notify""\n\
   command. However you can still type your commands during playback.\n\n";


class position {
public:

    position ( MIDIMultiTrack* t ) :
        time( 0 ), track( 1 ), step ( t->GetClksPerBeat() ), tracks( t ) {}
    MIDIClockTime gettime() const { return time; }
    void settime(MIDIClockTime t) { time = t; }
    int gettrack() const { return track; }
    void setstep(MIDIClockTime s) { step = s;}
    MIDIClockTime getstep() const { return step; }
    void rewind()   { time = 0; }
    void stepforward()  { time += step; }
    void stepback() { time = ( time > step ? time - step : 0 ); }
    void previoustrack()    { track = (track > 1 ? track - 1 : 1); }
    void nexttrack()    { track = (track < tracks->GetNumTracks()-1 ? track+1 : track); }

private:
    MIDIClockTime time;
    unsigned int track;
    MIDIClockTime step;
    MIDIMultiTrack* tracks;
};


struct edit_block {
    edit_block() : time_begin(0), time_end(0), track_begin(0), track_end(0) {}
    bool is_empty() { return time_begin == time_end; }
    MIDIClockTime time_begin;
    MIDIClockTime time_end;
    unsigned int track_begin;
    unsigned int track_end;
};

#endif // TEST_STEPSEQUENCER_H_INCLUDED
