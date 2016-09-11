#include "../include/notifier.h"
#include "../include/sequencer.h"


const char MIDISequencerGUIEvent::group_names[][10] =
        { "All      ", "Conductor", "Transport", "Track    " };
        const char MIDISequencerGUIEvent::conductor_items_names[][10] =
        { "All      ", "Tempo    ", "Timesig  ", "Keysig   ", "Marker   ", "User    " };
        const char MIDISequencerGUIEvent::transport_items_names[][10] =
        { "All      ", "Mode     ", "Measure  ", "Beat     ", "EndOfSong", "User    " };
        const char MIDISequencerGUIEvent::track_items_names[][10] =
        { "All      ", "Name     ", "Patch    ", "Note     ", "Volume   ", "User    " };


void MIDISequencerGUINotifierText::Notify(MIDISequencerGUIEvent e) {
    if (sequencer == 0) return;         // TODO: raise a warning
    if (!en) return;                    // not enabled

    ost << "GUI EVENT: " << MIDISequencerGUIEvent::group_names[e.GetGroup()] << " ";

    switch(e.GetGroup()) {
        case MIDISequencerGUIEvent::GROUP_ALL:
            ost << "GENERAL RESET";
            break;
        case MIDISequencerGUIEvent::GROUP_TRANSPORT:
            switch (e.GetItem()) {
                case MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT:
                    ost << "MEAS " << sequencer->GetCurrentMeasure() + 1 << " " <<
                           "BEAT " << sequencer->GetCurrentBeat() + 1;
                    break;
                case MIDISequencerGUIEvent::GROUP_TRANSPORT_ENDOFSONG:
                    ost << "ENDOFSONG";
                    break;
                case MIDISequencerGUIEvent::GROUP_TRANSPORT_USER:
                    ost << "USER DEFINED";
            }
            break;
        case MIDISequencerGUIEvent::GROUP_CONDUCTOR:
            switch (e.GetItem()) {
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_TEMPO:
                    ost << "TEMPO: " << sequencer->GetState()->tempobpm ;
                    break;
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_TIMESIG:
                    ost << "TIMESIG: " << sequencer->GetState()->timesig_numerator << "/" <<
                           sequencer->GetState()->timesig_denominator;
                    break;
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_KEYSIG:
                    ost << "KEYSIG: " << KeyName(sequencer->GetState()->keysig_sharpflat,
                                                 sequencer->GetState()->keysig_mode);
                    break;
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_MARKER:
                    ost << "MARKER: " << sequencer->GetState()->marker_text;
            }
            break;
        case MIDISequencerGUIEvent::GROUP_TRACK: {
            char s[10];
            int track_num = e.GetSubGroup();
            sprintf (s, "TRACK %3d ", track_num);
            switch (e.GetItem()) {
                case MIDISequencerGUIEvent::GROUP_TRACK_NAME:
                    ost << s << "NAME: " << sequencer->GetTrackState(track_num)->track_name;
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_PROGRAM:
                    ost << s << "PROGRAM: " << sequencer->GetTrackState(track_num)->program;
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_NOTE:
                    ost << s << "NOTE: ";  // TODO: manage this
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_VOLUME:
                    ost << s << "VOLUME: " << sequencer->GetTrackState(track_num)->control_values[C_MAIN_VOLUME];
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_PAN:
                    ost << s << "PAN: " << sequencer->GetTrackState(track_num)->control_values[C_PAN];
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_CHR:
                    ost << s << "CHORUS: " << sequencer->GetTrackState(track_num)->control_values[C_CHORUS_DEPTH];
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_REV:
                    ost << s << "REVERB: " << sequencer->GetTrackState(track_num)->control_values[C_EFFECT_DEPTH];
                break;
            }
        }
        ost << std::endl;
    }
}


