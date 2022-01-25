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


#include "../include/thru.h"
#include "../include/manager.h"
#include "esp_log.h"

static const char *TAG = "MIDIThru";  

MIDIThru::MIDIThru() : MIDITickComponent(PR_PRE_SEQ, StaticTickProc), in_port(0), out_port(0), in_channel(-1),
                                         out_channel(-1), processor(0)
{
    std::cout << "DEBUG MIDIThru constructor" << std::endl;
    //check if in and out ports exist
    if (!MIDIManager::IsValidInPortNumber(0) || !MIDIManager::IsValidOutPortNumber(0))
        throw RtMidiError("MIDIThru needs almost a MIDI in and out port in the system\n", RtMidiError::INVALID_DEVICE);
}


MIDIThru::~MIDIThru() {
    //std::cout << "MIDIThru destructor" << std::endl;
    Stop();
}



void MIDIThru::Reset() {
    Stop();
    SilentOut();
    in_port = 0;
    out_port = 0;
    processor = 0;
    in_channel = -1;
    out_channel = -1;
}


bool MIDIThru::SetInPort(unsigned int port) {
    if (!MIDIManager::IsValidInPortNumber(port))
        return false;                               // avoids out of range errors
    if (port == in_port)
        return true;                                // trying to assign same ports: nothing to do

    if (IsPlaying()) {
        proc_lock.lock();
        MIDIManager::GetInDriver(in_port)->ClosePort();
        SilentOut();
        MIDIManager::GetInDriver(port)->OpenPort();
        in_port = port;
        proc_lock.unlock();
    }
    else
        in_port = port;
    return true;
}


bool MIDIThru::SetOutPort(unsigned int port) {
    if (!MIDIManager::IsValidOutPortNumber(port))
        return false;                               // avoids out of range errors
    if (port == out_port)
        return true;                                // trying to assign same ports: nothing to do

    if (IsPlaying()) {
        proc_lock.lock();
        SilentOut();
        MIDIManager::GetOutDriver(out_port)->ClosePort();
        MIDIManager::GetOutDriver(port)->OpenPort();
        out_port = port;
        proc_lock.unlock();
    }
    else
        out_port = port;
    return true;
}


void MIDIThru::SetProcessor(MIDIProcessor* proc) {
    if (IsPlaying()) {
        proc_lock.lock();
        SilentOut();
        processor = proc;
        proc_lock.unlock();
    }
    else
        processor = proc;
}


bool MIDIThru::SetInChannel(char chan) {
    if (chan < -1 || chan > 15)                     // avoids out of range errors
        return false;
    if (IsPlaying()) {
        proc_lock.lock();
        SilentOut();
        in_channel = chan;
        proc_lock.unlock();
    }
    else
        in_channel = chan;
    return true;
}


bool MIDIThru::SetOutChannel(char chan) {
    if (chan < -1 || chan > 15)                     // avoids out of range errors
        return false;

    if (IsPlaying()) {
        proc_lock.lock();
        SilentOut();
        out_channel = chan;
        proc_lock.unlock();
    }
    else
        out_channel = chan;
    return true;
}


void MIDIThru::Start() {
    if (!IsPlaying()) {
        MIDIManager::GetInDriver(in_port)->OpenPort(); //FCKX
        MIDIManager::GetOutDriver(out_port)->OpenPort();
        MIDITickComponent::Start();
    }
}


void MIDIThru::Stop() {
    if (IsPlaying()) {
        MIDITickComponent::Stop();
        MIDIManager::GetInDriver(in_port)->ClosePort();
        SilentOut();
        MIDIManager::GetOutDriver(out_port)->ClosePort();
    }
}


void MIDIThru::SilentOut() {
    if (out_channel != -1)
        MIDIManager::GetOutDriver(out_port)->AllNotesOff(out_channel);
    else
        MIDIManager::GetOutDriver(out_port)->AllNotesOff();
}


void MIDIThru::StaticTickProc(tMsecs sys_time, void* pt) {
    MIDIThru* thru_pt = static_cast<MIDIThru *>(pt);
    thru_pt->TickProc(sys_time);
}


