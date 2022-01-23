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


#include "../include/timer.h"

#include <iostream>         // for debugging


//#define ESP32_TRHEAD
//enable for using esp_thread component
//NOT implemented, keep snippets until everything is OK
#ifdef ESP32_THREAD
//#include "esp_log.h"        //FCKX!
//#include "esp_pthread.h"
#include <esp_pthread.h>

//https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/pthread.html
esp_pthread_cfg_t MIDITimer::create_config(const char *name, int core_id, int stack, int prio)
{
    auto cfg = esp_pthread_get_default_config();
    cfg.thread_name = name;
    cfg.pin_to_core = core_id;
    cfg.stack_size = stack;
    cfg.prio = prio;
    return cfg;
}

#endif // ifdef ESP32_THREAD



#define ESP32_TIMER
//Enable for using ESP-IDF / freeRTOS based timer
//In the end this is intended to replace the original thread_based timer, but only for ESP32 (if needed) 
//The timer must achieve that the "core" function (MIDITimer::tick_proc(MIDITimer::GetSysTimeMs(), MIDITimer::tick_param) ) is called periodically
//ESP-IDF uses freeRTOS that is partially modified (extended)
//https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-reference/system/freertos.html
//do we need a freeTOS "timer" or a freeRTOS "task"?
//for both options: the callback needs to run at a targeted specific moment in the future.
//definition of a fixed delay is NOT suitable. We need a "sleep untill".
#ifdef ESP32_TIMER

void myTimerCallback(TimerHandle_t pxTimer){
    std::cout << "test timer callback" << std::endl; 
};


/*
TimerHandle_t create_freeRTOSTimer(){
    
  TimerHandle_t timerHandle;
  int32_t Id = 0;
  int durationTicks = 500; 
  
  timerHandle  = xTimerCreate(  "Timer",          // Just a text name, not used by the kernel.
                                durationTicks,    // The timer period in ticks.
                                pdTRUE,           // The timer will auto-reload itself when it expires.
                                ( void * ) Id,    // unique id.
                                myTimerCallback   // Callback when timer expires.
                             );  
    return timerHandle;  
};
*/

#endif




const MIDITimer::timepoint MIDITimer::sys_clock_base = std::chrono::steady_clock::now();

unsigned int MIDITimer::resolution = MIDITimer::DEFAULT_RESOLUTION;

void* MIDITimer::tick_param = 0;
MIDITick* MIDITimer::tick_proc = 0;
std::atomic<int> MIDITimer::num_open(0);
MIDITimer::timepoint MIDITimer::current;

#ifndef ESP32_TIMER
std::thread MIDITimer::bg_thread;
#endif

// We need this for assuring the destructor is called at exit, stopping the timer and joining the bg_thread
// Without this we have errors if exiting while the timer is open
static MIDITimer dummy;

MIDITimer::MIDITimer() {
    //std::cout << "MIDITimer constructor" << std::endl;
}

MIDITimer::~MIDITimer() {
    //std::cout << "MIDITimer destructor" << std::endl;
    HardStop();             // this joins the bg_thread or stops freeRTOSTimer
 };



void MIDITimer::SetResolution(unsigned int res) {
    int was_open = num_open;
    HardStop();
    resolution = res;
    if (was_open > 0) {
        Start();
        num_open = was_open;
    }
}


void MIDITimer::SetMIDITick(MIDITick t, void* tp) {
    int was_open = num_open;
    HardStop();
    tick_proc = t;
    tick_param = tp;
    if (was_open > 0) {
        Start();
        num_open = was_open;
    }
}


bool MIDITimer::Start () {
    //std::cout << "MIDITimer::Start enter with num_open: " << num_open << std::endl;

    if (tick_proc == 0)
        return false;                           // Callback not set

    num_open++;

    if (num_open == 1) {                       // Must create thread or start timer (ESP32)
        current = std::chrono::steady_clock::now();
       
#ifdef ESP32_TIMER
    //create timer  //this may be done earlier, e.g. in the MIDITimer constructor
    
    //TimerHandle_t timerHandle;
    int32_t Id = 0;
    
    
    //int periodTicks =  MIDITimer::resolution % portTICK_PERIOD_MS ; 
 std::cout << "Timer resolution " << MIDITimer::resolution << std::endl; 
    std::cout << "portTICK_PERIOD_MS " << portTICK_PERIOD_MS << std::endl; 
int periodTicks = MIDITimer::resolution / portTICK_PERIOD_MS;    
  std::cout << "Calculated periodTicks " << periodTicks << std::endl; 

    
    //need to see carefully if this leads to a fixed period BETWEEN CALLS
    TimerHandle_t freeRTOSTimer  = xTimerCreate("freeRTOSTimer",          // Just a text name, not used by the kernel.
                                                 periodTicks,    // The timer period in ticks.
                                                 pdTRUE,           // The timer will auto-reload itself when it expires.
                                                ( void * ) Id,    // unique id.
                                                MIDITimer::freeRTOSTimerCallback   // Callback when timer expires.                                 
                                                ); 
    // MIDITimer::freeRTOSTimer = MIDITimer::create_freeRTOSTimer();
     std::cout << "Timer started with " << periodTicks << " system ticks resolution" << std::endl; 
     std::cout << "Timer open with " << periodTicks*portTICK_PERIOD_MS << " msecs resolution" << std::endl;        
    //start timer
    xTimerStart( freeRTOSTimer, 0 );
    }
#else
    bg_thread = std::thread(ThreadProc);
    std::cout << "Timer open with " << resolution << " msecs resolution" << std::endl; 
    //where is the resolution variable used? 
}
    else   //Nic 220112
        std::cout << "Dummy Timer::Start()" << std::endl;    //Nic 220112

    
