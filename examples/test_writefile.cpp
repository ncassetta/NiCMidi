/*
 *   Example file for NiCMidi - A C++ Class Library for MIDI
 *
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

/*
  A nice little example demonstrating how to edit the MIDIMultitrack
  embedded in an AdvancedSequencer, play its content and then save
  it in a MIDI file.
*/


#include <string>

#include "../include/advancedsequencer.h"
#include "../include/filewritemultitrack.h"     // for WriteMIDIFile() function

using namespace std;


// A struct holding data for a note
struct note_data {
    unsigned char   note;
    MIDIClockTime   length;
    MIDIClockTime   time;
};


//////////////////////////////////////////////////////////////////
//                        G L O B A L S                         //
//////////////////////////////////////////////////////////////////

// The default ctor creates an AdvancedSequencer with 17 empty tracks (1 master + 16 channel)
AdvancedSequencer sequencer;

// Shown on opening
const char introstring[] =
"This example creates an AdvancedSequencer object, gets its MIDIMultiTrack and edits it\n\
inserting notes and other MIDI messages.\n\
Then saves the MIDIMultiTrack into a MIDIFile\n\n";

// Data for "Twinkle twinkle", track 1
int track1_len = 14;
note_data track1[] = {
    { 60, 110, 480 }, { 60, 110, 600 }, { 67, 110, 720 }, { 67, 110, 840 }, { 69, 110, 960 }, { 69, 110, 1080 },
    { 67, 230, 1200 }, { 65, 110, 1440 }, { 65, 110, 1560 }, { 64, 110, 1680 }, { 64, 110, 1800 },
    { 62, 110, 1920 }, { 62, 110, 2040 }, { 60, 210, 2160 }
};

// Data for "Twinkle twinkle", track 2
int track2_len = 10;
note_data track2[] = {
    { 48, 210, 480 }, { 48, 210, 720 }, { 53, 110, 960 }, { 53, 110, 1080 }, { 48, 210, 1200 }, { 50, 210, 1440 },
    { 48, 210, 1680 }, { 43, 110, 1920 }, { 43, 110, 2040 }, { 48, 210, 2160 }
};

// Data for "Twinkle twinkle", track 3
int track3_len = 47;
note_data track3[] = {
    { 35, 15, 480 }, { 42, 15, 540 }, { 38, 15, 600 }, { 42, 15, 600 }, { 35, 15, 660 }, { 42, 15, 660 },
    { 35, 15, 720 }, { 42, 15, 720}, { 42, 15, 780 }, { 38, 15, 840 }, { 42, 15, 840}, { 42, 15, 900 },
    { 35, 15, 960 }, { 42, 15, 1020 }, { 38, 15, 1080 }, { 42, 15, 1080 }, { 35, 15, 1140 }, { 42, 15, 1140},
    { 35, 15, 1200 }, { 42, 15, 1200}, { 42, 15, 1260 }, { 38, 15, 1320 }, { 42, 15, 1320 }, { 46, 15, 1380 },
    { 35, 15, 1440 }, { 42, 15, 1500 }, { 38, 15, 1560 }, { 42, 15, 1560 }, { 35, 15, 1620 }, { 42, 15, 1620 },
    { 35, 15, 1680 }, { 42, 15, 1680 }, { 42, 15, 1740 }, { 38, 15, 1800 }, { 42, 15, 1800 }, { 42, 15, 1860 },
    { 35, 15, 1920 }, { 42, 15, 1980 }, { 38, 15, 2040 }, { 42, 15, 2040 }, { 35, 15, 2100 }, { 42, 15, 2100 },
    { 35, 15, 2160 }, { 42, 15, 2160 }, { 42, 15, 2220 }, { 35, 15, 2280 }, { 49, 240, 2280 }
};



//////////////////////////////////////////////////////////////////
//                              M A I N                         //
//////////////////////////////////////////////////////////////////


