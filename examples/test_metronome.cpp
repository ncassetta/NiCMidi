/*
 *   Example file for NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2021, 2022  Nicola Cassetta
 *   https://github.com/ncassetta/NiCMidi
 *
 *   This file is part of NiCMidi.
 *
 *   NiCMidi is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation, either version 3 of
 *   the License, or (at your option) any later version.
 *
 *   NiCMidi is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with NiCMidi.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
  A command line example of the features of the Metronome class.
  It creates an instance of the class and allows the user to interact
  with it. You can start and stop the metronome, adjust tempo,
  measure counting,subdivide the metronome beats and adjust other
  parameters.
  Requires functions.cpp, which contains command line I/O functions.
*/


#include "../include/manager.h"
#include "../include/metronome.h"
#include "functions.h"                  // helper functions for input parsing and output


using namespace std;


//////////////////////////////////////////////////////////////////
//                        G L O B A L S                         //
//////////////////////////////////////////////////////////////////

Metronome* metro;                               // a Metronome (without GUI notifier)

extern string command, par1, par2;              // used by GetCommand() for parsing the user input

const char helpstring[] =                       // shown by the help command
"\nAvailable commands:\n\
   ports               : Enumerates MIDI In and OUT ports\n\
   start               : Starts the metronome\n\
   stop                : Stops the metronome\n\
   tempo bpm           : Sets the metronome tempo (bpm is a float)\n\
   tscale scale        : Sets global tempo scale. scale is in percent\n\
                         (ex. 200 = twice faster, 50 = twice slower)\n\
   subd n              : Sets the number of subdivisions (n can be\n\
                         0, 2, 3, 4, 5, 6, 0 disables subdivisions)\n\
   meas n              : Sets the number of beats of a measure (0 disables\n\
                         measure clicks)\n\
   measnote nn         : Sets the MIDI note for first beat of the measure\n\
   beatnote nn         : Sets the MIDI note for ordinary beats\n\
   subdnote nn         : Sets the MIDI note for subdivisions\n\
   outport port        : Sets the MIDI out port\n\
   outchan ch          : Sets the MIDI out channel\n\
   status              : Prints the status of the metronome\n\
   help                : Prints this help screen\n\
   quit                : Exits\n\
MIDI channels are numbered 0 .. 15\n\
All commands can be given during playback\n";


//////////////////////////////////////////////////////////////////
//                              M A I N                         //
//////////////////////////////////////////////////////////////////


