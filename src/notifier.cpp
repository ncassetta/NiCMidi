#include "../include/notifier.h"
#include "../include/sequencer.h"


void MIDISequencerGUIEventNotifierText::Notify( const MIDISequencer *seq, MIDISequencerGUIEvent e ) {
    if( en ) {
        fprintf( f, "GUI EVENT: G=%d, SG=%d, ITEM=%d\n",
                e.GetEventGroup(),
                e.GetEventSubGroup(),
                e.GetEventItem() );

        if( e.GetEventGroup() == MIDISequencerGUIEvent::GROUP_TRANSPORT ) {
            if( e.GetEventItem() == MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT )
                fprintf( f, "MEAS %3d BEAT %3d\n",
                        seq->GetCurrentMeasure()+1,
                        seq->GetCurrentBeat()+1 );

        }
        else if( e.GetEventGroup() == MIDISequencerGUIEvent::GROUP_CONDUCTOR ) {
            if( e.GetEventItem() == MIDISequencerGUIEvent::GROUP_CONDUCTOR_TIMESIG )
                fprintf( f, "TIMESIG: %d/%d\n",
                        seq->GetState()->timesig_numerator,
                        seq->GetState()->timesig_denominator );
            if(e.GetEventItem() == MIDISequencerGUIEvent::GROUP_CONDUCTOR_TEMPO)
                fprintf( f, "TEMPO: %3.2f\n",
                        seq->GetState()->tempobpm );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////


void MIDISequencerTrackNotifier::Notify( int item ) {
    if( notifier ) {
        notifier->Notify(seq,
            MIDISequencerGUIEvent(MIDISequencerGUIEvent::GROUP_TRACK, track_num, item));
    }
}


void MIDISequencerTrackNotifier::NotifyConductor( int item ) {
    // only notify conductor if we are track #0
    if( notifier && track_num==0 ) {
      notifier->Notify( seq,
            MIDISequencerGUIEvent( MIDISequencerGUIEvent::GROUP_CONDUCTOR, 0, item ));
    }
}

//////////////////////////////////////////////////////////////////////////////


#ifdef _WIN32

MIDISequencerGUIEventNotifierWin32::MIDISequencerGUIEventNotifierWin32 (
    HWND w, DWORD msg, WPARAM wparam_value_ ) :
    dest_window ( w ), window_msg ( msg ), wparam_value ( wparam_value_ ), en ( true ) {}


// NEW BY NC: auto sets window_msg and wparam_value
MIDISequencerGUIEventNotifierWin32::MIDISequencerGUIEventNotifierWin32 ( HWND w ) :
    dest_window ( w ), window_msg ( GetSafeSystemMsgId() ), wparam_value ( 0 ), en ( true ) {}


void MIDISequencerGUIEventNotifierWin32::Notify ( const MIDISequencer *seq, MIDISequencerGUIEvent e ) {
    if ( en )
        PostMessage ( dest_window, window_msg, wparam_value, ( unsigned long ) e );
}


#endif // _WIN32
