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

#include "jdkmidi/world.h"
#include "jdkmidi/manager.h"




MIDIManager::MIDIManager(
    MIDISequencerGUIEventNotifier *n,
    MIDISequencer *seq_ ) :
    sequencer(seq_),
    notifier( n ),
    sys_time_offset(0),
    seq_time_offset(0),
    play_mode(false),
    stop_mode(true),
    repeat_play_mode(false),
    repeat_start_measure(0),
    repeat_end_measure(0)

{
    RtMidiOut temp_MIDI_out;
    for (int i = 0; i < temp_MIDI_out.getPortCount(); i++)
        MIDI_outs.push_back(new MIDIOutDriver(i));
    //driver->SetTickProc( this );
}


void MIDIManager::Reset() {
    SeqStop();
    sys_time_offset = 0;
    seq_time_offset = 0;
    play_mode = false;
    stop_mode = true;
    if( notifier )
        notifier->Notify( sequencer, MIDISequencerGUIEvent( MIDISequencerGUIEvent::GROUP_ALL ) );
}


// to set and get the current sequencer
void MIDIManager::SetSeq ( MIDISequencer *seq ) {
    if ( notifier )
        notifier->Notify ( sequencer, MIDISequencerGUIEvent ( MIDISequencerGUIEvent::GROUP_ALL ) );
    sequencer = seq;
}


// to manage the playback of the sequencer
void MIDIManager::SeqPlay()
{
    seq_time_offset = ( unsigned long ) sequencer->GetCurrentTimeInMs();
    sys_time_offset = jdks_get_system_time_ms();

    stop_mode = false;
    play_mode = true;

    if ( notifier )
    {
        notifier->Notify ( sequencer,
                           MIDISequencerGUIEvent (
                               MIDISequencerGUIEvent::GROUP_TRANSPORT,
                               0,
                               MIDISequencerGUIEvent::GROUP_TRANSPORT_MODE
                           ) );
    }
}


// to manage the repeat playback of the sequencer
void MIDIManager::SetRepeatPlay ( bool flag, unsigned long start_measure, unsigned long end_measure ) {
    // shut off repeat play while we muck with values
    repeat_play_mode = false;
    repeat_start_measure = start_measure;
    repeat_end_measure = end_measure;
    // set repeat mode flag to how we want it.
    repeat_play_mode = flag;
}

void MIDIManager::SeqStop() {
    play_mode = false;
    stop_mode = true;

    if ( notifier )
    {
        notifier->Notify ( sequencer,
                           MIDISequencerGUIEvent (
                               MIDISequencerGUIEvent::GROUP_TRANSPORT,
                               0,
                               MIDISequencerGUIEvent::GROUP_TRANSPORT_MODE
                           ) );
    }
}


void MIDIManager::TimeTick( unsigned long sys_time_ ) {
    if( play_mode )
        TimeTickPlayMode(sys_time_);
    else if( stop_mode )
        TimeTickStopMode(sys_time_);
}


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


void MIDIManager::TimeTickStopMode( unsigned long sys_time_ ) {
}

