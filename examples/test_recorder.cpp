/*
 *   Example file for NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2020  Nicola Cassetta
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
  A command line example of the features of the MIDIRecorder class.
  You can record MIDI content through a MIDI in port in your system
  and store the recorded content in an AdvancedSequencer.
  Requires functions.cpp, which contains command line I/O functions.
*/


#include "../include/advancedsequencer.h"
#include "../include/recorder.h"
#include "../include/manager.h"
#include "functions.h"                  // helper functions for input parsing and output


using namespace std;


//////////////////////////////////////////////////////////////////
//                        G L O B A L S                         //
//////////////////////////////////////////////////////////////////

AdvancedSequencer sequencer;                // an AdvancedSequencer (without GUI notifier)
MIDIRecorder recorder;                      // a MIDIRecorder

extern string command, par1, par2;          // used by GetCommand() for parsing the user input

const char helpstring[] =                   // shown by the help command
"\nAvailable commands:\n\
   load filename       : Loads the file into the sequencer\n\
   save filename       : Save the current multitrack into a file\n\
   ports               : Enumerates MIDI In and OUT ports\n\
   play                : Starts playback from current time\n\
   stop                : Stops playback\n\
   rec on/off          : Enable/disable recording\n\
   enable port [chan]  : Enable input from port (if channel is not\n\
                         given records all channels)\n\
   disable port [chan] : Disable input from port (if channel is not\n\
                       : given disables all channels)\n\
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



//////////////////////////////////////////////////////////////////
//                              M A I N                         //
//////////////////////////////////////////////////////////////////


int main( int argc, char **argv ) {
    MIDIManager::AddMIDITick(&recorder);
    cout << "TYPE help TO GET A LIST OF AVAILABLE COMMANDS" << endl << endl;
    while ( command != "quit" ) {                   // main loop
        GetCommand();                               // gets user input and parses it (see functions.cpp)

        if( command == "load" ) {                   // loads a file
            if (sequencer.Load(par1.c_str()))
                cout << "Loaded file " << par1 << endl;
            else
                cout << "Error loading file" << endl;
        }
        else if( command == "save" ) {              // save the multitrack contents
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
        else if( command == "rec" ) {               // starts/stops recording
            if (par1 == "on")
                recorder.Start();
            else {
                recorder.Stop();
                sequencer.Load(recorder.GetMultiTrack());
            }
        }
        else if( command == "enable" ) {            // enables recording from a port
            int port = atoi(par1.c_str());
            int chan = (par2.size() ? atoi(par2.c_str()) : -1);
            if (port < 0 || (unsigned)port >= MIDIManager::GetNumMIDIIns() || chan < -1 || chan > 15)
                cout << "Invalid parameters" << endl;
            else {
                recorder.EnablePort(port, chan);
                if (chan == -1)
                    cout << "Enabled recording from port " << par1 << ", all channels" << endl;
                else
                    cout << "Enabled recording form port " << par1 << ", channel " << chan + 1 << endl;
            }
        }
        else if( command == "disable" ) {           // disables recording from a port
            int port = atoi(par1.c_str());
            int chan = (par2.size() ? atoi(par2.c_str()) : -1);
            if (port < 0 || (unsigned)port >= MIDIManager::GetNumMIDIIns() || chan < -1 || chan > 15)
                cout << "Invalid parameters" << endl;
            else {
                if (chan == -1) {
                    recorder.DisablePort(port);
                    cout << "Disabled recording from port " << par1 << endl;
                }
                else {
                    recorder.DisableChannel(port, chan);
                    cout << "Disabled recording form port " << par1 << ", channel " << chan + 1 << endl;
                }
            }
        }
        else if (command == "rew") {                // stops and rewinds to time 0
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
                DumpMIDIMultiTrackWithPauses(recorder.GetMultiTrack()); //in functions.cpp
            else {
                int trk_num = atoi(par1.c_str());
                if (recorder.GetMultiTrack()->IsValidTrackNumber(trk_num)) {
                    MIDITrack* trk = recorder.GetMultiTrack()->GetTrack(trk_num);
                    DumpMIDITrackWithPauses(trk, trk_num);              // in functions.cpp
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
        else if (command == "thru") {               // toggles MIDI thru on and off
            if (par1 == "on") {
                sequencer.SetMIDIThruEnable(true);
                if (sequencer.GetMIDIThru()->IsPlaying()) {
                    cout << "Set MIDI thru on" << endl;
                    cout << "In port " << MIDIManager::GetMIDIInName(sequencer.GetMIDIThru()->GetInPort()) << endl;
                    cout << "Out port" << MIDIManager::GetMIDIOutName(sequencer.GetMIDIThru()->GetOutPort()) << endl;
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
    return EXIT_SUCCESS;
}

