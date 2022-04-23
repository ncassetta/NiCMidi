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
  A command line example of the features of the MIDIRecorder class.
  You can record MIDI content through a MIDI in port in your system
  and store the recorded content in an AdvancedSequencer.
  Requires functions.cpp, which contains command line I/O functions.
*/


#include "../include/advancedsequencer.h"
#include "../include/recorder.h"
#include "../include/manager.h"
#include "../include/filewritemultitrack.h"
#include "functions.h"                  // helper functions for input parsing and output


using namespace std;


//////////////////////////////////////////////////////////////////
//                        G L O B A L S                         //
//////////////////////////////////////////////////////////////////

MIDISequencerGUINotifierText text_n;        // the AdvancedSequencer notifier
AdvancedSequencer sequencer(&text_n);       // an AdvancedSequencer (with GUI notifier)
MIDIRecorder recorder(&sequencer);          // a MIDIRecorder

extern string command, par1, par2;          // used by GetCommand() for parsing the user input
char filename[200];                         // used for saving files

const char helpstring[] =                   // shown by the help command
"\nAvailable commands:\n\
   load filename          : Loads the file into the sequencer; you must specify\n\
                          : the file name with .mid extension\n\
   save filename          : Save the current multitrack into a file; If the\n\
                          : file name is not specified you must type it with\n\
                          : .mid extension)\n\
   ports                  : Enumerates MIDI In and OUT ports\n\
   play                   : Starts playback from current time\n\
   stop                   : Stops playback\n\
   rec on/off             : Enable/disable recording\n\
   addtrack [n]           : Insert a new track (if n is not given appends\n\
                            it)\n\
   deltrack [n]           : Deletes a track (if n is not given deletes the\n\
                            last track)\n\
   enable trk [chan]      : Enable track trk for recording (if you don't specify\n\
                            the channel this will be the track channel if the\n\
                            track is a channel track or all channels\n\
   disable [trk]          : Disable track trk for recording (if you omit trk disables\n\
                          : all tracks)\n\
   recstart [meas] [beat] : Sets the recording start time from meas and beat\n\
                          : (numbered from 0) If you don't specify anything from 0.\n\
   recend[meas] [beat]    : Sets the recording end time to meas and beat. If you\n\
                            don't specify anything to infinite.\n\
   undo                   : Restore the sequence content before the recording.\n\
                            You have multiple undo levels.\n\
   rew                    : Goes to the beginning (stops the playback)\n\
   goto meas [beat]       : Moves current time to given meas and beat\n\
                            (numbered from 0)\n\
   dumps [trk]            : Prints a dump of all midi events in the sequencer\n\
                            (or in its track trk)\n\
   dumpr [trk]            : Prints a dump of all midi events in the recorder\n\
                            (or in its track trk)\n\
   inport track port      : Sets the MIDI in port for the given track\n\
   outport track port     : Sets the MIDI out port for the given track\n\
   program track p        : Sets the MIDI program (patch) for the given track\n\
   volume track v         : Sets the MIDI volume for the given track\n\
   trackinfo [v]          : Shows info about all tracks of the file. If you\n\
                            add the v the info are more complete.\n\
   b                      : (backward) Moves current time to the previous measure\n\
   f                      : (forward) Moves current time to the next measure\n\
   help                   : Prints this help screen\n\
   quit                   : Exits\n\
The recording related commands can be given only when the sequencer is stopped,\n\
other commands even during playback\n";



//////////////////////////////////////////////////////////////////
//                              M A I N                         //
//////////////////////////////////////////////////////////////////


