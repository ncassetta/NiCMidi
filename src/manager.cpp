/*
 *  libjdkmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  *** RELEASED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) April 27, 2004 ***
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// updated to reflect changes in jdksmidi

#include <iostream>         // DEBUG! togliere

#include "../include/world.h"
#include "../include/manager.h"



std::vector<std::string> MIDIManager::MIDI_out_names;
std::vector<std::string> MIDIManager::MIDI_in_names;






MIDIManager::MIDIManager(MIDISequencerGUINotifier *n, MIDISequencer *seq_) :
    sequencer(seq_),
    notifier(n),
    sys_time_offset(0),
    seq_time_offset(0),
    play_mode(false),

    thru_enable(false),
    thru_input(-1),
    thru_output(-1),

    repeat_play_mode(false),
    repeat_start_measure(0),
    repeat_end_measure(0),

    open_policy(AUTO_OPEN)

{
#ifdef WIN32    //TODO: this is temporary, needed by WIstd::atomic<unsigned char> busy;        // TODO: use the mutex???NDOWS10
     CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif // WIN32
    try {
        RtMidiOut temp_MIDI_out;
        for (unsigned int i = 0; i < temp_MIDI_out.getPortCount(); i++) {
            MIDI_outs.push_back(new MIDIOutDriver(i));
            MIDI_out_names.push_back(temp_MIDI_out.getPortName(i));
        }
        RtMidiIn temp_MIDI_in;
        for (unsigned int i = 0; i < temp_MIDI_in.getPortCount(); i++) {
            MIDI_ins.push_back(new MIDIInDriver(i));
            MIDI_in_names.push_back(temp_MIDI_in.getPortName(i));
        }
    }
    catch (RtMidiError &error) {
        error.printMessage();
        exit(EXIT_FAILURE);
    }
    timer = new MIDITimer();
    timer->SetMIDITick(TickProc, this);
    if (MIDI_ins.size() > 0 && MIDI_outs.size() > 0)    // set MIDI thru on ports 0 (if they exists)
        SetThruPorts(0, 0);
}


MIDIManager::~MIDIManager() {
    for (unsigned int i = 0; i < MIDI_outs.size(); i++)
        delete MIDI_outs[i];
    for (unsigned int i = 0; i < MIDI_ins.size(); i++)
        delete MIDI_ins[i];

#ifdef WIN32
    CoUninitialize();
#endif // WIN32
}


void MIDIManager::Reset() { // doesn't reset open_policy
    SeqStop();
    sys_time_offset = 0;
    seq_time_offset = 0;

    thru_enable = false;

    repeat_play_mode = false;
    repeat_start_measure = 0;
    repeat_end_measure = 0;
    if( notifier )
        notifier->Notify(MIDISequencerGUIEvent( MIDISequencerGUIEvent::GROUP_ALL));
    for(unsigned int i = 0; i < MIDI_outs.size(); i++)
        MIDI_outs[i]->Reset();
    for(unsigned int i = 0; i < MIDI_ins.size(); i++)
        MIDI_ins[i]->Reset();

    if (MIDI_ins.size() > 0 && MIDI_outs.size() > 0)
        SetThruPorts(0, 0);
}


void MIDIManager::SetSequencer (MIDISequencer *seq) {
    if (notifier)
        notifier->Notify (MIDISequencerGUIEvent (MIDISequencerGUIEvent::GROUP_ALL));
    sequencer = seq;
}


void MIDIManager::OpenOutPorts() {
    for (unsigned int i = 0; i < MIDI_outs.size(); i++)
        if (!MIDI_outs[i]->IsPortOpen())
            MIDI_outs[i]->OpenPort();
}


void MIDIManager::CloseOutPorts() {
    for (unsigned int i = 0; i < MIDI_outs.size(); i++)
        if (MIDI_outs[i]->IsPortOpen())
            MIDI_outs[i]->ClosePort();
}


// to manage the playback of the sequencer
void MIDIManager::SeqPlay() {
    if (sequencer) {

        std::cout << "The MIDIManager is starting the sequencer ..." << std::endl;

        OpenOutPorts();

        seq_time_offset = (unsigned long) sequencer->GetCurrentTimeInMs();
        sys_time_offset = timer->GetSysTimeMs();
        //stop_mode = false;
        play_mode = true;
        timer->Start();

        if (notifier) {
            notifier->Notify ( MIDISequencerGUIEvent (
                                   MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                   0,
                                   MIDISequencerGUIEvent::GROUP_TRANSPORT_MODE
                               ) );
        }
    }
}


void MIDIManager::SeqStop() {
    if (sequencer && play_mode == true) {
        timer->Stop();
        play_mode = false;
        //stop_mode = true;

        if (notifier) {
            notifier->Notify ( MIDISequencerGUIEvent (
                                   MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                   0,
                                   MIDISequencerGUIEvent::GROUP_TRANSPORT_MODE
                               ) );
        }
        if (open_policy == AUTO_OPEN)
            CloseOutPorts();

        std::cout << "The MidiManager has stopped the sequencer" << std::endl;
    }
}


// to manage the repeat playback of the sequencer
void MIDIManager::SetRepeatPlay ( bool on_off, unsigned int start_measure, unsigned int end_measure ) {
        // shut off repeat play while we muck with values
    repeat_play_mode = false;
    repeat_start_measure = start_measure;
    repeat_end_measure = end_measure;
        // set repeat mode flag to how we want it.
    repeat_play_mode = on_off;
}


// to set the MIDI thru
bool MIDIManager::SetThruEnable(bool f) {
    if (thru_input == -1 || thru_output == -1)      // we have not set the thru ports yet
        return false;
    if (thru_enable == f)                           // trying to set the same
        return true;
    bool ret = MIDI_ins[thru_input]->SetThruEnable(f);
    thru_enable = MIDI_ins[thru_input]->GetThruEnable();
    if (thru_enable) {
        MIDI_ins[thru_input]->OpenPort();
        MIDI_outs[thru_output]->OpenPort();
    }
    else if (ret) {
        MIDI_ins[thru_input]->ClosePort();
        MIDI_outs[thru_output]->AllNotesOff();
        MIDI_outs[thru_output]->ClosePort();
    }
    return ret;
}

// TODO: rearrange this to avoid closing and reopening same ports
// (for now leave. it gave errors)
bool MIDIManager::SetThruPorts(unsigned int in_port, unsigned int out_port) {
    if (in_port >= MIDI_ins.size() || out_port >= MIDI_outs.size())
        return false;                               // invalid port number: the function fails

    if (in_port == thru_input && out_port == thru_output)
        return true;                                // trying to assign same ports: nothing to do

    if (in_port == thru_input) {                    // same input port: avoids closing and reopening
        MIDI_ins[thru_input]->SetThruEnable(false); // locks the driver and mutes the actual out port
        MIDI_ins[thru_input]->SetThruEnable(thru_enable, MIDI_outs[out_port]);
                                                    // sets the new port
        thru_output = out_port;                     // updates the manager status
    }
    else {                                          // we must act both on in and out port
        bool was_enabled = thru_enable;
        SetThruEnable(false);                       // closes both actual ports
        MIDI_ins[in_port]->SetThruEnable(false, MIDI_outs[out_port]);
                                                    // sets the new output on the new input
        thru_input = in_port;                       // updates the manager status
        thru_output = out_port;
        SetThruEnable(was_enabled);
    }
    return true;
}

/* OLD WORKING FUNCTION
bool MIDIManager::SetThruPorts(unsigned int in_port, unsigned int out_port) {
    if (in_port >= MIDI_ins.size() || out_port >= MIDI_outs.size())
        return false;

    std::cout << "Step 1" << std::endl;
    if (thru_input != -1) {                         // an old MIDI thru input was set
        MIDI_ins[thru_input]->SetThruEnable(false); // turn off the MIDI thru
        MIDI_ins[thru_input]->ClosePort();          // and close the port
    }

    std::cout << "Step 2" << std::endl;

    if (thru_output != -1) {                        // an old MIDI thru output was set
        MIDI_outs[thru_output]->AllNotesOff();      // cut off sounding notes
// TODO: insert a wait?
        MIDI_outs[thru_output]->ClosePort();        // and close it
    }

    MIDI_ins[in_port]->SetThruEnable(false, MIDI_outs[out_port]);
                                                    // set the new MIDI thru
    MIDI_ins[in_port]->OpenPort();
    MIDI_outs[out_port]->OpenPort();
    MIDI_ins[in_port]->SetThruEnable(thru_enable);
    thru_input = in_port;
    thru_output = out_port;
    return true;
}
*/

