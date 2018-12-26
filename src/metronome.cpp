#include "../include/metronome.h"



////////////////////////////////////////////////////////////////////////////
//                            class Metronome                             //
////////////////////////////////////////////////////////////////////////////


Metronome::Metronome (MIDISequencerGUINotifier* n) :
    MIDITickComponent(PR_SEQ, StaticTickProc),
    notifier(n)
{
    Reset();
}



void Metronome::Reset() {
    Stop();
    if (MIDIManager::GetNumMIDIOuts() > 0)
        port = 0;
    else
        port = -1;
    chan = 9;
    meas_note = 60;
    beat_note = 58;
    subd_note = 56;
    meas_on = false;
    subd_on = false;
    subd_type = 2;
    cur_clock = 0;
    cur_time_ms = 0.0;
    cur_beat = 0;
    cur_measure = 0;
    beat_length = QUARTER_LENGTH;
    tempo_scale = 100;
    tempobpm = (float)MIDI_DEFAULT_TEMPO;
    timesig_numerator = MIDI_DEFAULT_TIMESIG_NUMERATOR;
    timesig_denominator = MIDI_DEFAULT_TIMESIG_DENOMINATOR;
    SetTempo(tempobpm);
}


float Metronome::GetCurrentTimeMs() const {
    return IsPlaying() ?
        MIDITimer::GetSysTimeMs() - sys_time_offset + dev_time_offset :
        cur_time_ms;
}


void Metronome::SetTempo(float t) {
    bool was_playing = false;
    if (t > 0.0 && t <= 300.0) {
        if (IsPlaying()) {
            was_playing = true;
            proc_lock.lock();
        }
        tempobpm = t;
        msecs_per_beat = 60000.0 / t;
        onoff_time = std::max((float)MIN_NOTE_LEN, msecs_per_beat / 4);
        if (was_playing)
            proc_lock.unlock();
    }
}


void Metronome::SetTempoScale(unsigned int scale) {
    if (IsPlaying()) {
        proc_lock.lock();
        tempo_scale = scale;
        proc_lock.unlock();
    }
    else
        tempo_scale = scale;
    //cur_time_ms = MIDItoMs(cur_clock);
}


void Metronome::SetOutPort(unsigned int p) {
    p %= MIDIManager::GetNumMIDIOuts();
    if (p == (unsigned)port)
        return;
    if (IsPlaying()) {
        proc_lock.lock();
        MIDIManager::GetOutDriver(port)->AllNotesOff(chan);
        MIDIManager::GetOutDriver(port)->ClosePort();
        port = p;
        MIDIManager::GetOutDriver(port)->OpenPort();
        proc_lock.unlock();
    }
    else
        port = p;
}


void Metronome::SetOutChannel(unsigned char ch) {
    if (IsPlaying()) {
        proc_lock.lock();
        MIDIManager::GetOutDriver(port)->AllNotesOff(chan);
        chan = ch;
        proc_lock.unlock();
    }
    else
        chan = ch;
}


void Metronome::SetMeasNote(unsigned char note) {
    if (IsPlaying()) {
        proc_lock.lock();
        msg_beat.SetNoteOff(chan, meas_note, 0);
        MIDIManager::GetOutDriver(port)->OutputMessage(msg_beat);
        meas_note = note;
        proc_lock.unlock();
    }
    else
        meas_note = note;
}


void Metronome::SetBeatNote(unsigned char note) {
    if (IsPlaying()) {
        proc_lock.lock();
        msg_beat.SetNoteOff(chan, beat_note, 0);
        MIDIManager::GetOutDriver(port)->OutputMessage(msg_beat);
        beat_note = note;
        proc_lock.unlock();
    }
    else
        beat_note = note;
}


void Metronome::SetSubdNote(unsigned char note) {
    if (IsPlaying()) {
        proc_lock.lock();
        msg_beat.SetNoteOff(chan, subd_note, 0);
        MIDIManager::GetOutDriver(port)->OutputMessage(msg_beat);
        subd_note = note;
        proc_lock.unlock();
    }
    else
        subd_note = note;
}


void Metronome::SetMeasEnable(bool on_off) {
    if (IsPlaying() && meas_on != on_off) {
        proc_lock.lock();
        msg_beat.SetNoteOff(chan, meas_note, 0);
        MIDIManager::GetOutDriver(port)->OutputMessage(msg_beat);
        meas_on = on_off;
        proc_lock.unlock();
    }
    if (!IsPlaying())
        meas_on = on_off;
}


void Metronome::SetSubdEnable(bool on_off) {
    bool was_playing = false;
    if (IsPlaying()) {
        was_playing = true;
        proc_lock.lock();
    }
    msg_beat.SetNoteOff(chan, subd_note, 0);
    MIDIManager::GetOutDriver(port)->OutputMessage(msg_beat);
    subd_on = on_off;
    beat_length = subd_on ? QUARTER_LENGTH / subd_type : QUARTER_LENGTH;
    msecs_per_beat = 60000.0 / (tempobpm * (subd_on ? subd_type : 1));
    onoff_time = std::max((float)MIN_NOTE_LEN, msecs_per_beat / 4);
    if (was_playing)
        proc_lock.unlock();
}


