/*
  Example of a basic custom MIDITickComponent which only plays a note
  every second. The file shows how to redefine the base class methods
  and how to add the component to the MIDIManager queue, making it
  effective.

  Copyright (C) 2019 - 2020 N.Cassetta
  ncassetta@tiscali.it

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program;
  if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "../include/tick.h"
#include "../include/manager.h"

using namespace std;


// If you want to implement your own MIDITickComponent derived class you must at least redefine
// the StaticTickProc() and TickProc() methods (and probably Start() and Stop() also).
// Before using the class you must add it to the MIDIManager queue with the
// MIDIManager::AddMIDITick().
// This is a very simple example which play a fixed note every second; see the comments
// to every method for details.
//
class TestComp : public MIDITickComponent {
    public:
                                TestComp();
        virtual void            Reset() {}
        virtual void            Start();
        virtual void            Stop();

    protected:
        static void             StaticTickProc(tMsecs sys_time, void* pt);
        virtual void            TickProc(tMsecs sys_time);


        static const tMsecs     NOTE_INTERVAL = 1000;   // the time (in msecs) between two note on
        static const tMsecs     NOTE_LENGTH = 400;      // the time between note on and note off

        tMsecs                  next_note_on;
        tMsecs                  next_note_off;
};


// The constructor must call the base class constructor with the priority and the StaticTickProc
// pointer as parameters.
// In our case, as we have only one object in the MIDIManager queue, the priority is irrelevant.
//
TestComp::TestComp() : MIDITickComponent(PR_PRE_SEQ, StaticTickProc) {}


// The Start() method should initialize the class and then call the base class Start(),
// which loads the sys_time_offset variable with the start time and enables the callback.
//
void TestComp::Start() {
    cout << "Starting the component ... " << endl;
    // opens MIDI out 0 port before playing the notes
    MIDIManager::GetOutDriver(0)->OpenPort();
    next_note_on = 0;                   // relative time of the 1st note on
    next_note_off = NOTE_LENGTH;        // relative time of the 1st note off
    MIDITickComponent::Start();
}


// The Stop() method should first call the base class Stop() which disables the callback.
//
void TestComp::Stop() {
    MIDITickComponent::Stop();
    cout << "Stopping the component ... " << endl;
    MIDIManager::GetOutDriver(0)->ClosePort();
}


// This is the typical implementation of the static callback.
// The MIDIManager gives it two parameters: the now absolute time (sys_time) and the object
// "this" pointer (as void*); the callback only should cast the void pointer to a class
// pointer and then call the (virtual) derived class callback (i.e. TickProc).
//
void TestComp::StaticTickProc(tMsecs sys_time, void* pt) {
    TestComp* c_pt = static_cast<TestComp*>(pt);
    c_pt->TickProc(sys_time);
}


// This is finally the object callback, which does all the work. Its parameter is the absolute
// now time (remember you have the start time in the sys_time_offset variable).
//
void TestComp::TickProc(tMsecs sys_time) {
    MIDITimedMessage msg;
    tMsecs deltat = sys_time - sys_time_offset; // the relative time (now time - start time)

    if (deltat >= next_note_off) {              // we must turn off the note
        msg.SetNoteOff(0, 60, 0);
        MIDIManager::GetOutDriver(0)->OutputMessage(msg);
                                                // sends a note off message to the MIDI 0 port
        cout << "and off" << endl;
        next_note_off += NOTE_INTERVAL;         // updates the next note off time
    }

    if (deltat >= next_note_on) {               // we must turn on the note
        msg.SetNoteOn(0, 60, 127);
        MIDIManager::GetOutDriver(0)->OutputMessage(msg);
                                                // sends a note on message to the MIDI 0 port
        cout << "Note on . . . ";
        next_note_on += NOTE_INTERVAL;          // updates the next note on time
    }
}

// The main() creates a class instance, adds it to the MIDIManager queue and then calls
// Start() and Stop() for enabling and disabling the callback
int main() {
    TestComp comp;                              // creates the component
    MIDIManager::AddMIDITick(&comp);            // adds it to the MIDIManager queue
    comp.Start();                               // starts the callback
    cout << "Waiting 10 secs ... " << endl;
    MIDITimer::Wait(10000);                     // waits 10 secs
    comp.Stop();                                // stops the callback
    cout << "Waiting 5 secs without playing ... " << endl;
    MIDITimer::Wait(5000);                      // waits 5 secs
    cout << "Exiting" << endl;
    return EXIT_SUCCESS;
}