int main( int argc, char **argv ) {
    try {
        metro = new Metronome;
    }
    catch( ... ) {
        // tests if MIDI ports are present in the system
        cout << "The Metronome constructor throws an exception if in the system is not present\n" <<
                 "almost a MIDI out port!\n" <<
                 "Press a key to quit\n";
        cin.get();
        return EXIT_SUCCESS;
    }

    cout << "TYPE help TO GET A LIST OF AVAILABLE COMMANDS\n\n";
    while (command != "quit") {                     // main loop
        GetCommand();                               // gets user input and splits it into command, par1, par2

        if(command == "")                           // empty command
            continue;
        if (command == "ports") {                   // enumerates the midi ports
            if (MIDIManager::GetNumMIDIIns()) {
                cout << "MIDI IN PORTS:" << endl;
                for (unsigned int i = 0; i < MIDIManager::GetNumMIDIIns(); i++)
                    cout << i << ": " << MIDIManager::GetMIDIInName( i ) << endl;
            }
            else
                cout << "NO MIDI IN PORTS" << endl;
            cout << "MIDI OUT PORTS:" << endl;
            for (unsigned int i = 0; i < MIDIManager::GetNumMIDIOuts(); i++)
                cout << i << ": " << MIDIManager::GetMIDIOutName( i ) << endl;
        }
        else if (command == "start") {               // starts the metronome
            metro->Start();
            cout << "Metronome started" << endl;
        }
        else if (command == "stop") {               // stops the metronome
            metro->Stop();
            cout << "Metronome stopped at " << metro->GetCurrentMeasure() << ":"
                 << metro->GetCurrentBeat() << endl;
        }
        else if (command == "tempo") {              // changes the metronome tempo
            float tempo = atof(par1.c_str());
            if (metro->SetTempo(tempo)) {
                cout << "Tempo set to " << tempo <<
                        "  Effective tempo: " << metro->GetTempoWithScale() << " bpm" << endl;
            }
            else
                cout << "Invalid tempo" << endl;
        }
        else if (command == "tscale") {             // scales the metronome tempo
            int scale = atoi(par1.c_str());
            if (metro->SetTempoScale(scale)) {
                cout << "Tempo scale : " << scale <<
                        "%  Effective tempo: " << metro->GetTempoWithScale() << " bpm" << endl;
            }
            else
                cout << "Invalid tempo scale" << endl;
        }
        else if (command == "subd") {               // sets the number of subdivision of each beat
            unsigned int type = atoi(par1.c_str());          // 0 disables subdivision clicks
            if (metro->SetSubdType(type)) {
                if (type == 0)
                    cout << "Subdivision click disabled" << endl;
                else
                     cout << "Number of subdivisions set to " << type << endl;
            }
            else
                cout << "Invalid number of subdivisions" << endl;
        }
        else if (command == "meas") {               // sets the number of beats of a measure
            unsigned int beats = atoi(par1.c_str());         // 0 disables measure clicks
            metro->SetTimeSigNumerator(beats);
            if (beats == 0)
                cout << "First beat disabled" << endl;
            else
                cout << "Beats set to " << beats << endl;
        }

        else if (command == "measnote") {           // sets the note for 1st beat of a measure
            unsigned int note = atoi(par1.c_str()) & 0x7f;
            metro->SetMeasNote(note);
            cout << "First beat note set to " << note << endl;
        }
        else if (command == "beatnote") {           // sets the note for ordinary beats
            unsigned int note = atoi(par1.c_str()) & 0x7f;
            metro->SetBeatNote(note);
            cout << "Beat note set to " << note << endl;
        }
        else if (command == "subdnote") {           // sets the note for subdivisions
            unsigned int note = atoi(par1.c_str()) & 0x7f;
            metro->SetSubdNote(note);
            cout << "Subdivision note set to " << note << endl;
        }
        else if (command == "outport") {            // changes the midi out port
            int port = atoi(par1.c_str());
            if (metro->SetOutPort(port))
                cout << "Assigned out port n. " << port << endl;
            else
                cout << "Invalid port number" << endl;
        }
        else if (command == "outchan") {            // changes the midi out chan
            int chan = atoi(par1.c_str());
            if (metro->SetOutChannel(chan))
                cout << "Assigned out channel n. " << (int)chan << endl;
            else
                cout << "Invalid channel number" << endl;
        }
        else if (command == "status") {
            cout << "\nMETRONOME STATUS:\n";
            cout << "MIDI out port:             " << MIDIManager::GetMIDIOutName(metro->GetOutPort()) << endl;
            cout << "MIDI out channel:          " << int(metro->GetOutChannel()) << endl;
            cout << "Tempo:                     " << metro->GetTempoWithoutScale() << " bpm" << endl;
            cout << "Tempo scale:               " << metro->GetTempoScale() << "% (effective tempo: " <<
                    metro->GetTempoWithScale() << " bpm)" << endl;
            cout << "Measure beats:             " << int(metro->GetTimeSigNumerator());
            cout << (metro->GetTimeSigNumerator() == 0 ? " (disabled)" : "") << endl;
            cout << "Subdivision:               " << int(metro->GetSubdType());
            cout << (metro->GetSubdType() == 0 ?" (disabled)" : "") << endl;
            cout << "Measure beat note:         " << int(metro->GetMeasNote()) << endl;
            cout << "Ordinary beat note:        " << int(metro->GetBeatNote()) << endl;
            cout << "Subdivision note:          " << int(metro->GetSubdNote()) << endl;
        }
        else if (command == "help")                 // prints help screen
            cout << helpstring;
        else if (command != "quit" )                // unrecognized command
            cout << "Unrecognized command" << endl;
    }
    delete metro;
    return EXIT_SUCCESS;
}
