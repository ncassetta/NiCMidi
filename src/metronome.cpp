/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2021, 2022  Nicola Cassetta
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
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with NiCMidi. If not, see <http://www.gnu.org/licenses/>.
 */


#include "../include/metronome.h"
#include "../include/manager.h"



////////////////////////////////////////////////////////////////////////////
//                            class Metronome                             //
////////////////////////////////////////////////////////////////////////////


Metronome::Metronome (MIDISequencerGUINotifier* n) :
    MIDITickComponent(PR_POST_SEQ, StaticTickProc),
    notifier(n)
{
    // verify if almost one out port is present in the system
    if (!MIDIManager::IsValidOutPortNumber(0))
        throw RtMidiError("Metronome needs almost a MIDI out port in the system\n", RtMidiError::INVALID_DEVICE);
    Reset();
}



void Metronome::Reset() {
    Stop();
    out_port = new_out_port = 0;
    chan = new_chan = DEFAULT_CHAN;
    meas_note = new_meas_note = DEFAULT_MEAS_NOTE;
    beat_note = new_beat_note = DEFAULT_BEAT_NOTE;
    subd_note = new_subd_note = DEFAULT_SUBD_NOTE;
    subd_type = new_subd_type = 0;
    timesig_numerator = new_timesig_numerator = 0;
    //timesig_denominator = new_timesig_denominator = MIDI_DEFAULT_TIMESIG_DENOMINATOR;
    tempobpm = new_tempobpm = (float)MIDI_DEFAULT_TEMPO;
    tempo_scale = new_tempo_scale = 100;

    cur_clock = 0;
    cur_time_ms = 0.0;
    cur_beat = 0;
    cur_measure = 0;
    beat_length = QUARTER_LENGTH;
    msecs_per_beat = 60000.0 / tempobpm;
    onoff_time = std::max((float)MIN_NOTE_LEN, msecs_per_beat / 4);
}


float Metronome::GetCurrentTimeMs() const {
    return IsPlaying() ?
        MIDITimer::GetSysTimeMs() - sys_time_offset + dev_time_offset :
        cur_time_ms;
}


bool Metronome::SetTempo(float t) {
    if (t < 1.0 || t > 300.0)
        return false;
    new_tempobpm = t;
    if (!IsPlaying())
        UpdateValues();
    return true;
}


bool Metronome::SetTempoScale(unsigned int scale) {
    if (scale == 0)
        return false;
    new_tempo_scale = scale;
    if (!IsPlaying())
        UpdateValues();
    return true;
}


bool Metronome::SetOutPort(unsigned int port) {
    if (!MIDIManager::IsValidOutPortNumber(port))
        return false;                                       // avoids out of range errors
    if (port == out_port)                                   // trying to assign same ports: nothing to do
        return true;
    if (IsPlaying()) {
        // this has immediate effect, so we must lock the mutex
        std::lock_guard<std::recursive_mutex> lock(proc_lock);
        MIDIManager::GetOutDriver(out_port)->AllNotesOff(chan);
        MIDIManager::GetOutDriver(out_port)->ClosePort();
        out_port = port;
        MIDIManager::GetOutDriver(out_port)->OpenPort();
    }
    else
        out_port = port;
    return true;
}


bool Metronome::SetOutChannel(unsigned int chan) {
    if (chan > 15)                                          // avoids out of range errors
        return false;
    new_chan = chan;
    if (!IsPlaying())
        UpdateValues();
    return true;
}


void Metronome::SetMeasNote(unsigned char note) {
    new_meas_note = note;
    if (!IsPlaying())
        UpdateValues();
}


void Metronome::SetBeatNote(unsigned char note) {
    new_beat_note = note;
    if (!IsPlaying())
        UpdateValues();
}


void Metronome::SetSubdNote(unsigned char note) {
    new_subd_note = note;
    if (!IsPlaying())
        UpdateValues();
}