#endif
        

    

    //std::cout << "MIDITimer::Start exit with num_open: " << num_open << std::endl;
    return true;
}


void MIDITimer::Stop() {
    //static const char *TAG = "MIDITimer::Stop";
    //ESP_LOGE(TAG,"Enter with num_open %d", num_open);
    std::cout << "MIDITimer::Stop enter with num_open: " << num_open << std::endl;
    if (num_open > 0) {
        num_open--;
        if (num_open == 0) {  //FCKX! 0
            
            #ifdef ESP32_TIMER
            std:: cout << "MIDITimer::Stop num_open == 0 : GOING TO CALL xTimerStop(freeRTOSTimer,0)" << std::endl;
            //xTimerStop(freeRTOSTimer,0);
              //this will be done in the callback, based on num_open == 0
            std:: cout << "Timer stopped by MIDITimer::Stop()" << std::endl;
        }  

  #else
            std:: cout << "MIDITimer::Stop num_open == 0 : GOING TO CALL bg_thread.join()" << std::endl;
            bg_thread.join();
                 std:: cout << "Timer stopped by MIDITimer::Stop()" << std::endl;
        }       
        else   //Nic 220112
            std::cout << "Dummy MIDITimer::Stop()" << std::endl;  //Nic 220112         
         
            #endif
            

    }
    //ESP_LOGE(TAG,"Exit with num_open %d", num_open);
    std::cout << "MIDITimer::Stop exit with num_open: " << num_open << std::endl;
}


void MIDITimer::HardStop() {
    if (num_open > 0) {
        num_open = 0;
            #ifdef ESP32_TIMER
            //xTimerStop(freeRTOSTimer,0);
            //don't have to stop the timer here
            //this will be done in the callback, based on num_open == 0
            #else
            bg_thread.join();
            #endif
        std:: cout << "Timer stopped by MIDITimer::HardStop()" << std::endl;
    }
}


#ifndef ESP32_TIMER
// This is the background thread procedure
void MIDITimer::ThreadProc() {
    duration tick(resolution);
    //std::cout << "before while block MIDITimer::ThreadProc() num_open: " << num_open << std::endl; //FCKX!
    while(MIDITimer::num_open) {
        // execute the supplied function
        // std::cout << "inside while block MIDITimer::ThreadProc() num_open: " << num_open << std::endl; //FCKX!
        MIDITimer::tick_proc(MIDITimer::GetSysTimeMs(), MIDITimer::tick_param);
        // find the next timepoint and sleep until it
        current += tick;
        std::this_thread::sleep_until(current);
    }
    //std::cout << "after while block MIDITimer::ThreadProc() num_open: " << num_open << std::endl; //FCKX!
}

#else
    // This is the freerTOS tick procedure
void MIDITimer::freeRTOSTimerCallback(TimerHandle_t pxTimer) {
    duration tick(resolution);  //for what do we need this?
 //   std::cout << "MIDITimer::freeRTOSTimerCallback() num_open: " << num_open << std::endl; //FCKX!
 //   while(MIDITimer::num_open) {  //doesn't need to cycle, this is achieved by the timer
        // execute the supplied function
        // std::cout << "inside while block MIDITimer::ThreadProc() num_open: " << num_open << std::endl; //FCKX!
        MIDITimer::tick_proc(MIDITimer::GetSysTimeMs(), MIDITimer::tick_param);
        // find the next timepoint and sleep until it
        current += tick;
          //doesn't need to sleep, this is achieved by the timer
  //      std::this_thread::sleep_until(current);
         if (num_open == 0) {
             std::cout << "MIDITimer::freeRTOSTimerCallback() num_open: " << num_open << " **STOPPING freeRTOSTimer** " <<std::endl; //FCKX!
             xTimerStop(pxTimer,0);
             };
  //  }
   // std::cout << "after while block MIDITimer::ThreadProc() num_open: " << num_open << std::endl; //FCKX!

}
#endif
