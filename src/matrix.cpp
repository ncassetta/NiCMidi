/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2021  Nicola Cassetta
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


#include "../include/matrix.h"
#include "../include/dump_tracks.h"


MIDIMatrix::MIDIMatrix() {
        for(int chan = 0; chan < 16; chan++) {
        channel_count[chan] = 0;
        min_note[chan] = 127;
        max_note[chan] = 0;
        hold_pedal[chan] = false;
        for(unsigned char note = 0; note < 128; note++)
            note_on_count[chan][note] = 0;
    }
    total_count = 0;
}


bool MIDIMatrix::Process(MIDITimedMessage* msg) {
    bool ret = false;

    if(msg->IsChannelMsg()) {
        int chan = msg->GetChannel();

        if(msg->IsAllNotesOff()) {
            ClearChannel(chan);
            ret = true;
        }
        else if(msg->IsNoteOn()) {
            unsigned char note = msg->GetNote();
            IncNoteCount(chan, note);
            if (note < min_note[chan])
                min_note[chan] = note;
            if (note > max_note[chan])
                max_note[chan] = note;
            ret = true;
        }
        else if(msg->IsNoteOff()) {
            unsigned char note = msg->GetNote();
            DecNoteCount(chan, note);
            if (channel_count[chan] == 0) {
                min_note[chan] = 127;
                max_note[chan] = 0;
            }
            else {
                if (note == min_note[chan] && note_on_count[chan][note] == 0) {
                    for (unsigned char i = note + 1; i <= max_note[chan]; i++) {
                        if (note_on_count[chan][i] > 0) {
                            min_note[chan] = i;
                            break;
                        }
                    }
                }
                if (note == max_note[chan] && note_on_count[chan][note] == 0) {
                    for (unsigned char i = note - 1; i >= min_note[chan]; i--) {
                        if (note_on_count[chan][i] > 0) {
                            max_note[chan] = i;
                            break;
                        }
                    }
                }
            }
            ret = true;
        }
        else if(msg->IsPedalOn()) {
            hold_pedal[chan] = true;
            ret = true;
        }
        else if(msg->IsPedalOff()) {
            hold_pedal[chan] = false;
            ret = true;
        }
        else
            OtherMessage(msg);
    }
    //CheckMIDIMatrix(*this);
    return ret;
}


void MIDIMatrix::Reset() {
    for(int chan = 0; chan < 16; ++chan)
        ClearChannel(chan);
    total_count = 0;
}


void MIDIMatrix::DecNoteCount(int chan, int note) {
    if(note_on_count[chan][note] > 0) {
      --note_on_count[chan][note];
      --channel_count[chan];
      --total_count;
    }
}


void MIDIMatrix::IncNoteCount(int chan, int note) {
    ++note_on_count[chan][note];
    ++channel_count[chan];
    ++total_count;
}


void MIDIMatrix::ClearChannel(int chan) {
    for(int note = 0; note < 128; ++note)
        note_on_count[chan][note] = 0;
    min_note[chan] = 127;
    max_note[chan] = 0;
    total_count -= channel_count[chan];
    channel_count[chan] = 0;
    hold_pedal[chan] = 0;
}