void MIDIManager::SetThruChannels(char in_chan, char out_chan) {
    if (in_chan != MIDI_ins[thru_input]->GetThruChannel())
        MIDI_ins[thru_input]->SetThruChannel(in_chan);
    if (out_chan != MIDI_outs[thru_output]->GetThruChannel())
        MIDI_outs[thru_output]->SetThruChannel(out_chan);
}


void MIDIManager::AllNotesOff() {
    for (unsigned int i = 0; i < MIDI_outs.size(); i++)
        MIDI_outs[i]->AllNotesOff();
}


void MIDIManager::TickProc( tMsecs sys_time_, void* p ) {
/*
    MIDIManager* manager = static_cast<MIDIManager *>(p);
    for (unsigned int i = 0; i < manager->tick_procedures.size(); i++)
        manager->tick_procedures[i](MIDITimer::GetSysTimeMs(), p);
*/

    MIDIManager* manager = static_cast<MIDIManager *>(p);
    if( manager->play_mode )
        manager->SequencerPlayProc(sys_time_);

    //std::cout << "MIDIManager TickProc" << std::endl;
}

/*  OLD WORKING FUNCTION WITH QUEUE
void MIDIManager::TimeTickPlayMode( unsigned long sys_time_ )
{
    static unsigned long old_sys_ = 0;     // debug


    double sys_time = (double)sys_time_ - (double)sys_time_offset;
    float next_event_time = 0.0;
    int ev_track;
    MIDITimedBigMessage ev;

    // if we are in repeat mode, repeat if we hit end of the repeat region
    if( repeat_play_mode
            && sequencer->GetCurrentMeasure()>=repeat_end_measure
      )
    {
        // yes we hit the end of our repeat block
        // shut off all notes on
        MIDI_outs[0]->AllNotesOff();

        // now move the sequencer to our start position

        sequencer->GoToMeasure( repeat_start_measure );

        // our current raw system time is now the new system time offset

        sys_time_offset = sys_time_;

        sys_time = 0;

        // the sequencer time offset now must be reset to the
        // time in milliseconds of the sequence start point

        seq_time_offset = (unsigned long)sequencer->GetCurrentTimeInMs();
    }

    // find all events that exist before or at this time,
    // but only if we have space in the output queue to do so!

    // also limit ourselves to 100 midi events max.

    int output_count=100;

    while(
        sequencer->GetNextEventTimeMs( &next_event_time )
        && (next_event_time-seq_time_offset)<=sys_time
        && MIDI_outs[0]->CanOutputMessage()
        && (--output_count)>0
    )
    {
        // found an event! get it!

        if( sequencer->GetNextEvent( &ev_track, &ev ) )
        {
            // ok, tell the driver the send this message now

            MIDI_outs[0]->OutputMessage( ev );
            ev.Clear();     // we always must delete eventual sysex pointers before reassigning to ev
        }
    }

    // auto stop at end of sequence

    if( !(repeat_play_mode && sequencer->GetCurrentMeasure()>=repeat_end_measure) &&
            !sequencer->GetNextEventTimeMs( &next_event_time ) )
    {
        // no events left
        stop_mode = true;
        play_mode = false;

        if( notifier )
        {
            notifier->Notify( sequencer,
                              MIDISequencerGUIEvent(
                                  MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                  0,
                                  MIDISequencerGUIEvent::GROUP_TRANSPORT_MODE
                              ) );

            notifier->Notify( sequencer,
                              MIDISequencerGUIEvent(
                                  MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                  0,
                                  MIDISequencerGUIEvent::GROUP_TRANSPORT_ENDOFSONG
                              ) );

        }
    }

// THIS IS THE OLD driver time tick

    // feed as many midi messages from out_queue to the hardware out port
    // as we can

    while(MIDI_outs[0]->GetQueue()->CanGet()) {
        // use the Peek() function to avoid allocating memory for
        // a duplicate sysex

        if(MIDI_outs[0]->HardwareMsgOut( *(MIDI_outs[0]->GetQueue()->Peek() ) ) == true ) {
            // ok, got and sent a message - update our out_queue now
            // added by me: we always must delete eventual sysex pointers before reassigning to ev ????? TODO
            MIDI_outs[0]->GetQueue()->Next();
        }
        else {
            // cant send any more, stop now.
            break;
        }
    }
}
*/


