/*
 * A simple step sequencer: you can
 * - Add, remove, edit MIDI events;
 * - Cut, copy and paste measures
 * - Play and save your file
 * (console app, no GUI!)
 *
 * Copyright (C) 2014 N.Cassetta
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
   gets it MultiTrack, and allow the user to edit it.
   You can load and save MIDI files, play them, view the file content, edit the file.
   You can insert, delete or change these MIDI events: note, control (in particular volume and pan)
   patch and tempo. For changing an event, insert a new event (same note, control, patch, tempo) at
   same time position.
*/


#include "test_stepsequencer.h"

#include <iostream>

using namespace std;


//
// globals
//

string command_buf, command, par1, par2, par3;      // used by GetCommand() for parsing the user input
MIDISequencerGUINotifierText notifier;              // a text notifier: send messages to std::cout (default)
AdvancedSequencer sequencer(&notifier);             // an AdvancedSequencer
MIDIMultiTrack* multitrack = sequencer.GetMultiTrack();
                                                    // our multitrack which will be edited
position cur_pos(multitrack);                       // the cursor position
edit_block cur_block;                               // used for cut, copy, paste
MIDIEditMultiTrack edit_multitrack;                 // used for cut, copy, paste
char filename[200];                                 // our filename


void GetCommand() {
// gets from the user the string command_buf, then parses it
// dividing it into command, par1, par2 and par3 substrings

    size_t pos1, pos2;

    cout << "\n=> ";
    getline(cin, command_buf);

    command = "";
    par1 = "";
    par2 = "";
    par3 = "";

    for (size_t i = 0; i < command_buf.size(); ++i)
        command_buf[i] = tolower( command_buf[i]);

    pos1 = command_buf.find_first_not_of ( " ");
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if (pos1 == string::npos)
        return;

    command = command_buf.substr (pos1, pos2 - pos1);
    pos1 = command_buf.find_first_not_of (" ", pos2);
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if ( pos1 == string::npos )
        return;

    par1 = command_buf.substr (pos1, pos2 - pos1);
    pos1 = command_buf.find_first_not_of (" ", pos2);
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if (pos1 == string::npos)
        return;

    par2 = command_buf.substr (pos1, pos2 - pos1);
    pos1 = command_buf.find_first_not_of ( " ", pos2);
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if (pos1 == string::npos)
        return;
    par3 = command_buf.substr (pos1, pos2 - pos1);
}


void DumpMIDIMultiTrackWithPauses (MIDIMultiTrack *mlt) {
    MIDIMultiTrackIterator iter (mlt);
    MIDITimedMessage *msg;
    int trk_num;
    int num_lines = 0;

    printf ("DUMP OF MIDI MULTITRACK\n");
    printf ("Clocks per beat: %d\n\n", mlt->GetClksPerBeat() );

    iter.GoToTime (0);

    while (iter.GetNextEvent (&trk_num, &msg)) {
        printf ("Tr %2d - ", trk_num);
        DumpMIDITimedMessage (msg);
        num_lines++;
        if (num_lines == 80) {
            printf ("Press <ENTER> to continue or q + <ENTER> to exit ...\n");
            char ch = std::cin.get();
            if (tolower(ch) == 'q')
                return;
            num_lines = 0;
        }
    }
}


void DumpMIDITrackWithPauses (MIDITrack *trk, int trk_num) {
    int num_lines = 0;

    printf ("DUMP OF MIDI TRACK %d\n", trk_num);

    for (unsigned int ev_num = 0; ev_num < trk->GetNumEvents(); ev_num++) {
        DumpMIDITimedMessage (trk->GetEventAddress(ev_num));
        num_lines++;
        if (num_lines == 80) {
            printf ("Press <ENTER> to continue or q + <ENTER> to exit ...\n");
            char ch = std::cin.get();
            if (tolower(ch) == 'q')
                return;
            num_lines = 0;
        }
    }
}


unsigned char NameToValue( string s ) {
// Converts a string as "C6" or "a#5" into corresponding MIDI note value

    static const unsigned char noteoffsets[7] = { 9, 11, 0, 2, 4, 5, 7,};

    int p = s.find_first_not_of(" ");
    char ch = tolower(s[p]);
    unsigned char note;
    unsigned char octave;
    if (string("abcdefg").find(ch) == string::npos)
        return 0;
    note = noteoffsets[ch - 'a'];
    p = s.find_first_not_of(" ", p + 1);
    if( s[p] == '#') {
        note++;
        p = s.find_first_not_of(" ", p+1);
    }
    else if (s[p] == 'b') {
        note--;
        p = s.find_first_not_of(" ", p + 1);
    }
    if (string("0123456789").find(s[p]) == string::npos)
        return 0;
    octave = s[p] - '0';
    return 12 * octave + note;
}


