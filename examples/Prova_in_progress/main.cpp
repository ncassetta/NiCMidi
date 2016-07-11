#include <iostream>

//#include "../../include/manager.h"
#include "../../rtmidi-2.1.1/RtMidi.h"

using namespace std;



void mycallback(double d, vector<unsigned char>* v, void* p) {
    static unsigned int rec_msg = 0;

    cout << "Received messages: " << ++rec_msg << endl;
}


int main()
{
    RtMidiIn inport;
    inport.setCallback(mycallback, 0);
    inport.openPort(0);
    cout << "Press RETURN to close the port\n";
    cin.get();
    inport.closePort();
    cout << "Correctly terminated!\n";

    return 0;
}
