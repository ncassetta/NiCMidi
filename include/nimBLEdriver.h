/***********************************************
*definitions to make NimBLE driver variables globally accessible
*see: https://stackoverflow.com/questions/19929681/c-global-variable-declaration/19929727
*answer by Wolf, but learn more!
*/


#ifndef __NIMBLEDRIVER__
#define __NIMBLEDRIVER__


#define RTMIDI_DLL_PUBLIC __attribute__( (visibility( "default" )))

//#define RTMIDI_DLL_PUBLIC 

#include <string>   //maybe try to prevent using this
#include <vector>
#include <NimBLEDevice.h>

/*******************
*nimBLEdriver.h
********************/

/* 
NicMidi 211203:

I saw your driver.h file in the version 13 and I think that is the right way. You should implement a class with the same methods as the RtMidi ports and assign it the port attribute in the MIDIOutDriver class. NiCMidi uses only some methods of the RtMidi ports.
The implementation should be like this:


class MidiOutNimBLE {
    public:
                                    MidiOutNimBLE();
                                    ~MidiOutNimBLE();
                                
        void                         openPort(unsigned int portNumber=0);
        void                         closePort();
        virtual bool                 isPortOpen();
        unsigned int                 getPortCount()      { return 1; }
        std::string                  getPortName(unsigned int portNumber=0);
        void                         sendMessage(const std::vector<unsigned char> *message);
}
(see also the documentation of RtMidi)
Moreover you should modify the method MIDIManager::Init() (in the manager.h file) which attempts to open RtMidi ports, making it open your ports.
For the MIDIInDriver things are more complex as it relies on a callback called by RtMid

FCKX: make sure that the members of this class correpond to the state and state changes
that are expected by the equivalent class(es) in RtMidi and NicMidi

*/


struct NimBLEMidiOutData {
    NimBLEServer* pServer;
    NimBLEService* pService; 
    NimBLECharacteristic* pCharacteristic;
    NimBLEAdvertising* pAdvertising;
 //NimBLEServer* client;//this "client" is actually a nimBLE server
                       //note that devices can have both roles concurrently
                       //after making a connection
                       //more important: the difference between Central and Peripheral
                       //this device is .....(?)   It takes the lead in making connections
                       //it starts "advertising"
  //clienName ......                     
  //nimBLE_port_t *port;
 // bool connected_; //in RtMidi, this is in the parent class
  //nimBLE_ringbuffer_t *buffSize;     //required for MidiIn
  //nimBLE_ringbuffer_t *buffMessage;  //required for MidiIn
  //nimBLE_time_t lastTime;            //???
#ifdef HAVE_SEMAPHORE                  //depends on the OS
  sem_t sem_cleanup;
  sem_t sem_needpost;
#endif
  //MidiInApi :: RtMidiInData *rtMidiIn; //???
  };



class RTMIDI_DLL_PUBLIC MidiOutNimBLE   //:public MidiOutApi //(what does this parent class add?) 
{
    public:                          MidiOutNimBLE();
                                    //MidiOutNimBLE(const std::string &clientName);
                                    ~MidiOutNimBLE();
                                
        void                         openPort(unsigned int portNumber=0);
        void                         closePort();
        //virtual bool                 isPortOpen();
        inline bool isPortOpen() const { return connected_; }
        unsigned int                 getPortCount();// { return 1; }
        std::string                  getPortName(unsigned int portNumber=0);
        void                         sendMessage(const std::vector<unsigned char> *message);
        void                         sendMessage(const unsigned char *message, size_t size);
        //setcallback1               see rtMidi for examples
        //setcallback2
        
    protected:
        NimBLEMidiOutData connectionData;
        void initialize( const std::string& clientName  ); 
        void *apiData_;   //in the RtMidi case this is in class .... MidiApi
        bool connected_;  //in the RtMidi case this is in class .... MidiApi      
        //callback1  ee rtMidi for examples
        //callback2

};


#endif