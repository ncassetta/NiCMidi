/*
  A command line example of the features of the AdvancedRecorder
  class. It creates an instance of the class and allows the user
  to interact with it. You can load MIDI files and play them
  changing a lot of parameters (muting, soloing, and transposing
  tracks; jumping from a time to another, changing the tempo, etc.)
  even while the sequencer is playing.
  Compile it together with functions.cpp, which contains command
  line I/O functions.

  Copyright (C) 2013 - 2019 N.Cassetta
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


#include "../include/advancedsequencer.h"
#include "../include/advancedrecorder.h"
#include "functions.h"

using namespace std;


//////////////////////////////////////////////////////////////////
//                        G L O B A L S                         //
//////////////////////////////////////////////////////////////////

MIDISequencerGUINotifierText notifier;
AdvancedSequencer sequencer(&notifier);         // an AdvancedSequencer (without GUI notifier)
AdvancedRecorder recorder(&sequencer);          // an AdvancedRecorder recording tracks of the sequencer

extern string command, par1, par2;              // used by GetCommand() for parsing the user input
const char helpstring[] =                       // shown by the help command
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
   dump rec/sec [trk]  : Prints a dump of all midi events in the recorder or\n\
                         sequencer (in the track trk)\n\
   outport track port  : Sets the MIDI port for the given track\n\
   solo track          : Soloes the given track. All other tracks are muted\n\
   unsolo              : Unsoloes all tracks\n\
   mute track          : Toggles on and off the muting of given track.\n\
   unmute              : Unmutes all tracks\n\
   tscale scale        : Sets global tempo scale. scale is in percent\n\
                         (ex. 200 = twice faster, 50 = twice slower)\n\
   vscale track scale  : Sets velocity scale for given track (scale as above)\n\
   trans track amount  : Sets transpose for given track.\n\
                         amount is in semitones (positive or negative)\n\
   thru on/off         : Sets the MIDI thru on and off.\n\
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
    cout << "TYPE help TO GET A LIST OF AVAILABLE COMMANDS" << endl << endl;
    while ( command != "quit" ) {                   // main loop
        GetCommand();                               // gets user input and parses it (see functions.cpp)

        if( command == "load" ) {                   // loads a file
            if (sequencer.Load(par1.c_str()))
                cout << "Loaded file " << par1 << endl;
            else
                cout << "Error loading file" << endl;
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
            recorder.Start();
            cout << "Recorder started at measure: " << sequencer.GetCurrentMeasure() << ":"
                 << sequencer.GetCurrentBeat() << endl;
        }
        else if (command == "loop") {               // sets repeated play
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
            recorder.Stop();
            cout << "Recorder stopped at measure: " << sequencer.GetCurrentMeasure() << ":"
                 << sequencer.GetCurrentBeat() << endl;
        }
        else if (command == "rew") {                // stops and rewinds to time 0
            recorder.GoToZero();
            cout << "Rewind to 0:0" << endl;
        }
        else if (command == "goto") {               // goes to meas and beat
            int measure = atoi(par1.c_str());
            int beat = atoi (par2.c_str());
            if (measure < 0 || measure > sequencer.GetNumMeasures() - 1)
                cout << "Invalid position" << endl;
            else {
                recorder.GoToMeasure(measure, beat);
                cout << "Actual position moved to: " << sequencer.GetCurrentMeasure() << ":"
                     << sequencer.GetCurrentBeat() << endl;
            }
        }
        else if (command == "dump") {               // prints a dump of the sequencer contents
            MIDIMultiTrack* tracks = 0;
            if (par1 == "seq")
                tracks = sequencer.GetMultiTrack();
            else if (par1 == "rec")
                tracks = recorder.GetMultiTrack();
            if (par2.size() == 0)
                DumpMIDIMultiTrackWithPauses(tracks);
            else {
                int trk_num = atoi(par1.c_str());
                if (tracks->IsValidTrackNumber(trk_num)) {
                    MIDITrack* trk = tracks->GetTrack(trk_num);
                    DumpMIDITrackWithPauses(trk, trk_num);
                }
                else
                    cout << "Invalid track number" << endl;
            }
        }
        else if (command == "solo") {               // soloes a track
            int track = atoi(par1.c_str());
            sequencer.SetTrackSolo(track);
            cout << "Soloed track " << track << endl;
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
        else if (command == "unsolo") {             // unsoloes all tracks
            sequencer.UnSoloTrack();
            cout << "Unsoloed all tracks" << endl;
        }
        else if (command == "mute") {               // mutes a track
            int track = atoi(par1.c_str());
            sequencer.SetTrackMute(track, !sequencer.GetTrackMute(track));
            if (sequencer.GetTrackMute(track))
                cout << "Muted track " << track << endl;
            else
                cout << "Unmuted track " << track << endl;
        }
        else if (command == "unmute") {             // unmutes a track
            sequencer.UnmuteAllTracks();
            cout << "Unmuted all tracks" << endl;
        }
        else if (command == "tscale") {             // scales playback tempo
            int scale = atoi(par1.c_str());
            sequencer.SetTempoScale(scale);
            cout << "Tempo scale : " << scale << "%  " <<
                    " Effective tempo: " << sequencer.GetTempoWithScale() << " bpm" << endl;
        }
        else if (command == "vscale") {             // scales velocity for a track
            int track = atoi(par1.c_str());
            int scale = atoi(par2.c_str());
            sequencer.SetTrackVelocityScale(track, scale);
            cout << "Track " << track << " velocity scale set to " << scale << "%" << endl;
        }
        else if (command == "trans") {              // transposes a track
            int track = atoi(par1.c_str());
            int amount = atoi(par2.c_str());
            sequencer.SetTrackTranspose(track, amount);
            cout << "Track " << track << " transposed by " << amount << " semitones " << endl;
        }
        else if (command == "tshift") {             // sets the time shift (in ticks) of a track
            int track = atoi(par1.c_str());
            int amount = atoi(par2.c_str());
            sequencer.SetTrackTimeShift(track, amount);
            cout << "Track " << track << " time shifted by " << amount << " MIDI ticks" << endl;
        }
        else if (command == "thru") {               // toggles MIDI thru on and off
            if (par1 == "on") {
                sequencer.SetMIDIThruEnable(true);
                if (sequencer.GetMIDIThruEnable()) {
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
                    cout <<"Sysex: " << (trk->HasSysex() ? "Yes " : "No  ") << "Events: "
                         << trk->GetNumEvents() << endl;
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
