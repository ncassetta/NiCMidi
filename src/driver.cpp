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

#define MIDI_IN_USED  //FCKX



#include "../include/driver.h"
#include "../include/timer.h"
#include "esp_log.h"
//FCKX
#include "nimBLEdriver.h" 
#include "MQTTdriver.h"

//static const char *TAG = "NICMIDI DRIVER";

/////////////////////////////////////////////////
//         class MIDIRawMessageQueue           //
/////////////////////////////////////////////////


void MIDIRawMessageQueue::Reset() {
    next_in = next_out = 0;
    for (unsigned int i = 0; i < buffer.size(); i++)
        buffer[i] = MIDIRawMessage();
}


void MIDIRawMessageQueue::PutMessage(const MIDIRawMessage& msg) {
    buffer[next_in] = msg;
    next_in = (next_in + 1) % buffer.size();
    if (next_in == next_out)
        next_out = (next_out + 1) % buffer.size();          // we lose actual out message
}


MIDIRawMessage& MIDIRawMessageQueue::GetMessage() {
    static MIDIRawMessage msg;      // needed if we want to return a reference
    if (next_in == next_out)
        return msg;
    else {
        unsigned int old_out = next_out;
        next_out = (next_out + 1) % buffer.size();
        return buffer[old_out];
    }
}


MIDIRawMessage& MIDIRawMessageQueue::ReadMessage(unsigned int n) {
    static MIDIRawMessage msg;      // needed if we want to return a reference
    if (n >= GetLength())
        return msg;
    else
        return buffer[(next_out + n) % buffer.size()];
}


/////////////////////////////////////////////////
//           class MIDIOutDriver               //
/////////////////////////////////////////////////


MIDIOutDriver::MIDIOutDriver(int id) :
    processor(0), port_id(id), num_open(0) {
    static const char *TAG = "NICMIDI DRIVER";  
    try {//FCKX
        //ESP_LOGE(TAG,"start creation of RtMidiOut port"); 
        //port = new RtMidiOut();
        //ESP_LOGE(TAG,"executed creation of RtMidiOut port");
        ESP_LOGE(TAG,"start creation of MidiOutNimBLE port");  
        //do not create a new port, but use one that is available globally        
        port = new MidiOutNimBLE();
        ESP_LOGE(TAG,"executed creation of MidiOutNimBLE port");
        }
    catch (RtMidiError& error) {
        ESP_LOGE(TAG,"catch ERROR on creation of RtMidiOut port");
        error.printMessage();
        //FCKX
        //port = new RtMidiOut(RtMidi::RTMIDI_DUMMY);// A non functional MIDI out, which won't throw further exceptions
    }
}


MIDIOutDriver::~MIDIOutDriver() {
    port->closePort();
    delete port;
}


void MIDIOutDriver::Reset() {
    port->closePort();
    processor = 0;
    num_open = 0;
}


void MIDIOutDriver::OpenPort() {
    static const char *TAG = "NICMIDI DRIVER";
    if (num_open == 0) {
        try {
            ESP_LOGE(TAG,"MIDIOutDriver::OpenPort() port_id %d", port_id);
            port->openPort(port_id);
#if DRIVER_USES_MIDIMATRIX
            //out_matrix.Clear();
            out_matrix.Reset();
#endif
        }
        catch (RtMidiError& error) {
            error.printMessage();
            return;
        }
    }
    num_open++;

    std::cout << "OUT Port " << port->getPortName(port_id) << " open";
    if (num_open > 1)
        std::cout << " (" << num_open << " times)";
    std::cout<< std::endl;
}


void MIDIOutDriver::ClosePort() {
    static const char *TAG = "NICMIDI DRIVER";
    ESP_LOGE(TAG,"Entered MIDIOutDriver::ClosePort()");
    if (num_open == 1)
        port->closePort();
    if (num_open > 0) {
        num_open--;
        std::cout << "OUT Port " << port->getPortName(port_id) << " closed";
        if (num_open > 0)
            std::cout << " (open " << num_open << " times)";
        std::cout << std::endl;
    }
    else
        std::cout << "OUT Port " << port->getPortName()
        << "Attempt to close an already closed port!" << std::endl;
    ESP_LOGE(TAG,"Exiting MIDIOutDriver::ClosePort()");
}


