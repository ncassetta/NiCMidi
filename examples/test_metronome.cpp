/*
  Metronome and MIDIRecorder class example for libJDKSmidi C++ MIDI
  Library (console app, no GUI!)

  Copyright (C) 2018 N.Cassetta
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
// Copyright (C) 2018 N. Cassetta
// ncassetta@tiscali.it
//


#include "../include/metronome.h"
#include "../include/processor.h"
#include <string>


using namespace std;

string command_buf, command, par1, par2;    // used by GetCommand() for parsing the user input
Metronome metro;                            // a Metronome (without GUI notifier)
MIDIProcessorPrinter printer;

const char helpstring[] =
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
   beatnote nn         : Sets the MIDI note for ordinaty beats\n\
   subdnote nn         : Sets the MIDI note for subdivisions\n\
   outport port        : Sets the MIDI out port\n\
   outchan ch          : Sets the MIDI out channel\n\
   help                : Prints this help screen\n\
   quit                : Exits\n\
All commands can be given during playback\n";



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





int main( int argc, char **argv ) {
    MIDIManager::GetOutDriver(0)->SetOutProcessor(&printer);
    MIDIManager::AddMIDITick(&metro);
    cout << "TYPE help TO GET A LIST OF AVAILABLE COMMANDS" << endl << endl;
    while ( command != "quit" ) {                   // main loop
        GetCommand();                               // gets user input and parse it

        if (command == "ports") {                   // enumerates the midi ports
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

        else if (command == "start") {               // starts the metronome
            metro.Start();
            cout << "Metronome started" << endl;
        }

        else if (command == "stop") {               // stops the mtronome
            metro.Stop();
            cout << "Metronome stopped at " << metro.GetCurrentMeasure() << ":"
                 << metro.GetCurrentBeat() << endl;
        }

        else if (command == "tempo") {              // changes the metronome tempo
            float tempo = atof(par1.c_str());
            metro.SetTempo(tempo);
            cout << "Set tempo to " << tempo << endl;
        }
        else if (command == "tscale") {             // scales the metronome tempo
            int scale = atoi(par1.c_str());
            metro.SetTempoScale(scale);
            cout << "Tempo scale : " << scale << "%  " <<
                    " Effective tempo: " << metro.GetTempoWithScale() << " bpm" << endl;
        }
        else if (command == "subd") {               // sets the number of subdivision of each beat
            int type = atoi(par1.c_str());          // 0 disables subdivision clicks
            metro.SetSubdType(type);
            if (metro.GetSubdType() == type)
                cout << "Set number of subdivisions to " << type << endl;
            else
                cout << "Set subdivisions failed" << endl;
        }
        else if (command == "meas") {               // sets the number of beats of a measuree
            int beats = atoi(par1.c_str());         // 0 disables measure clicks
            metro.SetTimeSigNumerator(beats);
            if (beats > 0)
                cout << "Beats set to " << beats << endl;
            else
                cout << "First beat disabled" << endl;
        }

        else if (command == "measnote") {           // sets the note for 1st beat of a measure
            int note = atoi(par1.c_str());
            metro.SetMeasNote(note);
            cout << "Set first beat note to " << note << endl;
        }
        else if (command == "beatnote") {           // sets the note for ordinary beats
            int note = atoi(par1.c_str());
            metro.SetBeatNote(note);
            cout << "Set beat note to " << note << endl;
        }
        else if (command == "subdnote") {           // sets the note for subdivisions
            int note = atoi(par1.c_str());
            metro.SetSubdNote(note);
            cout << "Set subdivision note to " << note << endl;
        }
        else if (command == "outport") {            // changes the midi out port
            int port = atoi(par1.c_str());
            if (port < 0 || (unsigned)port >= MIDIManager::GetNumMIDIOuts())
                cout << "Invalid port number" << endl;
            else {
                metro.SetOutPort(port);
                cout << "Assigned out port n. " << metro.GetOutPort() << endl;
            }
        }
        else if (command == "outchan") {            // changes the midi out chan
            int chan = atoi(par1.c_str());
            if (chan < 0 || chan > 15)
                cout << "Invalid channel number" << endl;
            else {
                metro.SetOutChannel(chan);
                cout << "Assigned out channel n. " << (int)metro.GetOutChannel() << endl;
            }
        }




        else if (command == "help")                 // prints help screen
            cout << helpstring;
        else if (command != "quit")
            cout << "Unrecognized command" << endl;
    }
    return EXIT_SUCCESS;
}