// NEW FUNCTION WITH DIRECT SEND WITH HardwareMsgOut
void MIDIManager::SequencerPlayProc( tMsecs sys_time_ )
{
    double sys_time = (double)sys_time_ - (double)sys_time_offset;
    double next_event_time = 0.0;
    int ev_track;
    MIDITimedMessage ev;

    // if we are in repeat mode, repeat if we hit end of the repeat region
    if(repeat_play_mode && sequencer->GetCurrentMeasure() >= repeat_end_measure) {
        // yes we hit the end of our repeat block
        // shut off all notes on
        MIDI_outs[0]->AllNotesOff();

        // now move the sequencer to our start position
        sequencer->GoToMeasure( repeat_start_measure );

        // our current raw system time is now the new system time offset
        sys_time_offset = sys_time_;
        sys_time = 0;

        // the sequencer time offset now must be reset to the
        // time in milliseconds of the sequence start point
        seq_time_offset = (unsigned long)sequencer->GetCurrentTimeInMs();
    }

    // find all events that exist before or at this time,
    // but only if we have space in the output queue to do so!
    // also limit ourselves to 100 midi events max.
    int output_count=100;

    while(
        sequencer->GetNextEventTimeMs( &next_event_time )
        && (next_event_time - seq_time_offset) <= sys_time
        && (--output_count)>0 ) {

        // found an event! get it!
        if(sequencer->GetNextEvent(&ev_track, &ev) &&
           !ev.IsMetaEvent()) {

            // ok, tell the driver the send this message now
            MIDI_outs[0]->OutputMessage(ev);
        }
    }

    // auto stop at end of sequence
    if( !(repeat_play_mode && sequencer->GetCurrentMeasure()>=repeat_end_measure) &&
            !sequencer->GetNextEventTimeMs( &next_event_time ) ) {
        // no events left
        //stop_mode = true;
        play_mode = false;

        if(notifier) {
            notifier->Notify( MIDISequencerGUIEvent(
                                  MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                  0,
                                  MIDISequencerGUIEvent::GROUP_TRANSPORT_MODE
                              ) );

            notifier->Notify( MIDISequencerGUIEvent(
                                  MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                  0,
                                  MIDISequencerGUIEvent::GROUP_TRANSPORT_ENDOFSONG
                              ) );

        }
    }
}
