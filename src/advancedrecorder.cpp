/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
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


#include "../include/advancedrecorder.h"
#include "../include/manager.h"


AdvancedRecorder::AdvancedRecorder(AdvancedSequencer* seq) : sequencer(seq), metro_delay(0), status(ST_STOP) {
    //metronome.set
    MIDIManager::AddMIDITick(this);
    MIDIManager::AddMIDITick(&metronome);
}


AdvancedRecorder::~AdvancedRecorder() {}


void AdvancedRecorder::Reset() {
    MIDIRecorder::Reset();
}


//void AdvancedRecorder::GoToZero()                      { GoToTime(0); }

bool AdvancedRecorder::GoToTime(MIDIClockTime time_clk) {
    if (sequencer->GoToTime(time_clk)) {
        if(IsPlaying() && rec_on.load()) {
            proc_lock.lock();
            start_time = time_clk;
            proc_lock.unlock();
        }
        else
            start_time = time_clk;
    }
    else
        start_time = time_clk;
    return true;
}


bool AdvancedRecorder::GoToTimeMs(float time_ms) {
}

bool AdvancedRecorder::GoToMeasure(int measure, int beat) {
    if (sequencer->GoToMeasure(measure, beat))
        start_time = sequencer->GetCurrentMIDIClockTime();
    return true;
}


void AdvancedRecorder::Start() {
    if(!IsPlaying()) {
        std::cout << "\t\tEntered in AdvancedRecorder::Start() ..." << std::endl;
        if(sequencer->IsPlaying()) {
            start_time = sequencer->GetCurrentMIDIClockTime();
            metro_delay = 0.0;
            status = ST_WITH_SEQ;
        }
        else {
            sequencer->GoToTime(start_time);
            metro_delay = 60000.0 / tempobpm * sequencer->GetTimeSigNumerator();
            status = ST_PRE_COUNT;
        }
        //tempobpm = sequencer->GetTempoWithScale();
        //metronome.SetTempo(tempobpm);
        //metronome.SetTimeSigDenominator(sequencer->GetTimeSigDenominator());
        //metronome.SetTimeSigNumerator(sequencer->GetTimeSigNumerator());
        //metronome.Start();
        std::cout << "\t\t ... Exiting from AdvancedRecorder::Start()" << std::endl;
        MIDIRecorder::Start();
        if (status == ST_PRE_COUNT)
            rec_on.store(false);
    }
}


void AdvancedRecorder::Stop() {
    if (IsPlaying()) {
        std::cout << "\t\tEntered in AdvancedRecorder::Stop() ..." << std::endl;
        MIDIRecorder::Stop();
        metronome.Stop();
        if (sequencer->IsPlaying())
            sequencer->Stop();
        status = ST_STOP;
        std::cout << "\t\t ... Exiting from MIDIRecorder::Stop()" << std::endl;
    }
}


void AdvancedRecorder::AdvancedRecorder::StaticTickProc(tMsecs sys_time, void* pt) {
    AdvancedRecorder* rec_pt = static_cast<AdvancedRecorder*>(pt);
    rec_pt->TickProc(sys_time);

}


void AdvancedRecorder::TickProc(tMsecs sys_time) {
    if (!sequencer->IsPlaying()) {
        if (status == ST_PRE_COUNT) {
            //std::cout << "Offset: " << sys_time - sys_time_offset << std::endl;
            if (sys_time - sys_time_offset >= metro_delay) {
                std::cout << "AdvancedRecorder started the sequencer\n";
                std::cout << "from time " << sequencer->GetCurrentMIDIClockTime() << std::endl;
                status = ST_WITH_SEQ;
                sequencer->Start();
                //sys_time_offset = MIDITimer::GetSysTimeMs();
                //rec_on.store(true);
            }
        }
        else
            status = ST_ALONE;
    }
    if (status == ST_WITH_SEQ || status == ST_ALONE)
        //MIDIRecorder::TickProc(sys_time);
        ;
}
