/***********************************************
* MQTTdriver for MIDI input , to be templated on:
*
*   - nimBLEdriver output port
*   - input driver code in RtMidi
*
* To be interfaced with manager.cpp and driver.cpp code (these codes have already been adapted. See MQTTMidiIn class.
*/


#ifndef __MQTTDRIVER__
#define __MQTTDRIVER__

#include <string>   //maybe try to prevent using this
#include <vector>
//#include <NimBLEDevice.h>

/*******************
*MQTTdriver.h
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
#ifdef UNBLOCK1
struct MQTTMidiInData {
 /*   
    NimBLEServer* pServer;
    NimBLEService* pService; 
    NimBLECharacteristic* pCharacteristic;
    NimBLEAdvertising* pAdvertising;
*/
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
  //MidiInApi :: MQTTMidiInData *rtMidiIn; //???
  };
#endif //UNBLOCK1


class MQTTMidiIn   //:public MidiInApi //(what does this parent class add?) 
{
    //template of this class taken from RtMidiIn class (RtMidi.h)
    //also see: MidiInApi and the various hardware dependent implementations in RtMidi.cpp
    
 public:
      
    //! User callback function type definition.
    //typedef void (*RtMidiCallback)( double timeStamp, std::vector<unsigned char> *message, void *userData );
    typedef void (*MQTTMidiCallback)( double timeStamp, std::vector<unsigned char> *message, void *userData );
       MQTTMidiIn();
    //MQTTMidiIn( const std::string &clientName, unsigned int queueSizeLimit);






    //! If a MIDI connection is still open, it will be closed by the destructor.
    // ~MQTTMidiIn();
    ~MQTTMidiIn (void) throw();
    //! Open a MIDI input connection given by enumeration number.
    
    //development helper to check analyze proper operation of the private data object        
    void printData(void);
            
    void   openPort(unsigned int portNumber=0);
         //! Close an open MIDI connection (if one exists).
    void   closePort(void);
          //! Returns true if a port is open and false if not.
  /*!
      Note that this only applies to connections made with the openPort()
      function, not to virtual ports.
  */
    //virtual bool                 isPortOpen() const;
     bool                 isPortOpen();
      // inline bool isPortOpen() const { return connected_; }   
    //inline bool isPortOpen() const { return connected_; }
    unsigned int                 getPortCount();// { return 1; }
    std::string                  getPortName(unsigned int portNumber=0);
   
   
  //! Fill the user-provided vector with the data bytes for the next available MIDI message in the input queue and return the event delta-time in seconds.
  /*!
    This function returns immediately whether a new message is
    available or not.  A valid message is indicated by a non-zero
    vector size.  An exception is thrown if an error occurs during
    message retrieval or an input connection was not previously
    established.
  */
  double getMessage( std::vector<unsigned char> *message );

   
   
        /*
        * setCallback              see rtMidi for interface
        * class RTMIDI_DLL_PUBLIC RtMidiIn : public RtMidi
        */
          //! Set a callback function to be invoked for incoming MIDI messages.
    void setCallback( /*RtMidiCallback*/ MQTTMidiCallback callback, void *userData = 0 );
    /*  
          class RTMIDI_DLL_PUBLIC MidiInApi : public MidiApi
          void setCallback( RtMidiIn::RtMidiCallback callback, void *userData );
  //! Cancel use of the current callback function (if one exists).
 
    Subsequent incoming MIDI messages will be written to the queue
    and can be retrieved with the \e getMessage function.
  */
    void cancelCallback();
       
        
        //call in driver.cpp: port->setCallback(HardwareMsgIn, this);
        //ignoreTypes see rtMidi for interface
        /*
                * class RTMIDI_DLL_PUBLIC RtMidiIn : public RtMidi
          void ignoreTypes( bool midiSysex = true, bool midiTime = true, bool midiSense = true );

                class RTMIDI_DLL_PUBLIC MidiInApi : public MidiApi
                 virtual void ignoreTypes( bool midiSysex, bool midiTime, bool midiSense );

        //call in driver.cpp: port->ignoreTypes(false, true, true);
*/

  //! Specify whether certain MIDI message types should be queued or ignored during input.
  /*!
    By default, MIDI timing and active sensing messages are ignored
    during message input because of their relative high data rates.
    MIDI sysex messages are ignored by default as well.  Variable
    values of "true" imply that the respective message type will be
    ignored.
  */
  //virtual?
  void ignoreTypes( bool midiSysex = true, bool midiTime = true, bool midiSense = true );


