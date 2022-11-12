/*
 *   Example file for NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2022  Nicola Cassetta
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
  A basic example which allows the user to record for 15 seconds and then listen
  the recorded messages, without no input from the computer keyboard.
*/


#include "../include/advancedsequencer.h"
#include "../include/recorder.h"
#include "../include/timer.h"
#include "../include/dump_tracks.h"


MIDISequencerGUINotifierText text_n;        // the AdvancedSequencer GUI notifier
AdvancedSequencer sequencer(&text_n);       // an AdvancedSequencer (with GUI notifier)
MIDIRecorder recorder(&sequencer);          // a MIDIRecorder //FCKX

int main( int argc, char **argv ) {
    MIDIClockTime t = sequencer.MeasToMIDI(5,0);
    recorder.SetEndRecTime(t);
    recorder.EnableTrack(1);
    recorder.SetTrackRecChannel(1,-1);      // sets recording of any channel on track 1

    // Uncomment next line if you don't want to see the notifier messages
    //text_n.SetEnable(false);
    std::cout << "When the recorder starts you can play for 15 seconds (all MIDI channels will be recorded)"
              << std::endl;
    MIDITimer::Wait(1000);

    recorder.Start();
    std::cout << "Recorder started\n";

    MIDITimer::Wait(15000);                 // Waits 15 secs: play something to record (remember to match
                                            // the input channel with the one set in SetTrackRecChannel)
    recorder.Stop();
    std::cout << "Recorder stopped\n";

    MIDITimer::Wait(1000);
    sequencer.GoToZero();                   // rewinds

    sequencer.Start();
    std::cout << "Now the sequencer plays what you have recorded\n" << std::endl;
    while (sequencer.IsPlaying())
        MIDITimer::Wait(50);
    std::cout << "Sequencer stopped" << std::endl;
}
