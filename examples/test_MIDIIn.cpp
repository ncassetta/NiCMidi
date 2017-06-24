/*

  MIDIThru class example for libJDKSmidi C++ MIDI Library
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
// Copyright (C) 2016 - 2017 N. Cassetta
// ncassetta@tiscali.it
//

#include "../include/driver.h"
#include "../include/manager.h"
#include "../include/process.h"
#include "../include/thru.h"

#include <iostream>
#include <string>

using namespace std;


//
// globals
//

string command_buf, command, par1, par2;    // used by GetCommand() for parsing the user input

const char helpstring[] =
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
   quit                : Exits\n";


MIDIManager manager;
MIDIThru thru;
MIDIProcessorPrinter printer;


void GetCommand()
// gets from the user the string command_buf, then parses it dividing it into command, par1 and par2 substrings
{
    size_t pos1, pos2;

    cout << "\n=> ";
    getline( cin, command_buf );

    command = "";
    par1 = "";
    par2 = "";

    for ( size_t i = 0; i < command_buf.size(); ++i)
        command_buf[i] = tolower( command_buf[i]);

    pos1 = command_buf.find_first_not_of ( " ");
    pos2 = command_buf.find_first_of (" ", pos1+1);
    if ( pos1 == string::npos )
        return;

    command = command_buf.substr (pos1, pos2 - pos1);

    pos1 = command_buf.find_first_not_of ( " ", pos2);
    pos2 = command_buf.find_first_of (" ", pos1+1);
    if ( pos1 == string::npos )
        return;

    par1 = command_buf.substr (pos1, pos2 - pos1);

    pos1 = command_buf.find_first_not_of ( " ", pos2);
    pos2 = command_buf.find_first_of (" ", pos1+1);
    if ( pos1 == string::npos )
        return;

    par2 = command_buf.substr (pos1, pos2 - pos1);
}


int main( int argc, char **argv ) {
    if (manager.GetNumMIDIIns() == 0) {
        cout << "This program has no functionality without a MIDI in port!\n" <<
                "Press a key to quit" << endl;
        cin.get();
        return EXIT_SUCCESS;
    }
    manager.AddMIDITick(&thru);
    manager.SeqPlay();
    cout << "TYPE help TO GET A LIST OF AVAILABLE COMMANDS" << endl << endl;
    while ( command != "quit" )                     // main loop
    {
        GetCommand();                               // gets user input and parse it

        if ( command == "ports") {                  // enumerates the midi ports
            cout << "MIDI IN PORTS:" << endl;
            for ( int i = 0; i < MIDIManager::GetNumMIDIIns(); i++ )
                cout << i << ": " << MIDIManager::GetMIDIInName( i ) << endl;
            if ( MIDIManager::GetNumMIDIOuts() ) {
                cout << "MIDI OUT PORTS:" << endl;
                for ( int i = 0; i < MIDIManager::GetNumMIDIOuts(); i++ )
                    cout << i << ": " << MIDIManager::GetMIDIOutName( i ) << endl;
            }
            else
                cout << "NO MIDI OUT PORTS" << endl;
        }
        else if ( command == "outport") {               // changes the midi out port
            int port = atoi( par1.c_str() );
            int chan = -1;
            if (par2.length() > 0)
                chan = atoi( par2.c_str() );
            if (port < 0 || port >= manager.GetNumMIDIOuts())
                cout << "Invalid port number\n";
            else
            {
                thru.SetOutPort(MIDIManager::GetOutDriver(port));
                thru.SetOutChannel(chan);
                cout << "Out port for MIDI thru\n" << thru.GetOutPort() <<
                        "\tChan: " << thru.GetOutChannel() << endl;;
            }
        }
        else if ( command == "inport" )                 // changes the midi in port
        {
            int port = atoi( par1.c_str() );
            int chan = -1;
            if (par2.length() > 0)
                chan = atoi( par2.c_str() );
            if (port < 0 || port >= manager.GetNumMIDIIns())
                cout << "Invalid port number\n";
            else
            {
                thru.SetInPort(MIDIManager::GetInDriver(port));
                thru.SetInChannel(chan);
                cout << "In port for MIDI thru\n" << thru.GetInPort()
                     << "\tChan: " << thru.GetInChannel() << endl;
            }
        }
        else if ( command == "thru")                    // sets theMIDI thru on and off
        {
            if (par1 == "on") {
                if (thru.SetEnable(true))               // could fail if we haven't MIDI outs
                    cout << "MIDIthru on, from  in port " << thru.GetInPort() <<
                            " to out port " << thru.GetOutPort();
                else
                    cout << "MIDIthru on failed" << endl;
            }
            else if (par1 == "off") {
                thru.SetEnable(false);
                cout << "MIDI thru off" << endl;
            }
        }
        else if ( command == "program" )                // sets repeates play
        {
            MIDITimedMessage msg;
            unsigned char prog = atoi( par1.c_str() ) % 0x7f;
            unsigned char chan = (thru.GetOutChannel() == -1 ? 0 : thru.GetOutChannel());
            msg.SetProgramChange(chan, prog);

            if (thru.GetOutPort()) {       // a MIDI thru out exists
                thru.GetOutPort()->OutputMessage(msg);
                cout << "Sent program change " << prog << "on port " << thru.GetOutPort()
                     << " channel " << chan << endl;
            }
        }
        else if ( command == "print") {
            if (par1 == "on") {
                thru.GetInPort()->SetProcessor(&printer);
                cout << "Print on (port " << thru.GetInPort() << ")" << endl;
            }
            else if (par1 == "off") {
                thru.GetInPort()->SetProcessor(0);
                cout << "Print off (port " << thru.GetInPort() << ")" << endl;
            }
        }
        else if ( command == "status" ) {
            std::cout << "TO DO ... \n";
        /*
            int thru_in = manager.GetThruInPort();
            int thru_out = manager.GetThruOutPort();
            cout << "\nPROGRAM STATUS:\n";
            cout << "MIDI in port:             " << thru.GetMIDIInName(thru_in) << "\n";
            cout << "MIDI in channel:          ";
            if (manager.GetThruInChannel() == -1)
                cout << "all\n";
            else
                cout << manager.GetThruInChannel() << "\n";
            cout << "MIDI out port (for thru): ";
            cout << (thru_out == -1 ? "not assigned" : manager.GetMIDIOutName(thru_out)) << "\n";
            cout << "MIDI out channel:         ";
            if (manager.GetThruOutChannel() == -1)
                cout << "all\n";
            else
                cout << manager.GetThruOutChannel() << "\n";
            cout << "MIDI thru:                ";
            cout << (manager.GetThruEnable() ? "ON\n" : "OFF\n");
            cout << "Print incoming messages : ";
            cout << ((manager.GetInDriver(manager.GetThruInPort())->GetProcessor() != 0) ? "ON\n" : "OFF\n");


*/
        }
        else if ( command == "help")                    // prints help screen
            cout << helpstring;
        else if ( command != "quit" )                   // unrecognized command
            cout << "Unrecognized command" << endl;
    }
    CoUninitialize();
    return EXIT_SUCCESS;
}

