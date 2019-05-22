/*
  A command line example of the features of the MIDIThru class.
  It creates an instance of the class and allows the user to interact
  with it. You can set the in and out ports and channels and the
  program change in the out channel; moreover you can print the
  passing MIDI messages with a MIDIrocessorPrinter.
  Compile it together with functions.cpp, which contains command
  line I/O functions.

  Copyright (C) 2016 - 2019 N.Cassetta
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


#include "../include/manager.h"
#include "../include/thru.h"
#include "functions.h"

#include <iostream>
#include <string>

using namespace std;


//////////////////////////////////////////////////////////////////
//                        G L O B A L S                         //
//////////////////////////////////////////////////////////////////

MIDIThru thru;                                  // a MIDIThru
MIDIProcessorPrinter printer;                   // a MIDIProcessor which prints passing MIDI events

extern string command, par1, par2;              // used by GetCommand() for parsing the user input

const char helpstring[] =                       // shown by the help command
"\nAvailable commands:\n\
   ports               : Enumerates MIDI IN and OUT ports\n\
   outport port [chan] : Sets port as current thru output device\n\
                         (default: port 0, chan all)\n\
   inport port [chan]  : Sets port as current input device\n\
                         (default: port 0, chan all)\n\
   thru on/off         : Sets MIDI thru on/off\n\
   program prog        : Sets the MIDI program (patch) number for\n\
                         the current out channel (if a single channel\n\
                         is selected\n\
   print on/off        : Turns on and off the printing of incoming\n\
                         messages\n\
   status              : Prints the status of the program\n\
   help                : Prints this help screen\n\
   quit                : Exits\n\n\
   MIDI channels are numbered 0 .. 15, and -1 stands for \"all channels\"";



//////////////////////////////////////////////////////////////////
//                              M A I N                         //
//////////////////////////////////////////////////////////////////


int main( int argc, char **argv ) {
    if (MIDIManager::GetNumMIDIIns() == 0 || MIDIManager::GetNumMIDIOuts()) {
        cout << "This program has no functionality without MIDI ports!\n" <<
                "Press a key to quit\n";
        cin.get();
        return EXIT_SUCCESS;
    }
    // adds the thru to the MIDIManager queue
    MIDIManager::AddMIDITick(&thru);
    // plugs the printer processor into the thru object
    thru.SetProcessor(&printer);
    cout << "TYPE help TO GET A LIST OF AVAILABLE COMMANDS\n\n";
    while ( command != "quit" )                     // main loop
    {
        GetCommand();                               // gets user input and parse it

        if ( command == "ports") {                  // enumerates the midi ports
            cout << "\tMIDI IN PORTS:\n";
            for ( unsigned int i = 0; i < MIDIManager::GetNumMIDIIns(); i++ )
                cout << i << ": " << MIDIManager::GetMIDIInName( i ) << endl;
            if ( MIDIManager::GetNumMIDIOuts() ) {
                cout << "MIDI OUT PORTS:\n";
                for ( unsigned int i = 0; i < MIDIManager::GetNumMIDIOuts(); i++ )
                    cout << i << ": " << MIDIManager::GetMIDIOutName( i ) << endl;
            }
        }
        else if ( command == "outport") {               // changes the midi out port
            int port = atoi( par1.c_str() );
            int chan = -1;
            if (par2.length() > 0)
                chan = atoi( par2.c_str() );
            if (port < 0 || (unsigned)port >= MIDIManager::GetNumMIDIOuts())
                cout << "Invalid port number\n";
            else {
                thru.SetOutPort(MIDIManager::GetOutDriver(port));
                thru.SetOutChannel(chan);
                cout << "Out port for MIDI thru:\n" << thru.GetOutPort()->GetPortName()
                     << "\tChan: " << thru.GetOutChannel() << endl;;
            }
        }
        else if ( command == "inport" ) {               // changes the midi in port
            int port = atoi( par1.c_str() );
            int chan = -1;
            if (par2.length() > 0)
                chan = atoi( par2.c_str() );
            if (port < 0 || (unsigned)port >= MIDIManager::GetNumMIDIIns())
                cout << "Invalid port number\n";
            else {
                thru.SetInPort(MIDIManager::GetInDriver(port));
                thru.SetInChannel(chan);
                cout << "In port for MIDI thru:\n" << thru.GetInPort()->GetPortName()
                     << "\tChan: " << thru.GetInChannel() << endl;
            }
        }
        else if ( command == "thru") {                  // sets the MIDI thru on and off
            if (par1 == "on") {
                thru.Start();
                cout << "MIDIthru on, from  in port " << thru.GetInPort()->GetPortName()
                     << " to out port " << thru.GetOutPort()->GetPortName() << endl;
            }
            else if (par1 == "off") {
                thru.Stop();
                cout << "MIDI thru off\n";
            }
        }
        else if ( command == "program" ) {              // sets the MIDI program
            MIDITimedMessage msg;
            unsigned char prog = atoi( par1.c_str() ) % 0x7f;
            unsigned char chan = (thru.GetOutChannel() == -1 ? 0 : thru.GetOutChannel());
            msg.SetProgramChange(chan, prog);

            thru.GetOutPort()->OutputMessage(msg);
            cout << "Sent program change " << prog << "on port " << thru.GetOutPort()->GetPortName()
                 << " channel " << chan << endl;
        }
        else if (command == "print") {                  // sets the printing of passing events on and off
            if (par1 == "on") {
                printer.SetPrint(true);
                cout << "Print on (port " << thru.GetInPort()->GetPortName() << ")" << endl;
            }
            else if (par1 == "off") {
                printer.SetPrint(false);
                cout << "Print off\n";
            }
        }
        else if ( command == "status" ) {               // prints the general thru status
            MIDIInDriver* thru_in = thru.GetInPort();
            MIDIOutDriver* thru_out = thru.GetOutPort();
            cout << "\nPROGRAM STATUS:\n";
            cout << "MIDI in port:             " << thru_in->GetPortName() << endl;
            cout << "MIDI in channel:          ";
            if (thru.GetInChannel() == -1)
                cout << "all\n";
            else
                cout << thru.GetInChannel() << "\n";
            cout << "MIDI out port:            " << thru_out->GetPortName() << endl;
            cout << "MIDI out channel:         ";
            if (thru.GetOutChannel() == -1)
                cout << "all\n";
            else
                cout << thru.GetOutChannel() << "\n";
            cout << "MIDI thru:                ";
            cout << (thru.IsPlaying() ? "ON\n" : "OFF\n");
            cout << "Print incoming messages : ";
            cout << (printer.GetPrint() ? "ON\n" : "OFF\n");
        }
        else if (command == "help")                    // prints help screen
            cout << helpstring;
        else if (command != "quit" )                   // unrecognized command
            cout << "Unrecognized command" << endl;
    }
    return EXIT_SUCCESS;
}

