/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2020  Nicola Cassetta
 *   https://github.com/ncassetta/NiCMidi
 *
 *   This file is part of NiCMidi.
 *
 *   NiCMidi is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   NiCMidi is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with NiCMidi.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "../include/notifier.h"
#include "../include/sequencer.h"


const char MIDISequencerGUIEvent::group_names[][10] =
        { "All      ", "Conductor", "Transport", "Track    ", "User    " };
const char MIDISequencerGUIEvent::conductor_items_names[][10] =
        { "Tempo    ", "Timesig  ", "Keysig   ", "Marker   ", "User    " };
const char MIDISequencerGUIEvent::transport_items_names[][10] =
        { "Start    ", "Stop     ", "Measure  ", "Beat     ", "User    " };
const char MIDISequencerGUIEvent::track_items_names[][10] =
        { "Name     ", "Program  ", "Note     ", "Volume   ", "Pan     ", "Chorus  ", "Reverb  ", "User    " };
const char MIDISequencerGUIEvent::user_items_names[][10] =
        { "User     " };


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



#ifdef _WIN32

MIDISequencerGUINotifierWin32::MIDISequencerGUINotifierWin32 (
    HWND w, DWORD msg, WPARAM param_value ) :
    dest_window ( w ), window_msg ( msg ), wparam_value ( param_value ) {}


// NEW BY NC: auto sets window_msg and wparam_value
MIDISequencerGUINotifierWin32::MIDISequencerGUINotifierWin32 ( HWND w ) :
    dest_window ( w ), window_msg ( GetSafeSystemMsgId() ), wparam_value ( 0 ) {}


void MIDISequencerGUINotifierWin32::Notify (const MIDISequencerGUIEvent &ev) {
    if ( en )
        PostMessage (dest_window, window_msg, wparam_value, (unsigned long) ev);
}


#endif // _WIN32
