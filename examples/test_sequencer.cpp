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
  A command line example of the features of the MIDISequencer
  class. It creates an instance of the class and allows the user
  to interact with it. You can load MIDI files and play them
  changing some parameters (it has more limited features than the
  AdvancedSequencer).
  Requires functions.cpp, which contains command line I/O functions.
*/


#include "../include/sequencer.h"
#include "../include/manager.h"
#include "../include/filereadmultitrack.h"
#include "functions.h"                  // helper functions for input parsing and output

using namespace std;


//////////////////////////////////////////////////////////////////
//                        G L O B A L S                         //
//////////////////////////////////////////////////////////////////

// a MIDIMultiTrack
MIDIMultiTrack tracks;
// this ctor creates the sequencer from the given multitrack
MIDISequencer sequencer(&tracks);

extern string command, par1, par2;          // used by GetCommand() for parsing the user input
const char helpstring[] =                   // shown by the help command
"\nAvailable commands:\n\
   load filename       : Loads the file into the sequencer\n\
   ports               : Enumerates MIDI In and OUT ports\n\
   play                : Starts playback from current time\n\
   loop meas1 meas2    : Sets loop play (doesn't start it!) from\n\
                         meas1 to meas2. Give 0 0 for non loop play\n\
   stop                : Stops playback\n\
   rew                 : Goes to the beginning (stops the playback)\n\
   goto meas [beat]    : Moves current time to given meas and beat\n\
                         (numbered from 0)\n\
   dump [trk]          : Prints a dump of all midi events in the file\n\
                         (or in the track trk)\n\
   outport track port  : Sets the MIDI port for the given track\n\
   tscale scale        : Sets global tempo scale. scale is in percent\n\
                         (ex. 200 = twice faster, 50 = twice slower)\n\
   tshift track amt    : Sets the time shift for given track. The amount can\n\
                         be positive or negative.\n\
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
    // you must add the sequencer to the MIDIManager queue (AdvancedSequencer
    // does it by itself
    MIDIManager::AddMIDITick(&sequencer);
    cout << "TYPE help TO GET A LIST OF AVAILABLE COMMANDS" << endl << endl;
    while ( command != "quit" ) {                   // main loop
        GetCommand();                               // gets user input and parse it

        if( command == "load" ) {                   // loads a file
            // this is different from the same on AdvancedSequencer, because there is
            // no dedicated method in MIDISequencer: you must load the multitrack and
            // then call MIDISequencer::Reset() for initializing the sequencer
            if (LoadMIDIFile(par1, &tracks))
                cout << "Loaded file " << par1 << endl;
            else
                cout << "Error loading file" << endl;
            sequencer.Reset();
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
        else if (command == "loop") {               // sets repeates play
            int beg = atoi(par1.c_str());
            int end = atoi(par2.c_str());
            if (!(beg == 0 && end == 0)) {

                sequencer.SetRepeatPlay( true, beg, end );
                cout << "Repeat play set from measure " << beg << " to measure " << end << endl;
            }
            else {
                sequencer.SetRepeatPlay(false, 0, 0);
                cout << "Repeat play cleared" << endl;
            }
        }
        else if (command == "stop") {               // stops playback
            sequencer.Stop();
            cout << "Sequencer stopped at measure: " << sequencer.GetCurrentMeasure() << ":"
                 << sequencer.GetCurrentBeat() << endl;
        }
        else if (command == "rew") {                // stops and rewind to time 0
            sequencer.GoToZero();
            cout << "Rewind to 0:0" << endl;
        }
        else if (command == "goto") {               // goes to meas and beat
            // if there are program change, control change or sysex events between actual
            // and new time, MIDISequencer doesn't send them, so you can expect inaccurate
            // playing when you jump from one time to another (this is solver in
            // AdvancedSequencer)
            int measure = atoi(par1.c_str());
            int beat = atoi (par2.c_str());
            MIDIClockTime now = sequencer.GetCurrentMIDIClockTime();
            if (sequencer.GoToMeasure(measure, beat))
                cout << "Actual position: " << sequencer.GetCurrentMeasure() << ":"
                     << sequencer.GetCurrentBeat() << endl;
            else {
                sequencer.GoToTime(now);
                cout << "Invalid position" << endl;
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
        else if (command == "tscale") {             // scales playback tempo
            int scale = atoi(par1.c_str());
            sequencer.SetTempoScale(scale);
            cout << "Tempo scale : " << scale << "%  " <<
                    " Effective tempo: " << sequencer.GetTempoWithScale() << " bpm" << endl;
        }
        else if (command == "tshift") {             // sets the time shift (in ticks) of a track
            int track = atoi(par1.c_str());
            int amount = atoi(par2.c_str());
            sequencer.SetTrackTimeShift(track, amount);
            cout << "Track " << track << " time shifted by " << amount << " MIDI ticks" << endl;
        }
        else if (command == "trackinfo") {          // prints info about tracks
            for (unsigned int i = 0; i < sequencer.GetNumTracks(); i++) {
                MIDITrack* trk = sequencer.GetMultiTrack()->GetTrack(i);
                cout << "Track " << i << ": " << sequencer.GetState()->track_states[i]->track_name << endl;
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
            if (sequencer.GoToMeasure(++meas))
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