/* VERSION IN JDKSMIDI TODO: revise
void MIDISequencerGUIEventNotifierText::Notify (
    const MIDISequencer *seq,
    MIDISequencerGUIEvent e
)
{
    if ( en )
    {
        if ( e.GetEventGroup() == MIDISequencerGUIEvent::GROUP_ALL)
        {
            fprintf ( f, "GUI RESET\n");
        }
        else
        if ( e.GetEventGroup() == MIDISequencerGUIEvent::GROUP_TRANSPORT )
        {
            if (
                e.GetEventItem() == MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT
            )
            {
                fprintf ( f, "MEAS %3d BEAT %3d\n",
                          seq->GetCurrentMeasure() + 1,
                          seq->GetCurrentBeat() + 1
                        );
            }
        }

        else
        if ( e.GetEventGroup() == MIDISequencerGUIEvent::GROUP_CONDUCTOR )
        {
            if (
                e.GetEventItem() == MIDISequencerGUIEvent::GROUP_CONDUCTOR_TIMESIG
            )
            {
                fprintf ( f, "TIMESIG: %d/%d\n",
						  seq->GetState ()->timesig_numerator,    	/* NC
                          seq->GetState ()->timesig_denominator	/* NC
                        );
            }

            if (
                e.GetEventItem() == MIDISequencerGUIEvent::GROUP_CONDUCTOR_TEMPO
            )
            {
                fprintf ( f, "TEMPO: %3.2f\n",
                          seq->GetState ()->tempobpm                        /* NC
						 );
            }
            if (                                /* NC new
                e.GetEventItem() == MIDISequencerGUIEvent::GROUP_CONDUCTOR_KEYSIG
            )
            {
                fprintf ( f, "KEYSIG: \n" );   /* NC : TODO: fix this
            }
            if (                                /* NC new
                e.GetEventItem() == MIDISequencerGUIEvent::GROUP_CONDUCTOR_MARKER
            )
            {
                fprintf ( f, "MARKER: %s\n",
                          seq->GetState()->marker_name
                        );
            }
        }
        else
        if ( e.GetEventGroup() == MIDISequencerGUIEvent::GROUP_TRACK )  /* NC: NEW
        {
            if (
                e.GetEventItem() == MIDISequencerGUIEvent::GROUP_TRACK_NAME
            )
            {
                fprintf ( f, "TRACK %2d NAME: %s\n",
                          e.GetEventSubGroup(),
                          seq->GetTrackState( e.GetEventSubGroup() )->track_name
                        );
            }
            if (
                e.GetEventItem() == MIDISequencerGUIEvent::GROUP_TRACK_PG
            )
            {
                fprintf ( f, "TRACK %2d PROGRAM: %d\n",
                          e.GetEventSubGroup(),
                          seq->GetTrackState( e.GetEventSubGroup() )->pg
						 );
            }
            if (                                /* NC new
                e.GetEventItem() == MIDISequencerGUIEvent::GROUP_TRACK_VOLUME
            )
            {
                fprintf ( f, "TRACK %2d VOLUME: %d\n",
                          e.GetEventSubGroup(),
                          seq->GetTrackState( e.GetEventSubGroup() )->volume
						 );
            }
            // GROUP_TRACK_NOTE ignored!
        }
        else
        {
           fprintf ( f, "GUI EVENT: G=%d, SG=%d, ITEM=%d\n",
                  e.GetEventGroup(),
                  e.GetEventSubGroup(),
                  e.GetEventItem()
                );
        }
    }
}
*/
/////////////////////////////////////////////////////////////////////////////

/* **** SEE THE HEADER
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
*/
//////////////////////////////////////////////////////////////////////////////


#ifdef _WIN32

MIDISequencerGUINotifierWin32::MIDISequencerGUINotifierWin32 (
    HWND w, DWORD msg, WPARAM wparam_value_ ) :
    dest_window ( w ), window_msg ( msg ), wparam_value ( wparam_value_ ) {}


// NEW BY NC: auto sets window_msg and wparam_value
MIDISequencerGUINotifierWin32::MIDISequencerGUINotifierWin32 ( HWND w ) :
    dest_window ( w ), window_msg ( GetSafeSystemMsgId() ), wparam_value ( 0 ) {}


void MIDISequencerGUINotifierWin32::Notify (MIDISequencerGUIEvent e) {
    if ( en )
        PostMessage ( dest_window, window_msg, wparam_value, ( unsigned long ) e );
}


#endif // _WIN32