bool Metronome::SetSubdType(unsigned char type) {
    if (type == 1 || type > 6)
        return false;
    new_subd_type = type;
    if (!IsPlaying())
        UpdateValues();
    return true;
    /*
    beat_length = subd_on ? QUARTER_LENGTH / subd_type : QUARTER_LENGTH;
    msecs_per_beat = 60000.0 / (tempobpm * (subd_on ? subd_type : 1));
    onoff_time = std::max((float)MIN_NOTE_LEN, msecs_per_beat / 4);
    if (was_playing)
        proc_lock.unlock();
    */
}



void Metronome::SetTimeSigNumerator(unsigned char n) {
    new_timesig_numerator = n;
    if (!IsPlaying())
        UpdateValues();
}


/*
                // calculate new milliseconds per clock: this comes from
                //  -true_bpm = old_tempo * tempo_scale / 100
                //  -clocks_per_sec = true_bpm * clks_per_beat / 60
                //  -clocks_per_ms = clocks_per_sec / 1000
                //  -ms_per_clock = 1 / clocks_per_ms
                ms_per_clock = 6000000.0 / (msg->GetTempo() *
                                (double)tempo_scale * state.multitrack->GetClksPerBeat());

                // update variables for next tempo change (or now_t == t)
                base_t = now_t;

*/

/*
void Metronome::SetTimeSigDenominator(unsigned char d) {
    new_timesig_denominator = d;
    if (!IsPlaying())
        UpdateValues();
}
*/


// Inherited from MIDITICK

void Metronome::Start() {
    if (!IsPlaying()) {
        //std::cout << "\t\tEntered in Metronome::Start() ..." << std::endl;
        MIDIManager::GetOutDriver(out_port)->OpenPort();
        if (notifier)
            notifier->Notify (MIDISequencerGUIEvent(MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                                    0,
                                                    MIDISequencerGUIEvent::GROUP_TRANSPORT_START));
        cur_clock = 0;
        cur_time_ms = 0.0;
        next_time_on = 0.0;
        next_time_off = onoff_time;
        cur_beat = 0;
        cur_measure = 0;
        MIDITickComponent::Start();

        //std::cout << "\t\t ... Exiting from Metronome::Start()" << std::endl;
    }
}


void Metronome::Stop() {
    if (IsPlaying()) {
        //std::cout << "\t\tEntered in Metronome::Stop() ..." << std::endl;
        MIDITickComponent::Stop();
        MIDIManager::GetOutDriver(out_port)->AllNotesOff(chan);
        MIDIManager::GetOutDriver(out_port)->ClosePort();

        if (notifier)
            notifier->Notify (MIDISequencerGUIEvent(MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                                    0,
                                                    MIDISequencerGUIEvent::GROUP_TRANSPORT_STOP));
        //std::cout << "\t\t ... Exiting from Metronome::Stop()" << std::endl;
    }
}



void Metronome::UpdateValues() {
    /* Not used
    if (new_port != port) {
        if (IsPlaying())
            MIDIManager::GetOutDriver(port)->AllNotesOff(chan);
        port = new_port;
     }
     */
    if (new_chan != chan) {
        if (IsPlaying())
            MIDIManager::GetOutDriver(out_port)->AllNotesOff(chan);
        chan = new_chan;
    }
    meas_note = new_meas_note;
    beat_note = new_beat_note;
    subd_note = new_subd_note;

    if (new_subd_type != subd_type) {
        subd_type = new_subd_type;
        beat_length = (subd_type == 0 ? QUARTER_LENGTH : QUARTER_LENGTH / subd_type);
        msecs_per_beat = 6000000.0 / (tempo_scale * tempobpm * (subd_type == 0 ? 1 : subd_type));
        onoff_time = std::max((float)MIN_NOTE_LEN, msecs_per_beat / 4);
    }

    if (new_timesig_numerator != timesig_numerator) {
        timesig_numerator = new_timesig_numerator;
        if (timesig_numerator == 0)
            cur_measure = 0;
        cur_beat = 0;
    }
    if (new_tempobpm != tempobpm) {
        tempobpm = new_tempobpm;
        msecs_per_beat = 6000000.0 / (tempo_scale * tempobpm * (subd_type == 0 ? 1 : subd_type));
        onoff_time = std::max((float)MIN_NOTE_LEN, msecs_per_beat / 4);
    }

    if (new_tempo_scale != tempo_scale) {
        tempo_scale = new_tempo_scale;
        msecs_per_beat = 6000000.0 / (tempo_scale * tempobpm * (subd_type == 0 ? 1 : subd_type));
        onoff_time = std::max((float)MIN_NOTE_LEN, msecs_per_beat / 4);
    }
}



