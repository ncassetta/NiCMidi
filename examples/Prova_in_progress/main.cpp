#include <iostream>

#include "../../include/manager.h"

using namespace std;

int main()
{
    MIDIManager manager;
    char a;

    for (int i = 0; i < manager.GetNumMIDIOuts(); i++)
        std::cout << "Device " << i << ": " << manager.GetMIDIOutName(i) << std::endl;

    std::cout << "Seq play" << std::endl;
    manager.SeqPlay();

    std::cout << "Waiting 10 secs ..." << std::endl;
    for(int i = 1; i <= 10; i++) {
       std::cout << i << std::endl;
       Wait(1000);
    }


    std::cout << "Seq stop" << std::endl;
    manager.SeqStop();

    return 0;
}