void PrintResolution() {
// prints info about multitrack and step size
    cout << "MultiTrack resolution is " << multitrack->GetClksPerBeat() << " clocks per beat" << endl;
    cout << "Current step size is " << cur_pos.getstep() << " clocks" << endl;
}


void PrintCurrentStatus() {
// prints info about current edited track and position
    cout << "*** Current cursor pos: Track: " << cur_pos.gettrack() << " Time: " << cur_pos.gettime() << " (" <<
        sequencer.GetCurrentMeasure() + 1 <<":" << sequencer.GetCurrentBeat() + 1;
    if (sequencer.GetCurrentBeatOffset() > 0)
        cout << ":" << sequencer.GetCurrentBeatOffset();
    cout << ")" << endl;
}



////////////////////////////////////////////////////////////
/////                       M A I N                    /////
////////////////////////////////////////////////////////////

int main(int argc, char **argv) {
    MIDITimedMessage msg;
    MIDITrack* trk = multitrack->GetTrack(cur_pos.gettrack());
    MIDIClockTime last_note_length = 120;
    int last_note_vel = 100;
    int event_num;

    *filename = 0;
    multitrack->SetClksPerBeat(120);
    cur_pos.setstep (120);
    cout << "Step sequencer example for jdksmidi library" << endl <<
            "Copyright 2014 - 2017 Nicola Cassetta" << endl << endl;
    PrintResolution();
    cout << endl << "TYPE help TO GET A LIST OF AVAILABLE COMMANDS" << endl << endl;

    // The main loop gets a command from the user and then execute it by mean of the AdvancedSequencer and
    // MIDIMultiTrack methods

    while (command != "quit") {                     // main loop
        PrintCurrentStatus();
        GetCommand();                               // gets user input and parse it

        if(command == "")                           // empty command
            continue;
        if(command == "load") {                     // loads a file
            if (sequencer.Load(par1.c_str())) {
                cout << "Loaded file " << par1 << endl;
                PrintResolution();
                strcpy (filename, par1.c_str());
            }
            else
                cout << "Error loading file" << endl;
        }

        else if (command == "save"){                // saves current file
            if (par1.length() > 0)
                strcpy (filename, par1.c_str() );
            if (strlen(filename ) == 0)
                cout << "File name not defined" << endl;
            else {
                if (WriteMIDIFile(filename, 1, multitrack))
                    cout << "File saved" << endl;
                else
                    cout << "Error writing file" << endl;
            }
        }

        else if (command == "play") {               // starts playback
            if (sequencer.IsLoaded())
                sequencer.Play();
            else
                cout << "No content in the multitrack!" << endl;
        }

        else if (command == "stop")                 // stops playback
            sequencer.Stop();

        else if ( command == "rew") {               // rewind
            cur_pos.rewind();
            sequencer.GoToZero();
        }

        else if (command == "goto") {               // goes to meas and beat
            int measure = atoi(par1.c_str()) - 1;
            int beat = (par2.length() == 0 ? 0 : atoi(par2.c_str()) - 1);
            if (measure < 0 || measure > sequencer.GetNumMeasures() - 1)
                cout << "Invalid position" << endl;
            else {
                sequencer.GoToMeasure(measure, beat);
                cur_pos.settime(sequencer.GetCurrentMIDIClockTime());
            }
        }

        else if (command == "dump") {               // prints a dump of the sequencer contents
            if (par1.size() == 0)
                DumpMIDIMultiTrackWithPauses(sequencer.GetMultiTrack());
            else {
                int trk_num = atoi(par1.c_str());
                if (sequencer.GetMultiTrack()->IsValidTrackNumber(trk_num)) {
                    MIDITrack* trk = sequencer.GetMultiTrack()->GetTrack(trk_num);
                    DumpMIDITrackWithPauses(trk, trk_num);
                }
                else
                    cout << "Invalid track number" << endl;
            }
        }

        else if (command == "notify") {             // sets notifier on/off
            if (par1 == "on")
                notifier.SetEnable(true);
            else if (par1 == "off")
                notifier.SetEnable(false);
        }

        else if (command == "<") {                  // step backward
            int steps = (par1.length() == 0 ? 1 : atoi(par1.c_str()));
            for (int i = 0; i < steps; i++)
                cur_pos.stepback();
            sequencer.GoToTime(cur_pos.gettime());
        }

        else if (command == ">") {                  // step forward
            int steps = (par1.length() == 0 ? 1 : atoi( par1.c_str()));
            for (int i = 0; i < steps; i++)
                cur_pos.stepforward();
            MIDIClockTime cur_time = cur_pos.gettime();
            if (cur_time > multitrack->GetEndTime()) {
                multitrack->SetEndTime(cur_time);
                sequencer.SetChanged();
            }
            sequencer.GoToTime(cur_time);
        }

        else if (command == "t<") {                 // previous track
            cur_pos.previoustrack();
            trk = multitrack->GetTrack(cur_pos.gettrack());
        }

        else if (command == "t>") {                 // next track
            cur_pos.nexttrack();
            trk = multitrack->GetTrack(cur_pos.gettrack());
        }

        else if (command == "step") {               // sets the step size
            cur_pos.setstep(atoi(par1.c_str()));
            PrintResolution();
        }

        else if (command == "note") {               // inserts a note event
            msg.SetTime(cur_pos.gettime());
            if (par2 != "*") {
                MIDIClockTime len = (par2.length() == 0 ? last_note_length : atoi(par2.c_str()));
                int vel = (par3.length() == 0 ? last_note_vel : atoi(par3.c_str()));
                msg.SetNoteOn(cur_pos.gettrack() - 1, NameToValue(par1), vel);
                trk->InsertNote (msg, len);
            }
            else {
                msg.SetNoteOn(cur_pos.gettrack() - 1, NameToValue(par1), 100);
                if (!trk->FindEventNumber(msg, &event_num, COMPMODE_SAMEKIND))
                    cout << "Event not found" << endl;
                else {
                    msg = trk->GetEvent(event_num);
                    trk->DeleteNote(msg);
                }
            }
            sequencer.SetChanged();
        }

        else if (command == "volume") {                     // inserts a volume event
            msg.SetTime(cur_pos.gettime());
            if (par1 != "*") {
                msg.SetControlChange(cur_pos.gettrack() - 1, C_MAIN_VOLUME, atoi(par1.c_str()));
                trk->InsertEvent (msg);
            }
            else {
                msg.SetControlChange(cur_pos.gettrack() - 1, C_MAIN_VOLUME, 0);
                if (!trk->FindEventNumber(msg, &event_num, COMPMODE_SAMEKIND))
                    cout << "Event not found" << endl;
                else {
                    msg = trk->GetEvent(event_num);
                    trk->DeleteEvent(msg);
                }
            }
            sequencer.SetChanged();
        }

        else if ( command == "pan") {                   // inserts a pan event
            msg.SetTime(cur_pos.gettime());
            if (par1 != "*") {
                msg.SetControlChange(cur_pos.gettrack() - 1, C_PAN, atoi(par1.c_str()));
                trk->InsertEvent (msg);
            }
            else {
                msg.SetControlChange(cur_pos.gettrack() - 1, C_PAN, 0);
                if (!trk->FindEventNumber(msg, &event_num, COMPMODE_SAMEKIND))
                    cout << "Event not found" << endl;
                else {
                    msg = trk->GetEvent(event_num);
                    trk->DeleteEvent(msg);
                }
            }
            sequencer.SetChanged();
        }

        else if (command == "control") {                // inserts a generic control event

            msg.SetTime(cur_pos.gettime());
            if (par2 != "*") {
                msg.SetControlChange(cur_pos.gettrack() - 1, atoi(par1.c_str()), atoi(par2.c_str()));
                trk->InsertEvent (msg);
            }
            else {
                msg.SetControlChange(cur_pos.gettrack() - 1, atoi(par1.c_str()), 0);
                if (!trk->FindEventNumber(msg, &event_num, COMPMODE_SAMEKIND))
                    cout << "Event not found" << endl;
                else {
                    msg = trk->GetEvent(event_num);
                    trk->DeleteEvent(msg);
                }
            }
            sequencer.SetChanged();
        }

        else if ( command == "patch") {                     // inserts a patch event
            msg.SetTime(cur_pos.gettime());
            if (par1 != "*") {
                msg.SetProgramChange(cur_pos.gettrack() - 1, atoi(par1.c_str()));
                trk->InsertEvent (msg);
            }
            else {
                msg.SetProgramChange(cur_pos.gettrack() - 1, 0);
                if (!trk->FindEventNumber(msg, &event_num, COMPMODE_SAMEKIND))
                    cout << "Event not found" << endl;
                else {
                    msg = trk->GetEvent(event_num);
                    trk->DeleteEvent(msg);
                }
            }
            sequencer.SetChanged();
        }

        else if ( command == "tempo") {                     // inserts a tempo event in track 0
            msg.SetTime (cur_pos.gettime());
            if (par1 != "*") {
                msg.SetTempo(atoi(par1.c_str()));
                multitrack->GetTrack(0)->InsertEvent(msg);
            }
            else {
                msg.SetTempo(120);
                if (!multitrack->GetTrack(0)->FindEventNumber (msg, &event_num, COMPMODE_SAMEKIND))
                    cout << "Event not found" << endl;
                else {
                    msg = multitrack->GetTrack(0)->GetEvent(event_num);
                    trk->DeleteEvent(msg);
                }
            }
            sequencer.SetChanged();
        }

        else if (command == "time") {
            msg.SetTime (cur_pos.gettime());
            if (par1 != "*") {
                msg.SetTimeSig(atoi(par1.c_str()), atoi(par2.c_str()));
                multitrack->GetTrack(0)->InsertEvent(msg);
            }
            else {
                msg.SetTimeSig(4, 4);
                if (!multitrack->GetTrack(0)->FindEventNumber (msg, &event_num, COMPMODE_SAMEKIND))
                    cout << "Event not found" << endl;
                else {
                    msg = multitrack->GetTrack(0)->GetEvent(event_num);
                    trk->DeleteEvent(msg);
                }
            }
            sequencer.SetChanged();
        }

        else if (command == "bb") {
            cur_block.time_begin = cur_block.time_end = cur_pos.gettime();
            if (par1.size() == 0) {
                cur_block.track_begin = 0;
                cur_block.track_end = multitrack->GetNumTracks() - 1;
            }
            else {
                cur_block.track_begin = cur_block.track_end = atoi(par1.c_str());
            }
        }

        else if (command == "be") {
            if (cur_pos.gettime() < cur_block.time_begin)
                cout << "Selection error" << endl;
            else {
                cur_block.time_end = cur_pos.gettime();
                cout << "Block selected: from " << cur_block.time_begin << " to " <<
                        cur_block.time_end << "; tracks " << cur_block.track_begin <<
                        "-" << cur_block.track_end << endl;
            }
        }

        else if (command == "bcopy") {
            multitrack->EditCopy(cur_block.time_begin, cur_block.time_end,
                                 cur_block.track_begin, cur_block.track_end, &edit_multitrack);
        }

        else if (command == "bclear") {
            multitrack->EditClear(cur_block.time_begin, cur_block.time_end,
                                  cur_block.track_begin, cur_block.track_end);
            sequencer.SetChanged();
        }

        else if (command == "bcut") {
            if (cur_block.track_begin != 0 && cur_block.track_end != multitrack->GetNumTracks() + 1)
                cout << "You can't cut a number of tracks lesser than actual number" << endl;
            else {
                multitrack->EditCut(cur_block.time_begin, cur_block.time_end, 0);
                sequencer.SetChanged();
            }
        }

        else if (command == "bpaste") {
            unsigned int tr_start = atoi(par1.c_str());
            if (par2.size() == 0) {
                multitrack->EditInsert(cur_pos.gettime(), tr_start, 1, true, &edit_multitrack);
                sequencer.SetChanged();
            }
            else if (par2 == "o") {
                multitrack->EditReplace(cur_pos.gettime(), tr_start, 1, true, &edit_multitrack);
                sequencer.SetChanged();
            }
            else
                cout << "You must indicate initial track" << endl;
        }

        else if (command == "help") {                   // prints help screen
            cout << helpstring1;
            cout << "Press ENTER to continue";
            getchar();
            cout << helpstring2;
            cout << "Press ENTER to continue";
            getchar();
            cout << helpstring3;
        }

        else if (command != "quit")                     // unrecognized
            cout << "Unrecognized command" << endl;
    }

    return EXIT_SUCCESS;
}