void MIDIOutDriver::AllNotesOff(int chan) {
    MIDIMessage msg;
    static const char *TAG = "NICMIDI DRIVER";
        ESP_LOGE(TAG,"MIDIOutDriver::AllNotesOff");
    if (!port->isPortOpen())
        return;

    if (chan == -1) {
        for (int i = 0; i < 16; i++)
            AllNotesOff(i);
        return;
    }
    out_mutex.lock();

#if DRIVER_USES_MIDIMATRIX                      // send a note off for every note on in the out_matrix
    if(out_matrix.GetChannelCount(chan) > 0) {
        for(int note = 0; note < 128; ++note) {
            while(out_matrix.GetNoteCount(chan,note) > 0) {
                msg.SetNoteOff((unsigned char)chan, (unsigned char)note, 0);
                HardwareMsgOut(msg);
            }
        }
    }
    msg.SetControlChange(chan,C_DAMPER,0 );     // send a pedal off for every channel
    HardwareMsgOut(msg);
#endif // DRIVER_USES_MIDIMATRIX

    msg.SetAllNotesOff( (unsigned char)chan );
    HardwareMsgOut(msg);

    out_mutex.unlock();
}


void MIDIOutDriver::OutputMessage(const MIDITimedMessage& msg) {
    static const char *TAG = "MIDIOutDriver::OutputMessage";
    ESP_LOGI(TAG,"MIDIOutDriver::OutputMessage size: %d", msg.GetLength() );    

    // MIDITimedMessage is good also for MIDIMessage
    MIDITimedMessage msg_copy(msg);

    if (processor)
        processor->Process(&msg_copy);

    int i = 0;
    for( ; i < DRIVER_MAX_RETRIES; i++) {
        if (out_mutex.try_lock()) {
            HardwareMsgOut(msg_copy);
            out_mutex.unlock();
            break;
        }
        std::cerr << "busy driver (" << (i + 1) << ") ... " << std::endl;
        MIDITimer::Wait(1);
    }
    if (i == DRIVER_MAX_RETRIES)
        std::cerr << "MIDIOutDriver::OutputMessage() failed!" << std::endl;
}


void MIDIOutDriver::HardwareMsgOut(const MIDIMessage &msg) {
    static const char *TAG = "MIDIOutDriver::HardwareMsgOut";
    ESP_LOGI(TAG,"Entering..."); 
    if (!port->isPortOpen()) {
    ESP_LOGE(TAG,"MIDIOutDriver::HardwareMsgOut **** PORT IS NOT OPEN ****");    
    return;}
    msg_bytes.clear();
#if DRIVER_USES_MIDIMATRIX
    if (msg.IsChannelMsg())
        out_matrix.Process (msg);
#endif

    if (msg.IsSysEx()) {
        for (int i = 0; i < msg.GetSysEx()->GetLength(); i++)
            msg_bytes.push_back(msg.GetSysEx()->GetData(i));
        //std::cout << "Driver sent sysex of " << msg.GetSysEx()->GetLength() << " bytes ... ";
    }

    //else if (msg.IsReset())         // a reset message, with the same status of meta events
    //    msg_bytes.push_back(msg.GetStatus()) TODO: for now don't send reset messages

    else if (msg.IsMetaEvent())
        return;                     // don't send meta events

    else {                          // other messages
        msg_bytes.push_back(msg.GetStatus());
        if (msg.GetLength() > 1)
            msg_bytes.push_back(msg.GetByte1());
        if (msg.GetLength() > 2)
            msg_bytes.push_back(msg.GetByte2());
    }

    if (msg_bytes.size() > 0) {
        try {
            port->sendMessage(&msg_bytes);
        }
        catch (RtMidiError& error) {
            error.printMessage();
        }
        //std::cout << "Driver sent nonSysex message" << std::endl;
    }
    if (msg.IsSysEx()) // || msg.IsReset())
        MIDITimer::Wait(DRIVER_WAIT_AFTER_SYSEX);
}

