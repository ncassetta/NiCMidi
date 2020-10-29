/*
  A command line example of the features of the MIDIThru class.
  It creates an instance of the class and allows the user to interact
  with it. You can set the in and out ports and channels and the
  program change in the out channel; moreover you can print the
  passing MIDI messages with a MIDIrocessorPrinter.
  Compile it together with functions.cpp, which contains command
  line I/O functions.

  Copyright (C) 2016 - 2020 N.Cassetta
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
#include "functions.h"                  // helper functions for input parsing and output

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
                         the current out channel (only if a single\n\
                         channel is selected)\n\
   print on/off        : Turns on and off the printing of incoming\n\
                         messages\n\
   status              : Prints the status of the thru\n\
   help                : Prints this help screen\n\
   quit                : Exits\n\n\
   MIDI channels are numbered 0 .. 15, and -1 stands for \"all channels\"";


//////////////////////////////////////////////////////////////////
//                              M A I N                         //
//////////////////////////////////////////////////////////////////


int main( int argc, char **argv ) {
    // tests if MIDI ports are present in the system
    if (MIDIManager::GetNumMIDIIns() == 0 || MIDIManager::GetNumMIDIOuts() == 0) {
        cout << "This program has no functionality without MIDI ports!\n" <<
                "Press a key to quit\n";
        cin.get();
        return EXIT_SUCCESS;
    }
    // adds the thru to the MIDIManager queue
    MIDIManager::AddMIDITick(&thru);
    // plugs the MIDIProcessorPrinter into the metronome, so all MIDI message will
    // be printed to stdout
    thru.SetProcessor(&printer);
    cout << "TYPE help TO GET A LIST OF AVAILABLE COMMANDS\n\n";
    while (command != "quit")                       // main loop
    {
        GetCommand();                               // gets user input and splits it into command, par1, par2

        if (command == "ports") {                   // enumerates the midi ports
            if (MIDIManager::GetNumMIDIIns()) {
                cout << "\tMIDI IN PORTS:" << endl;
                for (unsigned int i = 0; i < MIDIManager::GetNumMIDIIns(); i++)
                    cout << i << ": " << MIDIManager::GetMIDIInName(i) << endl;
            }
            if (MIDIManager::GetNumMIDIOuts()) {
                cout << "MIDI OUT PORTS:\n";
                for (unsigned int i = 0; i < MIDIManager::GetNumMIDIOuts(); i++)
                    cout << i << ": " << MIDIManager::GetMIDIOutName(i) << endl;
            }
        }
        else if (command == "outport") {            // changes the midi out port
            int port = atoi(par1.c_str());
            int chan = -1;                              // all channels
            if (par2.length() > 0)
                chan = atoi(par2.c_str()) & 0x0f;
            if (port < 0 || (unsigned)port >= MIDIManager::GetNumMIDIOuts())
                cout << "Invalid port number" << endl;
            else {
                thru.SetOutPort(port);
                thru.SetOutChannel(chan);
                cout << "Out port for MIDI thru:\n" << MIDIManager::GetMIDIOutName(thru.GetOutPort())
                     << "\tChan: " << thru.GetOutChannel() << endl;
            }
        }
        else if (command == "inport") {             // changes the midi in port
            int port = atoi(par1.c_str());
            int chan = -1;
            if (par2.length() > 0)
                chan = atoi(par2.c_str()) & 0x0f;
            if (port < 0 || (unsigned)port >= MIDIManager::GetNumMIDIIns())
                cout << "Invalid port number" << endl;
            else {
                thru.SetInPort(port);
                thru.SetInChannel(chan);
                cout << "In port for MIDI thru:\n" << MIDIManager::GetMIDIInName(thru.GetInPort())
                     << "\tChan: " << thru.GetInChannel() << endl;
            }
        }
        else if (command == "thru") {               // sets the MIDI thru on and off
            if (par1 == "on") {
                thru.Start();
                cout << "MIDIthru on, from in port " << MIDIManager::GetMIDIInName(thru.GetInPort())
                     << " to out port " << MIDIManager::GetMIDIOutName(thru.GetOutPort()) << endl;
            }
            else if (par1 == "off") {
                thru.Stop();
                cout << "MIDI thru off" << endl;
            }
        }
        else if (command == "program") {            // sets the MIDI program
            unsigned int prog = atoi(par1.c_str()) & 0x7f;      // program number
            int chan = thru.GetOutChannel();
            if (chan == -1)                                     // OUT is set on any channel
                chan = thru.GetInChannel();
            if (chan == -1)                                     // both are on any channel: no effect
                cout << "IN and OUT set to all channels. This command has no effect!" << endl;
            else {
                MIDITimedMessage msg;                           // new message
                msg.SetProgramChange(chan, prog);               // sets it as a program change
                MIDIManager::GetOutDriver(thru.GetOutPort())->OutputMessage(msg);
                                                                // sends it to the out port
                cout << "Sent program change " << prog << " on port " << MIDIManager::GetMIDIInName(thru.GetOutPort())
                     << " channel " << chan << endl;
            }
        }
        else if (command == "print") {              // sets the printing of passing events on and off
            if (par1 == "on") {
                printer.SetPrint(true);
                cout << "Print on (port " << MIDIManager::GetMIDIOutName(thru.GetInPort()) << ")" << endl;
            }
            else if (par1 == "off") {
                printer.SetPrint(false);
                cout << "Print off" << endl;
            }
        }
        else if (command == "status") {             // prints the general thru status
            cout << "\nTHRU STATUS:" << endl;
            cout << "MIDI in port:             " << MIDIManager::GetMIDIInName(thru.GetInPort()) << endl;
            cout << "MIDI in channel:          ";
            if (thru.GetInChannel() == -1)
                cout << "all" << endl;
            else
                cout << thru.GetInChannel() << endl;
            cout << "MIDI out port:            " << MIDIManager::GetMIDIOutName(thru.GetOutPort()) << endl;
            cout << "MIDI out channel:         ";
            if (thru.GetOutChannel() == -1)
                cout << "all" << endl;
            else
                cout << thru.GetOutChannel() << endl;
            cout << "MIDI thru:                ";
            cout << (thru.IsPlaying() ? "ON" : "OFF") << endl;
            cout << "Print incoming messages : ";
            cout << (printer.GetPrint() ? "ON" : "OFF") << endl;
        }
        else if (command == "help")                 // prints help screen
            cout << helpstring;
        else if (command != "quit" )                // unrecognized command
            cout << "Unrecognized command" << endl;
    }
    return EXIT_SUCCESS;
}