void Metronome::StaticTickProc(tMsecs sys_time, void* pt) {
    Metronome* met_pt = static_cast<Metronome *>(pt);
    met_pt->TickProc(sys_time);
}



void Metronome::TickProc(tMsecs sys_time) {
    static unsigned char last_note = 0;
    unsigned char note, vel;
    MIDISequencerGUIEvent ev;
    MIDIMessage msg_beat;
    //static unsigned int times;
    //times++;
    //if (!(times % 100))
    //    std::cout << "Metronome::TickProc() " << times << " times" << std::endl;


    std::lock_guard<std::recursive_mutex> lock(proc_lock);
    tMsecs cur_time = sys_time - sys_time_offset + dev_time_offset;

    if (cur_time >= static_cast<tMsecs>(next_time_on)) {    // we must send a note on
        if (cur_clock % QUARTER_LENGTH) {           // this is a subdivision beat
            note = subd_note;                       // send a subd note message
            vel = SUBD_NOTE_VEL;
            //std::cout << "SUBD on MIDI clock " << cur_clock << std::endl;
        }
        else {                                      // this is a quarter beat

            UpdateValues();

            if (timesig_numerator > 0) {            // measures count is on
                if (cur_beat == 0) {                // 1st beat of a measure
                    note = meas_note;               // if meas beat is on we send a meas note message ...
                    vel = MEAS_NOTE_VEL;
                    //std::cout << "MEAS on MIDI clock " << cur_clock << std::endl;
                    ev = MIDISequencerGUIEvent(MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                               0,
                                               MIDISequencerGUIEvent::GROUP_TRANSPORT_MEASURE);
                }
                else {
                    note = beat_note;               // ... otherwise an ordinary beat message
                    vel = BEAT_NOTE_VEL;
                    //std::cout << "BEAT on MIDI clock " << cur_clock << std::endl;
                    ev = MIDISequencerGUIEvent(MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                               0,
                                               MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT);
                }
                cur_beat++;
                if (cur_beat == timesig_numerator) {
                    cur_measure++;
                    cur_beat = 0;
                }
            }
            else {                                  // ordinary beat, no measures count
                note = beat_note;
                vel = BEAT_NOTE_VEL;
                //std::cout << "BEAT on MIDI clock " << cur_clock << std::endl;
                ev = MIDISequencerGUIEvent(MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                           0,
                                           MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT);
            }
        }

        // tell the driver to send the click note on
        msg_beat.SetNoteOn(chan, note, vel);
        MIDIManager::GetOutDriver(out_port)->OutputMessage(msg_beat);
        if (notifier)
            notifier->Notify(ev);
        //std::cout << "Note on ... ";

        // now adjust next beat time
        cur_clock += beat_length;
        next_time_on += msecs_per_beat;
        last_note = note;
    }

    else if (cur_time >= static_cast<tMsecs>(next_time_off)) {  // we must send the note off

        // tell the driver the send the beat note off
        msg_beat.SetNoteOff(chan, last_note, 0);
        MIDIManager::GetOutDriver(out_port)->OutputMessage(msg_beat);
        next_time_off += msecs_per_beat;
        //std::cout << "Note off" << std::endl;
    }
}