void MIDIThru::TickProc(tMsecs sys_time_)
{
   static const char *TAG = "MIDIThru::TickProc";
   proc_lock.lock();
    /*
    static unsigned int times = 0;
    times++;
    if (!(times % 100)) 
        std::cout << "FCKX MIDIThru::TickProc() called " << times << " times\n";
    std::cout << "FCKX MIDIManager::GetNumMIDIIns(): " << MIDIManager::GetNumMIDIIns() << "\n";
    */
    MIDIRawMessage rmsg;
    MIDITimedMessage msg;
    //std::cout << "in_port" << in_port << "\n";
    MIDIInDriver* in_driver = MIDIManager::GetInDriver(in_port);
    MIDIOutDriver* out_driver = MIDIManager::GetOutDriver(out_port);
    in_driver->LockQueue();
    //std::cout << "FCKX MIDIThru::TickProc() inspects input queue: size ="<<in_driver->GetQueueSize()<<"\n";

    for (unsigned int i = 0; i < in_driver->GetQueueSize(); i++) {
        std::cout << "Message found\n";
        //get message from the queue WITHOUT deleting it. This is done by Manager at the end of the MidiTicks queue
        in_driver->ReadMessage(rmsg, i);
        msg = rmsg.msg;
        ESP_LOGE(TAG,"msg.GetLength() %d", msg.GetLength() );       
  /* //can not access bytes per bytenr as GetByten is hard coded  //useful for longer RPN/NRPN messages
     for (unsigned int ii = 0; ii < msg.GetLength(); ii++) {
      
            
        }
  */
        ESP_LOGE(TAG,"msg.Status() %u 0x%X", msg.GetStatus(), msg.GetStatus());
        ESP_LOGE(TAG,"msg.GetByte1() %u 0x%X", msg.GetByte1(), msg.GetByte1());
        ESP_LOGE(TAG,"msg.GetByte2() %u 0x%X", msg.GetByte2(), msg.GetByte2());  
        ESP_LOGE(TAG,"msg.GetByte3() %u 0x%X", msg.GetByte3(), msg.GetByte3()); 
        /*
        std::cout << "msg.GetLength()"<< msg.GetLength() <<"\n"; 
        std::cout << "msg.GetStatus()"<< msg.GetStatus() <<"\n"; 
        std::cout << "msg.GetByte1()"<< msg.GetByte1() <<"\n"; 
        std::cout << "msg.GetByte2()"<< msg.GetByte2() <<"\n"; 
        std::cout << "msg.GetByte3()"<< msg.GetByte3() <<"\n";          
        */
        //ESP_LOGE(TAG,"msg.IsChannelMsg() %d", msg.IsChannelMsg()); 

        ESP_LOGE(TAG,"msg.IsChannelMsg() %d", msg.IsChannelMsg()); 
   
        if (msg.IsChannelMsg()) {
                ESP_LOGE(TAG,"in_channel %d out_channel %d msg.GetChannel() %d ",in_channel, out_channel, msg.GetChannel());     
                if (in_channel == msg.GetChannel() || in_channel == -1) { //FCKX!!
                //if (in_channel == msg.GetChannel() || in_channel == -1) {
                    if (out_channel != -1) { //FCKX!!
                    //if (out_channel != -1) {
                        msg.SetChannel(out_channel);
                        //std::cout << "MIDIThru::TickProc out_channel != -1\n";
                        }
                    if (processor) {
                        processor->Process(&msg); 
                        //std::cout << "MIDIThru::TickProc processor\n";                   
                        }
                    //std::cout << "MIDIThru::TickProc BEFORE out_driver->OutputMessage(msg) \n";      
                    out_driver->OutputMessage(msg);
                    //std::cout << "MIDIThru::TickProc AFTER out_driver->OutputMessage(msg) \n";                  
                }
            } else {ESP_LOGE(TAG,"NOT A CHANNEL MESSAGE"); }
        }
    in_driver->UnlockQueue();
    proc_lock.unlock();
}