#ifdef MIDI_IN_USED  //FCKX
/////////////////////////////////////////////////
//           class MIDIInDriver                //
/////////////////////////////////////////////////


MIDIInDriver::MIDIInDriver(int id, unsigned int queue_size) :
    processor(0), port_id(id), num_open(0), in_queue(queue_size) {
        static const char *TAG = "NICMIDI DRIVER INPUT";
/*  //FCKX
    try {
        port = new RtMidiIn();
        port->setCallback(HardwareMsgIn, this);
        port->ignoreTypes(false, true, true);
    }
*/
    try {//FCKX
        //ESP_LOGE(TAG,"start creation of RtMidiOut port"); 
        //port = new RtMidiOut();
        //ESP_LOGE(TAG,"executed creation of RtMidiOut port");
        ESP_LOGE(TAG,"start creation of MQTTMidiIn port");         
        port = new MQTTMidiIn();
        port->setCallback(HardwareMsgIn, this);
        port->ignoreTypes(false, true, true);
        ESP_LOGE(TAG,"executed creation of MQTTMidiIn port");
        }

    catch (RtMidiError& error) {  
        ESP_LOGE(TAG,"catch ERROR on creation of RtMidiOut port");   //FCKX
        error.printMessage();
        //port = new RtMidiIn(RtMidi::RTMIDI_DUMMY);// A non functional MIDI out, which won't throw further exceptions
    }
}

/* FCKX
error: deleting object of polymorphic class type 'MQTTMidiIn' which has non-virtual destructor might cause undefined behavior [-Werror=delete-non-virtual-dtor]
     delete port;
*/

MIDIInDriver::~MIDIInDriver() {
    port->closePort();
 //  delete port; //FCKX
}


void MIDIInDriver::Reset() {
    port->closePort();
    num_open = 0;
    in_queue.Reset();

    processor = 0;
}


void MIDIInDriver::OpenPort() {
    //FCKX probably use subscribe to MQTT topic here
    if (num_open == 0) {
        try {
            port->openPort(port_id);
        }
        catch (RtMidiError& error) {
            error.printMessage();
            return;
        }
    }
    num_open++;

    std::cout << "IN Port " << port->getPortName() << " open";
    if (num_open > 1)
        std::cout << " (" << num_open << " times)";
    std::cout<< std::endl;
}


void MIDIInDriver::ClosePort() {
    //FCKX probably use unsubscribe from MQTT topic here
    if (num_open == 1)
            port->closePort();
    if (num_open > 0) {
        num_open--;

        std::cout << "IN Port " << port->getPortName() << " closed";
        if (num_open > 0)
            std::cout << " (" << num_open << " times)";
        std::cout << std::endl;
    }
    else
        std::cout << "IN Port " << port->getPortName()
        << "Attempt to close an already closed port!" << std::endl;
}


void MIDIInDriver::FlushQueue() {
    in_mutex.lock();
    in_queue.Flush();
    in_mutex.unlock();
}


void MIDIInDriver::SetProcessor(MIDIProcessor* proc) {
    in_mutex.lock();
    processor = proc;
    in_mutex.unlock();
}


bool MIDIInDriver::InputMessage(MIDIRawMessage &msg) {
    //try to keep this code untouched !
    //let the MQTT handler put incoming messages in the queue
    if (!in_queue.IsEmpty()) {
        msg = in_queue.GetMessage();
        return true;
    }
    return false;
}


bool MIDIInDriver::ReadMessage(MIDIRawMessage& msg, unsigned int n) {
    if (in_queue.GetLength() > 0) {
        msg = in_queue.ReadMessage(n);
        return true;
    }
    return false;
}

