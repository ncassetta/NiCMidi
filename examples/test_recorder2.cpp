#include "../include/advancedsequencer.h"
#include "../include/recorder.h"
#include "../include/timer.h"
#include "../include/dump_tracks.h"


MIDISequencerGUINotifierText text_n;        // the AdvancedSequencer GUI notifier
AdvancedSequencer sequencer(&text_n);       // an AdvancedSequencer (with GUI notifier)
MIDIRecorder recorder(&sequencer);          // a MIDIRecorder //FCKX

int main( int argc, char **argv ) {

    MIDIManager::AddMIDITick(&recorder);
    //text_n.SetSequencer(&sequencer);      // This is already called by AdvancedSequencer constructor

    MIDIClockTime t = sequencer.MeasToMIDI(5,0); //endMeasure, endBeat record the first 6 beats
    recorder.SetEndRecTime(t);
    recorder.EnableTrack(1); //FCKX
    recorder.SetTrackRecChannel(1,-1);      // Can you set this? Otherwise set a specific channel

    recorder.Start();
    std::cout << "Recorder started\n";

    MIDITimer::Wait(15000);                 // Waits 15 secs: play something to record (remember to match
                                            // the input channel with the one set in SetTrackRecChannel)
    recorder.Stop();
    std::cout << "Recorder stopped\n";

    MIDITimer::Wait(1000);
    sequencer.GoToZero();                   // rewinds

    sequencer.Start();
    std::cout << "Now the sequencer plays what you have recorded\n";
    while (sequencer.IsPlaying())
        MIDITimer::Wait(50);
    std::cout << "Sequencer stopped" << std::endl;
}
