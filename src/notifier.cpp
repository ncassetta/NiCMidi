#include "../include/notifier.h"
#include "../include/sequencer.h"


const char MIDISequencerGUIEvent::group_names[][10] =
        { "All      ", "Conductor", "Transport", "Track    ", "User     " };
const char MIDISequencerGUIEvent::conductor_items_names[][10] =
        { "All      ", "Tempo    ", "Timesig  ", "Keysig   ", "Marker   ", "User    " };
const char MIDISequencerGUIEvent::transport_items_names[][10] =
        { "All      ", "Mode     ", "Measure  ", "Beat     ", "EndOfSong", "User    " };
const char MIDISequencerGUIEvent::track_items_names[][10] =
        { "All      ", "Name     ", "Patch    ", "Note     ", "Volume   ", "User    " };


void MIDISequencerGUINotifierText::Notify(const MIDISequencerGUIEvent &ev) {
// reworked with an unique call to ost <<, so that there's no trouble with
// cout call in other threads. (Crashed???)
    if (sequencer == 0) return;
    if (!en) return;                    // not enabled

    char s[200];
    int track_num = ev.GetSubGroup();   // used only for track events
    int wr = sprintf(s, "GUI EVENT: %s ", MIDISequencerGUIEvent::group_names[ev.GetGroup()]);

    switch(ev.GetGroup()) {
        case MIDISequencerGUIEvent::GROUP_ALL:
            sprintf(s + wr, "GENERAL RESET");
            break;
        case MIDISequencerGUIEvent::GROUP_CONDUCTOR:
            switch (ev.GetItem()) {
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_TEMPO:
                    sprintf(s + wr, "TEMPO:    %2f bpm", sequencer->GetState()->tempobpm);
                    break;
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_TIMESIG:
                    sprintf(s + wr, "TIMESIG:  %d/%d", sequencer->GetState()->timesig_numerator,
                           sequencer->GetState()->timesig_denominator);
                    break;
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_KEYSIG:
                    sprintf(s + wr, "KEYSIG:   %s", KeyName(sequencer->GetState()->keysig_sharpflat,
                                                       sequencer->GetState()->keysig_mode));
                    break;
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_MARKER:
                    sprintf(s + wr, "MARKER:   %s", sequencer->GetState()->marker_text.c_str());
                    break;
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_USER:
                    sprintf(s + wr, "USER EV Item %d", ev.GetItem());
                    break;
            }
            break;
        case MIDISequencerGUIEvent::GROUP_TRANSPORT:
            switch (ev.GetItem()) {
                case MIDISequencerGUIEvent::GROUP_TRANSPORT_START:
                    sprintf(s + wr, "SEQUENCER START");
                    break;
                case MIDISequencerGUIEvent::GROUP_TRANSPORT_STOP:
                    sprintf(s + wr, "SEQUENCER STOP");
                    break;
                case MIDISequencerGUIEvent::GROUP_TRANSPORT_MEASURE:
                    sprintf(s + wr, "MEAS %d", sequencer->GetCurrentMeasure() + 1);
                    break;
                case MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT:
                    sprintf(s + wr, "MEAS %d BEAT %d", sequencer->GetCurrentMeasure() + 1,
                           sequencer->GetCurrentBeat() + 1);
                    break;
                case MIDISequencerGUIEvent::GROUP_TRANSPORT_USER:
                    sprintf(s + wr, "USER EV Item %d", ev.GetItem());
                    break;
            }
            break;
        case MIDISequencerGUIEvent::GROUP_TRACK:
            wr += sprintf (s + wr, "TRACK %3d ", track_num);
            switch (ev.GetItem()) {
                case MIDISequencerGUIEvent::GROUP_TRACK_NAME:
                    sprintf(s + wr, "NAME: %s", sequencer->GetTrackState(track_num)->track_name.c_str());
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_PROGRAM:
                    sprintf(s + wr, "PROGRAM: %d", sequencer->GetTrackState(track_num)->program);
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_NOTE:
                    sprintf(s + wr, "NOTE");
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_VOLUME:
                    sprintf(s + wr, "VOLUME: %d",sequencer->GetTrackState(track_num)->control_values[C_MAIN_VOLUME]);
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_PAN:
                    sprintf(s + wr, "PAN: %d", sequencer->GetTrackState(track_num)->control_values[C_PAN]);
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_CHR:
                    sprintf(s + wr, "CHORUS: %d", sequencer->GetTrackState(track_num)->control_values[C_CHORUS_DEPTH]);
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_REV:
                    sprintf(s + wr, "REVERB: %d", sequencer->GetTrackState(track_num)->control_values[C_EFFECT_DEPTH]);
                    break;
                case MIDISequencerGUIEvent::GROUP_TRACK_USER:
                    sprintf(s + wr, "USER EV Item %d", ev.GetItem());
                    break;
            }
            break;
        case MIDISequencerGUIEvent::GROUP_USER:
            sprintf(s + wr, "Subgroup: %d Item: %d", ev.GetSubGroup(), ev.GetItem());
            break;
    }
    strcat(s, "\n");
    ost << s;
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


void MIDISequencerGUINotifierWin32::Notify (const MIDISequencerGUIEvent &ev) {
    if ( en )
        PostMessage (dest_window, window_msg, wparam_value, (unsigned long) ev);
}


#endif // _WIN32