int main (int argc, char **argv) {
    MIDIManager::AddMIDITick(&recorder);
    text_n.SetSequencer(&sequencer);
    cout << "TYPE help TO GET A LIST OF AVAILABLE COMMANDS" << endl << endl;
    while (command != "quit") {                     // main loop
        GetCommand();                               // gets user input and parses it (see functions.cpp)

        if(command == "")                           // empty command
            continue;
        if(command == "load") {                     // loads a file
            if (sequencer.Load(par1.c_str())) {
                cout << "Loaded file " << par1 << endl;
                recorder.GetMultiTrack()->SetClksPerBeat(sequencer.GetClksPerBeat());
            }
            else
                cout << "Error loading file" << endl;
        }
        else if(command == "save") {                // save the multitrack contents
            if (par1.size() > 0)
                strcpy (filename, par1.c_str());
            if (strlen(filename) > 0) {
                if (WriteMIDIFile(filename, 1, sequencer.GetMultiTrack()))
                    cout << "File saved" << endl;
                else
                    cout << "Error writing file" << endl;
            }
            else
                cout << "File name not defined" << endl;
        }
        else if (command == "ports") {              // enumerates the midi ports
            if (MIDIManager::GetNumMIDIIns()) {
                cout << "MIDI IN PORTS:" << endl;
                for (unsigned int i = 0; i < MIDIManager::GetNumMIDIIns(); i++)
                    cout << i << ": " << MIDIManager::GetMIDIInName(i) << endl;
            }
            else
                cout << "NO MIDI IN PORTS" << endl;
            if (MIDIManager::GetNumMIDIOuts()) {
                cout << "MIDI OUT PORTS:" << endl;
                for (unsigned int i = 0; i < MIDIManager::GetNumMIDIOuts(); i++)
                    cout << i << ": " << MIDIManager::GetMIDIOutName(i) << endl;
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
        else if(command == "rec") {                 // starts/stops recording
            if (par1 == "on") {
                recorder.Start();
                cout << "Recording on" << endl;
            }
            else if (par1 == "off") {
                recorder.Stop();
                cout << "Recording off" << endl;
                //sequencer.Load(recorder.GetMultiTrack());
            }
            else
                cout << "You must specify on or off" << endl;
        }
        else if(command == "addtrack") {            // inserts a track into the multitrack
            int trk_num = (par1.size() ? atoi(par1.c_str()) : -1);
            if (!sequencer.GetMultiTrack()->IsValidTrackNumber(trk_num) && trk_num != -1)
                cout << "Invalid parameters" << endl;
            else {
                sequencer.InsertTrack(trk_num);
                recorder.InsertTrack(trk_num);
                if (trk_num == -1) trk_num = sequencer.GetNumTracks() - 1;
                cout << "Track inserted at position " << trk_num;
            }
        }
        else if(command == "deltrack") {            // deletes a track from the multitrack
            int trk_num = (par1.size() ? atoi(par1.c_str()) : -1);
            if (!sequencer.GetMultiTrack()->IsValidTrackNumber(trk_num) && trk_num != -1)
                cout << "Invalid parameters" << endl;
            else {
                sequencer.DeleteTrack(trk_num);
                recorder.DeleteTrack(trk_num);
                if (trk_num == -1) trk_num = sequencer.GetNumTracks();
                cout << "Deleted track " << trk_num;
            }
        }
        else if(command == "enable") {              // enables a track for recording
            int trk_num = atoi(par1.c_str());
            int chan = (par2.size() ? atoi(par2.c_str()) : -1);
            if (!sequencer.GetMultiTrack()->IsValidTrackNumber(trk_num) || chan < -1 || chan > 15)
                cout << "Invalid parameters" << endl;
            else {
                recorder.EnableTrack(trk_num);
                if (chan != -1)
                    recorder.SetTrackRecChannel(trk_num, chan);
                if (recorder.GetTrack(trk_num)->GetRecChannel() == -1)
                    cout << "Enabled track " << par1 << " for recording from all channels" << endl;
                else
                    cout << "Enabled track " << par1 << " for recording from channel " <<
                            (int)recorder.GetTrack(trk_num)->GetRecChannel() << endl;
            }
        }
        else if(command == "disable") {             // disables a track for recording
            if (par1 == "") {
                for (unsigned int i = 0; i < sequencer.GetNumTracks(); i++)
                    recorder.DisableTrack(i);
                cout << "Disabled all tracks for recording" << endl;
            }
            else {
                int trk_num = atoi(par1.c_str());
                if (!sequencer.GetMultiTrack()->IsValidTrackNumber(trk_num))
                    cout << "Invalid parameters" << endl;
                else {
                    recorder.DisableTrack(trk_num);
                    cout << "Disabled track " << par1 << " for recording" << endl;
                }
            }
        }
        else if (command == "recstart") {           // sets the recording start time
            int meas = atoi(par1.c_str());
            int beat = atoi(par2.c_str());
            MIDIClockTime t = sequencer.MeasToMIDI(meas, beat);
            recorder.SetStartRecTime(t);
            cout << "Set starting rec time to " << t << endl;
        }
        else if (command == "recend") {           // sets the recording end time
            if (par1.size() == 0 && par2.size() == 0) {
                recorder.SetEndRecTime(TIME_INFINITE);
                cout << "Reset end rec time" << endl;
            }
            else {
                int meas = atoi(par1.c_str());
                int beat = atoi(par2.c_str());
                MIDIClockTime t = sequencer.MeasToMIDI(meas, beat);
                recorder.SetEndRecTime(t);
                cout << "Set end rec time to " << t << endl;
            }
        }
        else if (command == "undo") {               // discard last recording
            if(recorder.UndoRec())
                cout << "Undo last recording" << endl;
            else
                cout << "Undo not possible" << endl;
        }
        else if (command == "rew") {                // stops and rewinds to time 0
            sequencer.GoToZero();
            cout << "Rewind to 0:0" << endl;
        }
        else if (command == "goto") {               // goes to meas and beat
            int meas = atoi(par1.c_str());
            int beat = atoi (par2.c_str());
            if (meas < 0 || meas > sequencer.GetNumMeasures() - 1)
                cout << "Invalid position" << endl;
            else {
                sequencer.GoToMeasure(meas, beat);
                cout << "Actual position: " << sequencer.GetCurrentMeasure() << ":"
                     << sequencer.GetCurrentBeat() << endl;
            }
        }
        else if (command == "dumps") {              // prints a dump of the sequencer contents
            if (par1.size() == 0)
                DumpMIDIMultiTrackWithPauses(sequencer.GetMultiTrack()); //in functions.cpp
            else {
                int trk_num = atoi(par1.c_str());
                if (sequencer.GetMultiTrack()->IsValidTrackNumber(trk_num)) {
                    MIDITrack* trk = sequencer.GetTrack(trk_num);
                    DumpMIDITrackWithPauses(trk, trk_num);              // in functions.cpp
                }
                else
                    cout << "Invalid track number" << endl;
            }
        }
        else if (command == "dumpr") {              // prints a dump of the sequencer contents
            if (par1.size() == 0)
                DumpMIDIMultiTrackWithPauses(recorder.GetMultiTrack()); //in functions.cpp
            else {
                int trk_num = atoi(par1.c_str());
                if (recorder.GetMultiTrack()->IsValidTrackNumber(trk_num)) {
                    MIDITrack* trk = recorder.GetTrack(trk_num);
                    DumpMIDITrackWithPauses(trk, trk_num);              // in functions.cpp
                }
                else
                    cout << "Invalid track number" << endl;
            }
        }
        else if (command == "inport") {             // changes the midi in port
            int trk_num = atoi(par1.c_str());
            int port = atoi(par2.c_str());
            if (sequencer.SetTrackOutPort(trk_num, port))
                cout << "Assigned out port n. " << port << " to track " << trk_num << endl;
            else
                cout << "Invalid parameters" << endl;
        }
        else if (command == "outport") {            // changes the midi out port
            int trk_num = atoi(par1.c_str());
            int port = atoi(par2.c_str());
            if (sequencer.SetTrackOutPort(trk_num, port))
                cout << "Assigned out port n. " << port << " to track " << trk_num << endl;
            else
                cout << "Invalid parameters" << endl;
        }
        else if (command == "program") {            // changes the midi track program (patch)
            if (par1 == "" || par2 == "")
                cout << "You must specify the track and the program" << endl;
            else {
                int trk_num = atoi(par1.c_str());
                int prog = atoi(par2.c_str());
                if (!sequencer.GetMultiTrack()->IsValidTrackNumber(trk_num) || prog < 0 || prog > 127)
                    cout << "Invalid parameters" << endl;
                else {
                    MIDITrack* trk = sequencer.GetTrack(trk_num);
                    char chan = -1;
                    if (trk->GetRecChannel() != -1)
                        chan = trk->GetRecChannel();
                    else if (trk->GetChannel() != -1)
                        chan = trk->GetChannel();
                    if (chan != -1) {
                        MIDITimedMessage msg;
                        msg.SetProgramChange(chan, prog);
                        sequencer.GetTrack(trk_num)->InsertEvent(msg);
                        sequencer.UpdateStatus();
                        cout << "Assigned program " << prog << " to track " << trk_num
                             << " on channel " << (int)chan;
                    }
                    else
                        cout << "Cannot determine the track channel. Try setting its MIDI in channel" << endl;
                }
            }
        }
        else if (command == "volume") {             // changes the midi track volume
            if (par1 == "" || par2 == "")
                cout << "You must specify the track and the volume" << endl;
            else {
                int trk_num = atoi(par1.c_str());
                int vol = atoi(par2.c_str());
                if (!sequencer.GetMultiTrack()->IsValidTrackNumber(trk_num) || vol < 0 || vol > 127)
                    cout << "Invalid parameters" << endl;
                else {
                    MIDITrack* trk = sequencer.GetTrack(trk_num);
                    char chan = -1;
                    if (trk->GetRecChannel() != -1)
                        chan = trk->GetRecChannel();
                    else if (trk->GetChannel() != -1)
                        chan = trk->GetChannel();
                    if (chan != -1) {
                        MIDITimedMessage msg;
                        msg.SetVolumeChange(chan, vol);
                        sequencer.GetTrack(trk_num)->InsertEvent(msg);
                        sequencer.UpdateStatus();
                        cout << "Assigned volume " << vol << " to track " << trk_num
                             << " on channel " << (int)chan;
                    }
                    else
                        cout << "Cannot determine the track channel. Try setting its MIDI in channel" << endl;
                }
            }
        }
        else if (command == "trackinfo") {          // prints info about tracks
            bool verbose = (par1 == "v");
            DumpAllTracksAttr(sequencer.GetMultiTrack(), verbose);
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