int main( int argc, char **argv ) {
    // gets the address of the sequencer MIDIMultiTrack, so we can edit it
    MIDIMultiTrack* tracks = sequencer.GetMultiTrack();
    MIDITrack* trk;
    // the constructor creates an undefined (NoOp) message with time 0
    MIDITimedMessage msg;

    cout << introstring;

    // this is a joke: the multitrack is now empty
    cout << "\n\nNow the MultiTrack is empty: press <ENTER> to play nothing :-) ...";
    cin.get();
    cout << "Playing ...";
    sequencer.Play();
    while (sequencer.IsPlaying())
        MIDITimer::Wait(50);
    cout << "    Stop!\n";

    // now trk points to the master track (track 0 of the multitrack)
    trk = tracks->GetTrack(0);

    msg.SetTimeSig(4, 4);           // remember that the message time is 0
    trk->InsertEvent(msg);          // inserts the time signature (4/4)
    msg.SetKeySig(0, 0);
    trk->InsertEvent(msg);          // inserts the key signature (CM)
    msg.SetTempo(100.0);
    trk->InsertEvent(msg);          // inserts the tempo

    // now trk points to track 1, we'll use it for MIDI channel 1
    trk = tracks->GetTrack(1);
    unsigned char channel = 0;      // MIDI channel 1

    msg.SetProgramChange(channel, 11);
    trk->InsertEvent(msg);          // inserts the program change at time 0
    msg.SetVolumeChange(channel, 110);
    trk->InsertEvent(msg);          // inserts the track volume
    // and now inserts all the notes, taking them from our track1[] array
    for(int i = 0; i < track1_len; i++) {
        msg.SetNoteOn(channel, track1[i].note, 100);
        msg.SetTime(track1[i].time);
        trk->InsertNote(msg, track1[i].length);
    }

    // When you edit the AdvancedSequencer multitrack you must update the
    // sequencer parameters before playing: this does the job
    sequencer.UpdateStatus();

    // now we can play track 1 only
    cout << "\n\nNow track 1 has notes: press <ENTER> to play ...";
    cin.get();
    cout << "Playing ...";
    sequencer.Play();
    while (sequencer.IsPlaying())
        MIDITimer::Wait(50);
    cout << "    Stop!\n";

    // now do the same for track 2 (bass, channel 2)
    trk = tracks->GetTrack(2);
    channel = 1;

    msg.Clear();                    // resets the message
    msg.SetProgramChange(channel, 33);
    trk->InsertEvent(msg);
    msg.SetVolumeChange(channel, 90);
    trk->InsertEvent(msg);
    for(int i = 0; i < track2_len; i++) {
        msg.SetNoteOn(channel, track2[i].note, 100);
        msg.SetTime(track2[i].time);
        trk->InsertNote(msg, track2[i].length);
    }

    sequencer.UpdateStatus();
    sequencer.GoToZero();           // rewinds the sequencer

    cout << "\n\nNow track 2 also has notes: press <ENTER> to play ...";
    cin.get();
    cout << "Playing ...";
    sequencer.Play();
    while (sequencer.IsPlaying())
        MIDITimer::Wait(50);
    cout << "    Stop!\n";

    // ... and 3 (percussion, channel 10)
    trk = tracks->GetTrack(3);
    channel = 9;

    msg.Clear();
    //msg.SetProgramChange(channel, 33);    // uncomment these if your device doesn't sets
    //trk->InsertEvent(msg);                // automatically the drums on channel 10
    msg.SetVolumeChange(channel, 120);
    trk->InsertEvent(msg);
    for(int i = 0; i < track3_len; i++) {
        msg.SetNoteOn(channel, track3[i].note, 100);
        msg.SetTime(track3[i].time);
        trk->InsertNote(msg, track3[i].length);
    }

    sequencer.UpdateStatus();
    sequencer.GoToZero();

    cout << "\n\nNow there is also the drumset: press <ENTER> to play ...";
    cin.get();
    cout << "Playing ...";
    sequencer.Play();
    while (sequencer.IsPlaying())
        MIDITimer::Wait(50);
    cout << "    Stop\n";

    cout << "Enter \"y\" if you want to save the file, something else to exit";
    char ch;
    cin >> ch;

    if (tolower(ch) == 'y') {
        cout << "\n\nNow choose the name of your MIDI file (type the .mid too) ...\n";
        char filename[1024];
        cin >> filename;
        cout << "... and the MIDI file format (0 = All in one track, 1 = Separate tracks)\n";
        int format;
        cin >> format;

        // the last parameter strips out (in case of MIDI format 1) the unused tracks
        if (WriteMIDIFile(filename, format, tracks, true))
            cout << "MIDI file " << filename << " saved\n";
        else
            cout << "Error writing the file\n";
    }

    // Try to load the file you have created into your favourite sequencer!

    return EXIT_SUCCESS;
}
