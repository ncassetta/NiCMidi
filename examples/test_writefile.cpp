//
// Copyright (C) 2016 N. Cassetta
// ncassetta@tiscali.it
//

#include <string>

#include "../include/advancedsequencer.h"
#include "../include/filewritemultitrack.h"


using namespace std;

struct note_data {
    unsigned char   note;
    MIDIClockTime   length;
    MIDIClockTime   time;
};


int track1_len = 14;
note_data track1[] = {
    { 60, 110, 480 }, { 60, 110, 600 }, { 67, 110, 720 }, { 67, 110, 840 }, { 69, 110, 960 }, { 69, 110, 1080 },
    { 67, 230, 1200 }, { 65, 110, 1440 }, { 65, 110, 1560 }, { 64, 110, 1680 }, { 64, 110, 1800 },
    { 62, 110, 1920 }, { 62, 110, 2040 }, { 60, 210, 2160 }
};

int track2_len = 10;
note_data track2[] = {
    { 48, 210, 480 }, { 48, 210, 720 }, { 53, 110, 960 }, { 53, 110, 1080 }, { 48, 210, 1200 }, { 50, 210, 1440 },
    { 48, 210, 1680 }, { 43, 110, 1920 }, { 43, 110, 2040 }, { 48, 210, 2160 }
};

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








//
// globals
//

AdvancedSequencer sequencer;                // an AdvancedSequencer (without GUI notifier)

const char introstring[] =
"This example creates an AdvancedSequencer object, gets its MIDIMultiTrack and edits it\n\
inserting notes and other MIDI messages.\n\
Then saves the MIDIMultiTrack into a MIDIFile\n\n";


int main( int argc, char **argv ) {

    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    MIDIMultiTrack* tracks = sequencer.GetMultiTrack();
    MIDITrack* trk;
    MIDITimedMessage msg;

    cout << introstring;

    cout << "\n\nNow the MultiTrack is empty: press <ENTER> to play nothing :-) ...";
    cin.get();
    cout << "Playing ...\n";
    sequencer.Play();
    while (sequencer.IsPlaying())
        MIDITimer::Wait(50);
    cout << "Stop\n";

    trk = tracks->GetTrack(0);

    msg.SetTimeSig(4, 4);
    trk->InsertEvent(msg);
    msg.SetKeySig(0, 0);
    trk->InsertEvent(msg);
    msg.SetTempo(100.0);
    trk->InsertEvent(msg);

    trk = tracks->GetTrack(1);
    unsigned char channel = 0;

    msg.SetProgramChange(channel, 11);
    trk->InsertEvent(msg);
    msg.SetVolumeChange(channel, 110);
    trk->InsertEvent(msg);
    for(int i = 0; i < track1_len; i++) {
        msg.SetNoteOn(channel, track1[i].note, 100);
        msg.SetTime(track1[i].time);
        trk->InsertNote(msg, track1[i].length);
    }

    sequencer.SetChanged();

    cout << "\n\nNow track 1 has notes: press <ENTER> to play ...";
    cin.get();
    cout << "Playing ...\n";
    sequencer.Play();
    while (sequencer.IsPlaying())
        MIDITimer::Wait(50);
    cout << "Stop\n";

    trk = tracks->GetTrack(2);
    channel = 1;

    msg.Clear();
    msg.SetProgramChange(channel, 33);
    trk->InsertEvent(msg);
    msg.SetVolumeChange(channel, 90);
    trk->InsertEvent(msg);
    for(int i = 0; i < track2_len; i++) {
        msg.SetNoteOn(channel, track2[i].note, 100);
        msg.SetTime(track2[i].time);
        trk->InsertNote(msg, track2[i].length);
    }

    sequencer.SetChanged();
    sequencer.GoToZero();

    cout << "\n\nNow track 2 also has notes: press <ENTER> to play ...";
    cin.get();
    cout << "Playing ...\n";
    sequencer.Play();
    while (sequencer.IsPlaying())
        MIDITimer::Wait(50);
    cout << "Stop\n";

    trk = tracks->GetTrack(3);
    channel = 9;

    msg.Clear();
    //msg.SetProgramChange(channel, 33);
    //trk->InsertEvent(msg);
    msg.SetVolumeChange(channel, 120);
    trk->InsertEvent(msg);
    for(int i = 0; i < track3_len; i++) {
        msg.SetNoteOn(channel, track3[i].note, 100);
        msg.SetTime(track3[i].time);
        trk->InsertNote(msg, track3[i].length);
    }

    sequencer.SetChanged();
    sequencer.GoToZero();

    cout << "\n\nNow there is also the drumset: press <ENTER> to play ...";
    cin.get();
    cout << "Playing ...\n";
    sequencer.Play();
    while (sequencer.IsPlaying())
        MIDITimer::Wait(50);
    cout << "Stop\n";

    cout << "\n\nNow choose the name of your MIDI file (type the .mid too) ...\n";
    char filename[1024];
    cin >> filename;
    cout << "... and the MIDI file format (0 = All in one track, 1 = Separate tracks)\n";
    int format;
    cin >> format;

    if (WriteMIDIFile(filename, format, tracks, true))
        cout << "MIDI file " << filename << " saved\n";
    else
        cout << "Error writing the file\n";

    CoUninitialize();
    return EXIT_SUCCESS;
}
