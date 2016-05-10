#include "../include/notifier.h"
#include "../include/sequencer.h"


const char MIDISequencerGUIEvent::group_names[][10] =
        { "All      ", "Conductor", "Transport", "Track    " };
        const char MIDISequencerGUIEvent::conductor_items_names[][10] =
        { "All      ", "Tempo    ", "Timesig  ", "Keysig   ", "Marker   " };
        const char MIDISequencerGUIEvent::transport_items_names[][10] =
        { "All      ", "Mode     ", "Measure  ", "Beat     ", "EndOfSong" };
        const char MIDISequencerGUIEvent::track_items_names[][10] =
        { "All      ", "Name     ", "Patch    ", "Note     ", "Volume   " };


void MIDISequencerGUIEventNotifierText::Notify( const MIDISequencer *seq, MIDISequencerGUIEvent e ) {
    if( en ) {
        ost << "GUI EVENT: " << MIDISequencerGUIEvent::group_names[e.GetEventGroup()] << " ";

        switch(e.GetEventGroup()) {
            case MIDISequencerGUIEvent::GROUP_ALL:
                break;
            case MIDISequencerGUIEvent::GROUP_TRANSPORT:
                switch (e.GetEventItem()) {
                    case MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT:
                        ost << "MEAS " << seq->GetCurrentMeasure()+1 << " " << "BEAT "<< seq->GetCurrentBeat()+1;
                    case MIDISequencerGUIEvent::GROUP_TRANSPORT_ENDOFSONG:
                        ost << "ENDOFSONG";}
                break;
            case MIDISequencerGUIEvent::GROUP_CONDUCTOR:
                switch (e.GetEventItem()) {
                    case MIDISequencerGUIEvent::GROUP_CONDUCTOR_TEMPO:
                        ost << "TEMPO: " << seq->GetState()->tempobpm ;
                    case MIDISequencerGUIEvent::GROUP_CONDUCTOR_TIMESIG:
                        ost << "TIMESIG: " << seq->GetState()->timesig_numerator << "/" <<
                        seq->GetState()->timesig_denominator;
                    case MIDISequencerGUIEvent::GROUP_CONDUCTOR_KEYSIG:
                        ost << "TIMESIG: ";
                    case MIDISequencerGUIEvent::GROUP_CONDUCTOR_MARKER:
                        ost << "MARKER: ";
                }
                break;
            case MIDISequencerGUIEvent::GROUP_TRACK:
                break;
        }
        ost << std::endl;
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