void Metronome::SetSubdType(unsigned char type) {
    if (type < 2 || type > 6)
        return;
    bool was_playing = false;
    if (IsPlaying()) {
        was_playing = true;
        proc_lock.lock();
    }
    subd_type = type;
    beat_length = subd_on ? QUARTER_LENGTH / subd_type : QUARTER_LENGTH;
    msecs_per_beat = 60000.0 / (tempobpm * (subd_on ? subd_type : 1));
    onoff_time = std::max((float)MIN_NOTE_LEN, msecs_per_beat / 4);
    if (was_playing)
        proc_lock.unlock();
}


void Metronome::SetTimeSigNumerator(unsigned char n) {
    if (IsPlaying()) {
        proc_lock.lock();
        timesig_numerator = n;
        proc_lock.unlock();
    }
    else
        timesig_numerator = n;
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

// Inherited from MIDITICK

void Metronome::Start() {
    if (!IsPlaying()) {
        std::cout << "\t\tEntered in MIDISequencer::Start() ..." << std::endl;
        MIDIManager::OpenOutPorts();
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

        std::cout << "\t\t ... Exiting from MIDISequencer::Start()" << std::endl;
    }
}


void Metronome::Stop() {
    if (IsPlaying()) {
        std::cout << "\t\tEntered in Metronome::Stop() ..." << std::endl;
        MIDITickComponent::Stop();
        MIDIManager::AllNotesOff();
        MIDIManager::CloseOutPorts();

        if (notifier)
            notifier->Notify (MIDISequencerGUIEvent(MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                                    0,
                                                    MIDISequencerGUIEvent::GROUP_TRANSPORT_STOP));
        std::cout << "\t\t ... Exiting from Metronome::Stop()" << std::endl;
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
    //static unsigned int times;
    //times++;
    //if (!(times % 100))
    //    std::cout << "Metronome::TickProc() " << times << " times" << std::endl;


    //
    tMsecs cur_time = sys_time - sys_time_offset + dev_time_offset;

    if (static_cast<tMsecs>(next_time_on) <= cur_time) {    // we must send a note on
        if (cur_clock % QUARTER_LENGTH) {           // this is a subdivision beat
            note = subd_note;                       // send a subd note message
            vel = SUBD_NOTE_VEL;
            std::cout << "SUBD on MIDI clock " << cur_clock << std::endl;
        }
        else {                                      // this is a quarter beat
            if (cur_beat == 0) {                    // 1st beat of a measure
                if (meas_on) {
                    note = meas_note;               // if meas beat is on we send a meas note message ...
                    vel = MEAS_NOTE_VEL;
                    std::cout << "NOTE on MIDI clock " << cur_clock << std::endl;
                }
                else {
                    note = beat_note;               // ... otherwise an ordinary beat message
                    vel = BEAT_NOTE_VEL;
                    std::cout << "BEAT on MIDI clock " << cur_clock << std::endl;
                }
                ev = MIDISequencerGUIEvent(MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                           0,
                                           MIDISequencerGUIEvent::GROUP_TRANSPORT_MEASURE);
            }
            else {                                  // ordinary beat
                note = beat_note;
                vel = BEAT_NOTE_VEL;
                std::cout << "BEAT on MIDI clock " << cur_clock << std::endl;
                ev = MIDISequencerGUIEvent(MIDISequencerGUIEvent::GROUP_TRANSPORT,
                                           0,
                                           MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT);
            }
            cur_beat++;
            if (cur_beat == timesig_numerator) {
                cur_beat = 0;
                cur_measure++;
            }
        }

        // tell the driver the send the beat note on
        msg_beat.SetNoteOn(chan, note, vel );
        MIDIManager::GetOutDriver(port)->OutputMessage(msg_beat);
        if (notifier)
            notifier->Notify(ev);
        //std::cout << "Note on ... ";

        // now adjust next beat time
        cur_clock += beat_length;
        next_time_on += msecs_per_beat;
        last_note = note;
    }

    else if (static_cast<tMsecs>(next_time_off) <= cur_time) {  // we must send the note off

        // tell the driver the send the beat note off
        msg_beat.SetNoteOff(chan, last_note, 0);
        MIDIManager::GetOutDriver(port)->OutputMessage(msg_beat);
        next_time_off += msecs_per_beat;
        //std::cout << "Note off" << std::endl;
    }
    /*
    // auto stop at end of sequence
    if( !(repeat_play_mode && GetCurrentMeasure() >= repeat_end_meas) &&
        !GetNextEventTimeMs(&next_event_time)) {
        // no events left

        std::thread(StaticStopProc, this).detach();
        std::cout << "Stopping the sequencer: StaticStopProc called" << std::endl;
    }
    */
}