void MIDIInDriver::HardwareMsgIn(double time,
                                 std::vector<unsigned char>* msg_bytes,
                                 void* p) {
    static const char *TAG = "HardwareMsgIn (driver.cpp)";                                
    ESP_LOGW(TAG,"A sign of life from HardwareMsgIn (make it protected again in driver.h!!!)");                             
 
    for (int i = 0; i < msg_bytes->size(); i++) {

    ESP_LOGI(TAG, "msg_bytes.at(%d) %u (0x%X)" ,i, msg_bytes->at(i), msg_bytes->at(i));
    };
    
    //changes for MQTT will NOT be here
    //This function is called as a call back.
    //The changes on the MQTT side must be where msg_bytes are prepared for feeding the callback with data 
    //The data are available in the MQTT event handler!
    
    //rough first analysis of what happens here
    //the message is entered into the procedure as msg_bytes (see above)
    //create a MIDITimedMessage
    //this is filled with data depending on the received msg_bytes (encoding)
    //after processing the message is put into a queue
    //this queue will be polled by.... manager(?)
    
    //most likely this routine is a callback to/from ..... can hopefully be used as an onMessage callback by the MQTT part
    //openPort and closePort will probably use MQTT subscribe / unsubscribe
    
    MIDIInDriver* drv = static_cast<MIDIInDriver*>(p);

    std::cout <<"Midi In PortName "<< drv->GetPortName() << " callback executed\n";

    if (!drv->port->isPortOpen() || msg_bytes->size() == 0) {
        ESP_LOGE(TAG,"Something is wrong with the Midi In Port or the message");
        ESP_LOGE(TAG,"drv->port->isPortOpen() %d", drv->port->isPortOpen() );
        ESP_LOGE(TAG,"msg_bytes->size() %d", msg_bytes->size());
    return; }


    drv->in_mutex.lock();
    MIDITimedMessage msg;
    msg.SetStatus(msg_bytes->operator[](0));        // in msg_bytes[0] there is the status byte
    if (msg.IsSysEx()) {
        ESP_LOGE(TAG,"RECEIVED SYSEX MESSAGE");
        msg.AllocateSysEx(msg_bytes->size());
        for (unsigned int i = 0; i < msg_bytes->size(); i++)
            msg.GetSysEx()->PutSysByte(msg_bytes->operator[](i));   // puts the 0xf0 also in the sysex buffer
    }
    else if (msg.GetStatus() == 0xff) { // this is a reset message, NOT a meta
    ESP_LOGE(TAG,"RECEIVED RESET MESSAGE (0xff)");
    }
    else {
        if (msg.GetLength() > 1) {
            msg.SetByte1(msg_bytes->operator[](1));
            ESP_LOGE(TAG,"msg.GetLength() > 1 HANDLED");
        }
        if (msg.GetLength() > 2) {
            msg.SetByte2(msg_bytes->operator[](2)); // byte3 surely 0 in non-meta messages
            ESP_LOGE(TAG,"msg.GetLength() > 2 HANDLED");
        }  
    }


    if (!msg.IsNoOp()) {                            // now we have a valid message
    
        ESP_LOGI(TAG,"!msg.IsNoOp BEFORE PROCESSOR %s", msg.MsgToText().c_str()); 
        if (drv->processor) {
            drv->processor->Process(&msg);          // process it with the in processor

        ESP_LOGI(TAG,"!msg.IsNoOp AFTER PROCESSOR %s", msg.MsgToText().c_str());                                            
        }  else {ESP_LOGI(TAG,"!msg.IsNoOp NO action by PROCESSOR required"); }
        drv->in_queue.PutMessage(MIDIRawMessage(msg,   // adds the message to the queue
                                                MIDITimer::GetSysTimeMs(),
                                                drv->port_id));
        ESP_LOGE(TAG,"added message to in_queue");                                        
        std::cout << "Got message, queue size: " << drv->in_queue.GetLength() << std::endl;
    }
    else {
        ESP_LOGE(TAG,"NO message NOTHING added to in_queue");     
    std::cout << "No message, queue size: " << drv->in_queue.GetLength() << std::endl;
    
    }
    drv->in_mutex.unlock();
}
#endif  //FCKX