  //! Set an error callback function to be invoked when an error has occured.
  /*!
    The callback function will be called whenever an error has occured. It is best
    to set the error callback function before opening a port.
  */
 // virtual void setErrorCallback( MQTTMidiCallback errorCallback = NULL, void *userData = 0 );



/*
// **************************************************************** //
//
// Inline RtMidiIn and RtMidiOut definitions.
// what is the purpose of INLINE definitions in general?
// **************************************************************** //

inline RtMidi::Api RtMidiIn :: getCurrentApi( void ) throw() { return rtapi_->getCurrentApi(); }
inline void RtMidiIn :: openPort( unsigned int portNumber, const std::string &portName ) { rtapi_->openPort( portNumber, portName ); }
inline void RtMidiIn :: openVirtualPort( const std::string &portName ) { rtapi_->openVirtualPort( portName ); }
inline void RtMidiIn :: closePort( void ) { rtapi_->closePort(); }
inline bool RtMidiIn :: isPortOpen() const { return rtapi_->isPortOpen(); }
inline void RtMidiIn :: setCallback( RtMidiCallback callback, void *userData ) { static_cast<MidiInApi *>(rtapi_)->setCallback( callback, userData ); }
inline void RtMidiIn :: cancelCallback( void ) { static_cast<MidiInApi *>(rtapi_)->cancelCallback(); }
inline unsigned int RtMidiIn :: getPortCount( void ) { return rtapi_->getPortCount(); }
inline std::string RtMidiIn :: getPortName( unsigned int portNumber ) { return rtapi_->getPortName( portNumber ); }
inline void RtMidiIn :: ignoreTypes( bool midiSysex, bool midiTime, bool midiSense ) { static_cast<MidiInApi *>(rtapi_)->ignoreTypes( midiSysex, midiTime, midiSense ); }
inline double RtMidiIn :: getMessage( std::vector<unsigned char> *message ) { return static_cast<MidiInApi *>(rtapi_)->getMessage( message ); }
inline void RtMidiIn :: setErrorCallback( RtMidiErrorCallback errorCallback, void *userData ) { rtapi_->setErrorCallback(errorCallback, userData); }

        */
        
  // A MIDI structure used internally by the class to store incoming
  // messages.  Each message represents one and only one MIDI message.
  struct MidiMessage {
    std::vector<unsigned char> bytes;

    //! Time in seconds elapsed since the previous message
    double timeStamp;

    // Default constructor.
    MidiMessage()
      : bytes(0), timeStamp(0.0) {}
  };

  struct MidiQueue {
    unsigned int front;
    unsigned int back;
    unsigned int ringSize;
    MidiMessage *ring;

    // Default constructor.
    MidiQueue()
      : front(0), back(0), ringSize(0), ring(0) {}
    bool push( const MidiMessage& );
    bool pop( std::vector<unsigned char>*, double* );
    unsigned int size( unsigned int *back=0, unsigned int *front=0 );
  };

  // The MQTTMidiInData (RtMidiInData) structure is used to pass private class data to
  // the MIDI input handling function or thread.
  struct MQTTMidiInData {
    MidiQueue queue;
    MidiMessage message;
    unsigned char ignoreFlags;
    bool doInput;
    bool firstMessage;
    void *apiData;
    bool usingCallback;
    MQTTMidiIn::MQTTMidiCallback userCallback;
    void *userData;
    bool continueSysex;
    

    // Default constructor.
    MQTTMidiInData()
      : ignoreFlags(7), doInput(false), firstMessage(true), apiData(0), usingCallback(false),
        userCallback(0), userData(0), continueSysex(false) {}
  };

 
    protected:

       MQTTMidiInData inputData_;  //ORIGINAL
       //MQTTMidiInData * inputData_;  //FCKX! 

       //If defined as pointer, many references with -> must be changed
       //If NOT defined as pointer: 
 /*      
../components/NiCMidi/src/MQTTdriver.cpp: In member function 'void MQTTMidiIn::closePort()':
../components/NiCMidi/src/MQTTdriver.cpp:215:69: error: invalid static_cast from type 'MQTTMidiIn::MQTTMidiInData' to type 'MQTTMidiIn::MQTTMidiInData*'
    MQTTMidiInData *data = static_cast< MQTTMidiInData *> (inputData_);
                                                                     ^
../components/NiCMidi/src/MQTTdriver.cpp:215:20: warning: unused variable 'data' [-Wunused-variable]
    MQTTMidiInData *data = static_cast< MQTTMidiInData *> (inputData_);
*/
       
        void initialize( const std::string& clientName  );  //ORIG
        
        void *apiData_;   //in the RtMidi case this is in class .... MidiApi
        bool connected_;  //in the RtMidi case this is in class .... MidiApi      
        //callback1  ee rtMidi for examples
        //callback2

};


#endif