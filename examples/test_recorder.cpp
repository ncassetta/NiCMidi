/*
  AdvancedSequencer class example for libJDKSmidi C++ MIDI Library
  (console app, no GUI!)

  Copyright (C) 2013 N.Cassetta
  ncassetta@tiscali.it

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program;
  if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
//
// Copyright (C) 2013 N. Cassetta
// ncassetta@tiscali.it
//

#include "../include/advancedsequencer.h"
#include "../include/recorder.h"
#include "../include/dump_tracks.h"

#include <iostream>
#include <string>

using namespace std;


//
// globals
//

string command_buf, command, par1, par2;    // used by GetCommand() for parsing the user input
AdvancedSequencer sequencer;                // an AdvancedSequencer (without GUI notifier)
MIDIRecorder recorder;                      // a MIDIRecorder

const char helpstring[] =
"\nAvailable commands:\n\
   load filename       : Loads the file into the sequencer\n\
   save filename       : Save the current multitrack into a file\n\
   ports               : Enumerates MIDI In and OUT ports\n\
   play                : Starts playback from current time\n\
   stop                : Stops playback\n\
   rec on/off          : Enable/disable recording\n\
   rew                 : Goes to the beginning (stops the playback)\n\
   goto meas [beat]    : Moves current time to given meas and beat\n\
                         (numbered from 0)\n\
   dump [trk]          : Prints a dump of all midi events in the file\n\
                         (or in the track trk)\n\
   outport track port  : Sets the MIDI port for the given track\n\
   thru on/off         : Sets the MIDI thru on and off.\n\
   trackinfo           : Shows info about all tracks of the file\n\
   b                   : (backward) Moves current time to the previous measure\n\
   f                   : (forward) Moves current time to the next measure\n\
   help                : Prints this help screen\n\
   quit                : Exits\n\
All commands can be given during playback\n";

const char TRACK_TYPES[6][12] = {
    "MAIN TRACK",
    "TEXT ONLY ",
    "CHANNEL   ",
    "IRREG CHAN",
    "MIXED CHAN",
    "IRREGULAR "

};


void GetCommand()
// gets from the user the string command_buf, then parses it dividing it into command, par1 and par2 substrings
{
    size_t pos1, pos2;

    cout << "\n=> ";
    getline(cin, command_buf);

    command = "";
    par1 = "";
    par2 = "";

    for (size_t i = 0; i < command_buf.size(); ++i)
        command_buf[i] = tolower( command_buf[i]);

    pos1 = command_buf.find_first_not_of ( " ");
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if (pos1 == string::npos)
        return;

    command = command_buf.substr (pos1, pos2 - pos1);

    pos1 = command_buf.find_first_not_of (" ", pos2);
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if (pos1 == string::npos)
        return;

    par1 = command_buf.substr (pos1, pos2 - pos1);
    pos1 = command_buf.find_first_not_of (" ", pos2);
    pos2 = command_buf.find_first_of (" ", pos1 + 1);
    if (pos1 == string::npos)
        return;

    par2 = command_buf.substr (pos1, pos2 - pos1);
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


int main( int argc, char **argv ) {
    //CoInitializeEx(NULL, COINIT_MULTITHREADED);
    MIDIManager::AddMIDITick(&recorder);
    cout << "TYPE help TO GET A LIST OF AVAILABLE COMMANDS" << endl << endl;
    while ( command != "quit" ) {                   // main loop
        GetCommand();                               // gets user input and parse it

        if( command == "load" ) {                   // loads a file
            if (sequencer.Load(par1.c_str()))
                cout << "Loaded file " << par1 << endl;
            else
                cout << "Error loading file" << endl;
        }
        else if( command == "load" ) {              // save the multitrack contents
            /*
            if (sequencer.Load(par1.c_str()))
                cout << "Loaded file " << par1 << endl;
            else
                cout << "Error loading file" << endl;
            */
        }
        else if (command == "ports") {              // enumerates the midi ports
            if (MIDIManager::GetNumMIDIIns()) {
                cout << "MIDI IN PORTS:" << endl;
                for (unsigned int i = 0; i < MIDIManager::GetNumMIDIIns(); i++)
                    cout << i << ": " << MIDIManager::GetMIDIInName( i ) << endl;
            }
            else
                cout << "NO MIDI IN PORTS" << endl;
            if (MIDIManager::GetNumMIDIOuts()) {
                cout << "MIDI OUT PORTS:" << endl;
                for (unsigned int i = 0; i < MIDIManager::GetNumMIDIOuts(); i++)
                    cout << i << ": " << MIDIManager::GetMIDIOutName( i ) << endl;
            }
            else
                cout << "NO MIDI OUT PORTS" << endl;
        }
        else if (command == "play") {               // starts playback
            sequencer.Play();
            cout << "Sequencer started at measure: " << sequencer.GetCurrentMeasure() << ":"
                 << sequencer.GetCurrentBeat() << endl;
        }
        else if (command == "stop") {               // stops playback
            sequencer.Stop();
            cout << "Sequencer stopped at measure: " << sequencer.GetCurrentMeasure() << ":"
                 << sequencer.GetCurrentBeat() << endl;
        }
        else if( command == "rec" ) {               // enable/disable recording
            /*
            if (sequencer.Load(par1.c_str()))
                cout << "Loaded file " << par1 << endl;
            else
                cout << "Error loading file" << endl;
            */
        }
        else if (command == "rew") {                // stops and rewind to time 0
            sequencer.GoToZero();
            cout << "Rewind to 0:0" << endl;
        }
        else if (command == "goto") {               // goes to meas and beat
            int measure = atoi(par1.c_str());
            int beat = atoi (par2.c_str());
            if (measure < 0 || measure > sequencer.GetNumMeasures() - 1)
                cout << "Invalid position" << endl;
            else {
                sequencer.GoToMeasure(measure, beat);
                cout << "Actual position: " << sequencer.GetCurrentMeasure() << ":"
                     << sequencer.GetCurrentBeat() << endl;
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
        else if (command == "outport") {            // changes the midi out port
            int track = atoi(par1.c_str());
            int port = atoi(par2.c_str());
            if (port < 0 || (unsigned)port >= MIDIManager::GetNumMIDIOuts())
                cout << "Invalid port number" << endl;
            else {
                sequencer.SetTrackOutPort(track, port);
                cout << "Assigned out port n. " << sequencer.GetTrackPort(track)
                     << " to track " << track << endl;
            }
        }
        /* COULD BE USED FOR MIDI THRU
        else if ( command == "inport" )                 // changes the midi in port
        {
            int port = atoi( par1.c_str() );
            sequencer.SetInputPort( port );
            cout << "Assigned in port n. " << sequencer.GetInputPort();
        }
        */
        else if (command == "thru") {               // toggles MIDI thru on and off
            if (par1 == "on") {
                sequencer.SetMIDIThruEnable(true);
                if (sequencer.GetMIDIThru()->IsPlaying()) {
                    cout << "Set MIDI thru on" << endl;
                    cout << "In port " << sequencer.GetMIDIThru()->GetInPort()->GetPortName() << endl;
                    cout << "Out port" << sequencer.GetMIDIThru()->GetOutPort()->GetPortName() << endl;
                }
            }
            else if (par1 == "off") {
                sequencer.SetMIDIThruEnable(false);
                cout << "Set MIDI thru off" << endl;
            }
        }
        else if (command == "trackinfo") {          // prints info about tracks
            for (unsigned int i = 0; i < sequencer.GetNumTracks(); i++) {
                MIDITrack* trk = sequencer.GetMultiTrack()->GetTrack(i);
                cout << "Track " << i << ": " << sequencer.GetTrackName(i) << endl;
                if (trk->IsEmpty())
                    cout << "EMPTY" << endl;
                else {
                    cout << "Type: " << TRACK_TYPES[trk->GetType() - MIDITrack::TYPE_MAIN];
                    if (trk->GetChannel() == -1)
                        cout << "     ";
                    else
                        cout << " (" << (int)trk->GetChannel() << ") ";
                    cout <<"Sysex: " << (trk->HasSysex() ? "Yes " : "No  ") << "Events: " << trk->GetNumEvents() << endl;
                }
            }
        }
        else if (command == "b") {                  // goes a measure backward
            int meas = sequencer.GetCurrentMeasure();
            if (meas > 0)
                sequencer.GoToMeasure(--meas);
            cout << "Actual position: " << sequencer.GetCurrentMeasure() << ":"
                 << sequencer.GetCurrentBeat() << endl;
        }
        else if (command == "f") {                  // goes a measure forward
            int meas = sequencer.GetCurrentMeasure();
            if (meas < sequencer.GetNumMeasures())
                sequencer.GoToMeasure(++meas);
            cout << "Actual position: " << sequencer.GetCurrentMeasure() << ":"
                 << sequencer.GetCurrentBeat() << endl;
        }
        else if (command == "help")                 // prints help screen
            cout << helpstring;
        else if (command != "quit")
            cout << "Unrecognized command" << endl;
    }
    //CoUninitialize();
    return EXIT_SUCCESS;
}

