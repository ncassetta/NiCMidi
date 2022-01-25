/* implementation of class MQTTMidiIn */

/*
* FCKX december 2021
*/
#include "esp_log.h"
#include "MQTTdriver.h" 

//#include <NimBLEDevice.h>
//commenting this may have led to missing out of vTaskDelay and portTICK_PERIOD_MS
//you may have to include some freertos headers
//but which one?

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "freertos/semphr.h"
//#include "freertos/queue.h"
//#include "freertos/timers.h" //for using software timers, NOT required for the nbDelay (?)

//#include "freertos/event_groups.h"


static const char *TAG = "MQTTDRIVER";

/*  OBSOLETE
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara
    Refactored back to IDF by H2zero

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/

/** NimBLE differences highlighted in comment blocks **/

/*******original********
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
***********************/

//bool deviceConnected = false;
//bool oldDeviceConnected = false;
//uint32_t value = 0;
#ifdef CONNECTEDTASK
 void connectedTask (void * parameter){
    for(;;) {  //loop forever
        // only act if connection status changes or if there is a stable connection
        if (deviceConnected) {
               //ESP_LOGE(TAG,"Testing NiCMidi functionality: MIDItimer MIDITickComponent, MIDIManager");
  //ESP_LOGE(TAG,"Testing NiCMidi functionality: test_midi_ports.cpp");    

//NOTE:  this is a CYCLIC TASK executed after the first connect to the BLE interface
//PUT THE INSTANTIATION PARTS IN THE MAIN_TEST_ ROUTINES OUTSIDE THIS LOOP
//OR FLAG THEM AS DONE TO PREVENT EXECUTING THEM AGAIN AND AGAIN
//THIS IS SOLVED FOR THE main_recorder EXAMPLE  
//  main_test_midiports(); //code above
  //ESP_LOGE(TAG,"Testing NiCMidi functionality: test_metronome.cpp SWITCHED OFF");
//  main_test_metronome(command); //code above
  //ESP_LOGE(TAG,"Testing NiCMidi functionality: test_component.cpp SWITCHED OFF");                                                                               
//  main_test_component(); //code above  DOESN'T EXIST 
               //A CALLBACK HERE                                                                                           
            //ESP_LOGW(TAG,"STABLE CONNECTION "); 
            /*
            //pCharacteristic->setValue((uint8_t*)&value, 4);
            midiPacket[2] = 0x90; //keyOn, channel 0
            midiPacket[4] = 127; //velocity
            pCharacteristic->setValue(midiPacket, 5);
            pCharacteristic->notify();
            //value++;
            vTaskDelay(1000/portTICK_PERIOD_MS);  // bluetooth stack will go into congestion, if too many packets are sent
            //pCharacteristic->setValue((uint8_t*)&value, 4);
            midiPacket[2] = 0x80; //keyOff, channel 0
            midiPacket[4] = 0; //velocity  
            pCharacteristic->setValue(midiPacket, 5);            
            pCharacteristic->notify();
            //value++;
            vTaskDelay(1000/portTICK_PERIOD_MS);  // bluetooth stack will go into congestion, if too many packets are sent
            */



 }
        // disconnecting
        if (!deviceConnected && oldDeviceConnected) {
            ESP_LOGW(TAG,"BLE disconnected"); 
            ESP_LOGW(TAG,"Do required stuff, depending on the needs for this lost connection");
            vTaskDelay(500/portTICK_PERIOD_MS); // give the bluetooth stack the chance to get things ready
  //vTaskDelay(5000); // give the bluetooth stack the chance to get things ready
  //portTICK_PERIOD_MS unknown in this scpoe !?!?!?
            ESP_LOGW(TAG,"Start advertising again"); 
            //DO THIS EXERCISE INSIDE THE MidiOutNimBLE CLASS            
            //pServer->startAdvertising();        // restart advertising
            //printf("start advertising\n");
            oldDeviceConnected = deviceConnected;
        }
        // connection established
        if (deviceConnected && !oldDeviceConnected) {
            ESP_LOGW(TAG,"BLE connected, do required stuff, depending on the needs for this connection");
            // do stuff here on connecting
            oldDeviceConnected = deviceConnected;
        }
        
        vTaskDelay(10/portTICK_PERIOD_MS); // Delay between loops to reset watchdog timer
    }
    
    vTaskDelete(NULL);
}
#endif // CONNECTEDTASK   

//MIDI data packet, taken from Brian Duhan arduino-esp32-BLE-MIDI / BLE_MIDI.ino

/*  //uint8_t midiPacket already defined in nimBLEdriver.cpp
    //PROBABLY WISE TO CREATE/RE-USE A STRUCTURE/CLASS WHERE MQTTdriver and niBLEdriver inherit this (AND MORE) from
uint8_t midiPacket[] = {
    0x80, //header
    0x80, //timestamp, not implemented
    0x00, //status
    0x3c, //== 60 == middle c
    0x00  //velocity
};
*/
 
/*******************************************************************/

/*
 //MOVE STRUCT definition to header file
struct NimBLEMidiOutData {
  NimBLEServer* client;//this "client" is actually a nimBLE server
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
*/

 //see RtMidi.cpp for detailed implementation
 //class MidiInAPI for hardware independent part
//othe classes for hardware implementation 

#ifdef MQTTPROCESSIN
static int MQTTProcessIn( jack_nframes_t nframes, void *arg )
//static int jackProcessIn( jack_nframes_t nframes, void *arg )
{
  /*  
  JackMidiData *jData = (JackMidiData *) arg;
  MidiInApi :: RtMidiInData *rtData = jData->rtMidiIn;
  jack_midi_event_t event;
  jack_time_t time;

  // Is port created?
  if ( jData->port == NULL ) return 0;

  void *buff = jack_port_get_buffer( jData->port, nframes );
  bool& continueSysex = rtData->continueSysex;
  unsigned char& ignoreFlags = rtData->ignoreFlags;

  // We have midi events in buffer
  int evCount = jack_midi_get_event_count( buff );
  for (int j = 0; j < evCount; j++) {
    MidiInApi::MidiMessage& message = rtData->message;
    jack_midi_event_get( &event, buff, j );

    // Compute the delta time.
    time = jack_get_time();
    if ( rtData->firstMessage == true ) {
      message.timeStamp = 0.0;
      rtData->firstMessage = false;
    } else
      message.timeStamp = ( time - jData->lastTime ) * 0.000001;

    jData->lastTime = time;

    if ( !continueSysex )
      message.bytes.clear();

    if ( !( ( continueSysex || event.buffer[0] == 0xF0 ) && ( ignoreFlags & 0x01 ) ) ) {
      // Unless this is a (possibly continued) SysEx message and we're ignoring SysEx,
      // copy the event buffer into the MIDI message struct.
      for ( unsigned int i = 0; i < event.size; i++ )
        message.bytes.push_back( event.buffer[i] );
    }

    switch ( event.buffer[0] ) {
      case 0xF0:
        // Start of a SysEx message
        continueSysex = event.buffer[event.size - 1] != 0xF7;
        if ( ignoreFlags & 0x01 ) continue;
        break;
      case 0xF1:
      case 0xF8:
        // MIDI Time Code or Timing Clock message
        if ( ignoreFlags & 0x02 ) continue;
        break;
      case 0xFE:
        // Active Sensing message
        if ( ignoreFlags & 0x04 ) continue;
        break;
      default:
        if ( continueSysex ) {
          // Continuation of a SysEx message
          continueSysex = event.buffer[event.size - 1] != 0xF7;
          if ( ignoreFlags & 0x01 ) continue;
        }
        // All other MIDI messages
    }

    if ( !continueSysex ) {
      // If not a continuation of a SysEx message,
      // invoke the user callback function or queue the message.
      if ( rtData->usingCallback ) {
        RtMidiIn::RtMidiCallback callback = (RtMidiIn::RtMidiCallback) rtData->userCallback;
        callback( message.timeStamp, &message.bytes, rtData->userData );
      }
      else {
        // As long as we haven't reached our queue size limit, push the message.
        if ( !rtData->queue.push( message ) )
          std::cerr << "\nMidiInJack: message queue limit reached!!\n\n";
      }
    }
  }

  return 0;
  */
}

#endif //MQTTPROCESSIN




 MQTTMidiIn :: MQTTMidiIn ()
 //MQTTMidiIn :: MQTTMidiIn ( const std::string &clientName, unsigned int queueSizeLimit)
//MQTTMidiIn :: MQTTMidiIn (const std::string &clientName)
{     ESP_LOGW(TAG,"MQTTMidiIn instantiation");
    //MQTTMidiIn::initialize("fckx_seq2"); 
     //Instantiation automatically calls initialize
     const std::string &clientName = "testName";
     MQTTMidiIn::initialize(clientName);  //ORIG    
}

#ifdef CONNECTON
void MQTTMidiIn :: connect()
{
  MQTTMidiData *data = static_cast<MQTTMidiData *> (apiData_);
  if ( data->client )
    return;
  //For MQTT init of client NOT required as it is already part of the basic setup
 
 /*
 // Initialize Jack client
  if (( data->client = jack_client_open( clientName.c_str(), JackNoStartServer, NULL )) == 0) {
    errorString_ = "MidiInJack::initialize: JACK server not running?";
    error( RtMidiError::WARNING, errorString_ );
    return;
  }
*/

  //MUST SEE WHAT IS THE ESSENCE OF THIS PART OF THE CODE FOR MQTT
  //It may mean that the VERY ESSENCE of connect() is seting the callback !
  //Also see equivalents for other OS's !!!
  //It is about the ProcessIn which is set as a callback here
  // THis could relate to HardwareMsgIn and the callback!  //FCKX! 
  /*
  jack_set_process_callback( data->client, jackProcessIn, data );
  jack_activate( data->client );
  */
}
#endif //CONNECTON

void MQTTMidiIn :: initialize( const std::string& clientName )
{
  ESP_LOGW(TAG,"MQTTMidiIn :: initialize");  
 // MQTTMidiData *data = new MQTTMidiData;
 MQTTMidiInData *data = new MQTTMidiInData;  //construct MQTTMidiInData with default constructor
                                             //check on these


#ifdef BLOCKTHEAPIDATA
  apiData_ = (void *) data;  //ORGINAL ORIGINAL
  
  
//error: 'apiData_' was not declared in this scope
//some example occurrence:   
//AlsaMidiData *data = static_cast<AlsaMidiData *> (apiData_);  
  
  
  //inputData_ = (void *) data;  //ORIGINAL


/*
this error is probably due to non-matching types
look up what was the type of the original variable (apiData_)

error: no match for 'operator=' (operand types are 'MQTTMidiIn::MQTTMidiInData' and 'void*')
 inputData_ = (void *) data;
 
 apiData_ and inputData_  both exist in RtMidi
 DO WE NEED BOTH?
 CHECK THIS: IS IN THE END ONE A SUB OBJECT OF THE OTHER ??
*/

  data->rtMidiIn = &inputData_;   //FCKX! ??????? is rtMidiIn OK?
                                  //----> have a look where this member is accessed in the Jack case
#endif //BLOCKTHEAPIDATA

  //data->port = NULL;                   //FCKX!
  //data->client = NULL;
  //this->clientName = clientName;
  #ifdef CONNECTON
  connect();
  #endif
}


MQTTMidiIn :: ~MQTTMidiIn ()
{
    MQTTMidiInData *data = static_cast<MQTTMidiInData *> (apiData_); 
    //close a connection if it exists
    MQTTMidiIn::closePort();
     ESP_LOGW(TAG, "Closed MQTTMidiIn NO CLEANUP: BEWARE of MEMORY LEAKS"); 
    //Cleanup
    //For MQTT no need to cleanup client as there are still other tasks for that client
/*
      if ( data->client )
    jack_client_close( data->client );
*/
//  delete data;  //ORIG BEWARE OF MEMORY LEAK
//delete &data;  

 
}


void MQTTMidiIn :: printData(void) {
  //development helper to check analyze proper operation of the private data object
  ESP_LOGW(TAG,"MQTTMidiIn private data object");  
    
}


void MQTTMidiIn :: openPort( unsigned int portNumber) {
  ESP_LOGW(TAG, "open MQTTMidiIn port"); 
  connected_ = true;
}

void MQTTMidiIn :: closePort( void ) {

   //why instantiate here, on closing ???????
  //how is this done in RtMidi   
  //there it is used in every procedure that accessess MidiData
  //it is probably a way to have a temp (local) access to the buffer etc
  //so you have to understand the code
  //....
  
  /*
  in RtMidi there is some jiggling with pointers in the initialize parts
  e.g.
  CoreMidiData *data = (CoreMidiData *) new CoreMidiData;
  data->client = client;
  data->endpoint = 0;
  apiData_ = (void *) data;
  inputData_.apiData = (void *) data;
  
  THIS MAY HAVE TO DO WITH THE FOLLOWIN ERROR
  */
  
   
   //MQTTMidiInData *data = static_cast< MQTTMidiInData *> (inputData_); 
 MQTTMidiInData *data = static_cast< MQTTMidiInData *> (apiData_);
 //FCKX!
 //error: invalid static_cast from type 'MQTTMidiIn::MQTTMidiInData' to type 'MQTTMidiIn::MQTTMidiInData*'
   //error disappears when changing MQTTMidiInData  inputData_; in MQTTdriver.h into  MQTTMidiInData * inputData_; 
   //but leads to:MQTTdriver.cpp:232:19: error: request for member 'usingCallback' in '((MQTTMidiIn*)this)->MQTTMidiIn::inputData_', which is of pointer type 'MQTTMidiIn::MQTTMidiInData*' (maybe you meant to use '->' ?)
/*
NOTE!!!! in a number of hardware initialize codes we have:

  // Save our api-specific connection information.
  WinMidiData *data = (WinMidiData *) new WinMidiData;
  apiData_ = (void *) data;
  inputData_.apiData = (void *) data;
  
  Where the last two lines are non-specific
  
  We still miss this code in MQTTdriver.cpp


*/
   
/*
  if ( data->endpoint ) {
    MIDIEndpointDispose( data->endpoint );
    data->endpoint = 0;
  }
*/
/*
  if ( data->port ) {
    MIDIPortDispose( data->port );
    data->port = 0;
  }
*/
//unsubscribe MQTT??
//update MQTTMidiInData

  connected_ = false;
}


bool MQTTMidiIn :: isPortOpen( ) {
    
  return connected_;  
}



unsigned int MQTTMidiIn :: getPortCount()
{
  //CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0, false );
  //return MIDIGetNumberOfDestinations();
  return 1;
}


std::string MQTTMidiIn :: getPortName(unsigned int portNumber)
{
  //must return clientName from niBLEMidiData ?
  return "MQTT_In";
}


//*********************************************************************//
//  Heritage from MidiInApi RtMidi.cpp:580 e.v.
//*********************************************************************//

//constructor and destructor of MidiInApi are obsolete here


void MQTTMidiIn :: setCallback( MQTTMidiIn::MQTTMidiCallback callback, void *userData )
{   //IMPLEMENTATION CHECKED OK against CLEAN RtMidi.cpp 580 
    if ( inputData_.usingCallback ) {  //FCKX!
  //if ( inputData_.usingCallback ) {
    //errorString_ = "MidiInApi::setCallback: a callback function is already set!";
 
    ESP_LOGE(TAG,"MidiInApi::setCallback: a callback function is already set!"); 
    //error( RtMidiError::WARNING, errorString_ );
    return;
  }

  if ( !callback ) {
    //errorString_ = "RtMidiIn::setCallback: callback function value is invalid!";
    //error( RtMidiError::WARNING, errorString_ );
    ESP_LOGE(TAG,"RtMidiIn::setCallback: callback function value is invalid!");

    return;
  }
/*
  inputData_.userCallback = callback;
  inputData_.userData = userData;
  inputData_.usingCallback = true;
*/ //FCKX! 
  ESP_LOGE(TAG,"MQTTMidiIn::setCallback: STORING CALLBACK IN inputData_");
  inputData_.userCallback = callback;
  inputData_.userData = userData;
  inputData_.usingCallback = true; 
}

void MQTTMidiIn :: cancelCallback()  //member MidiInApi in RtMidi 
{   //IMPLEMENTATION CHECKED OK against CLEAN RtMidi.cpp 599 
    /*  SOMEWHERE BEFORE THIS inputData_ should have been converted to an object
    where is it instantiated?
    error: request for member 'usingCallback' in '((MQTTMidiIn*)this)->MQTTMidiIn::inputData_', which is of pointer type 'MQTTMidiIn::MQTTMidiInData*' (maybe you meant to use '->' ?)
   if ( !inputData_.usingCallback )
    */   
    
  if ( !inputData_.usingCallback ) {
   // errorString_ = "RtMidiIn::cancelCallback: no callback function was set!";
   // error( RtMidiError::WARNING, errorString_ );
   ESP_LOGE(TAG,"RtMidiIn::cancelCallback: no callback function was set!"); 
 
 return;
  }
  /*
  inputData_.userCallback = 0;
  inputData_.userData = 0;
  inputData_.usingCallback = false;
  */
  inputData_.userCallback = 0;
  inputData_.userData = 0;
  inputData_.usingCallback = false;
  
}

void MQTTMidiIn :: ignoreTypes( bool midiSysex, bool midiTime, bool midiSense )
{ //IMPLEMENTATION CHECKED OK against CLEAN RtMidi.cpp 612 

  /* //FCKX!
    inputData_.ignoreFlags = 0;
  if ( midiSysex ) inputData_.ignoreFlags = 0x01;
  if ( midiTime ) inputData_.ignoreFlags |= 0x02;
  if ( midiSense ) inputData_.ignoreFlags |= 0x04;
  */
  inputData_.ignoreFlags = 0;
  if ( midiSysex ) inputData_.ignoreFlags = 0x01;
  if ( midiTime ) inputData_.ignoreFlags |= 0x02;
  if ( midiSense ) inputData_.ignoreFlags |= 0x04;
  
}

double MQTTMidiIn :: getMessage( std::vector<unsigned char> *message )
{ //IMPLEMENTATION CHECKED OK against CLEAN RtMidi.cpp 620
  message->clear();

  if ( inputData_.usingCallback ) 
  {
   // errorString_ = "RtMidiIn::getNextMessage: a user callback is currently set for this port.";
   // error( RtMidiError::WARNING, errorString_ );
    ESP_LOGE(TAG, "MQTTMidiIn::getMessage: ERROR !!!!! a user callback is currently set for this port."); //error( RtMidiError::DRIVER_ERROR, errorString_ );
    return 0.0;
  }

  double timeStamp;
    if ( !inputData_.queue.pop( message, &timeStamp ) ) {

    ESP_LOGE(TAG, "MQTTMidiIn::getMessage: queue.pop FALSE return timestamp 0.0"); 
    return 0.0; 
    }
    ESP_LOGE(TAG, "MQTTMidiIn::getMessage: INFO timestamp %f", timeStamp); 
  return timeStamp;
}

unsigned int MQTTMidiIn::MidiQueue::size( unsigned int *__back,
                                         unsigned int *__front )
{//IMPLEMENTATION CHECKED OK against CLEAN RtMidi.cpp 637
  // Access back/front members exactly once and make stack copies for
  // size calculation
  unsigned int _back = back, _front = front, _size;
  if ( _back >= _front )
    _size = _back - _front;
  else
    _size = ringSize - _front + _back;

  // Return copies of back/front so no new and unsynchronized accesses
  // to member variables are needed.
  if ( __back ) *__back = _back;
  if ( __front ) *__front = _front;
  return _size;
}

// As long as we haven't reached our queue size limit, push the message.
bool MQTTMidiIn::MidiQueue::push( const MQTTMidiIn::MidiMessage& msg )
{  //IMPLEMENTATION CHECKED OK against CLEAN RtMidi.cpp 656
  // Local stack copies of front/back
  unsigned int _back, _front, _size;

  // Get back/front indexes exactly once and calculate current size
  _size = size( &_back, &_front );

  if ( _size < ringSize-1 )
  {
    ring[_back] = msg;
    back = (back+1)%ringSize;
    return true;
  }

  return false;
}

bool MQTTMidiIn::MidiQueue::pop( std::vector<unsigned char> *msg, double* timeStamp )
{  //IMPLEMENTATION CHECKED OK against CLEAN RtMidi.cpp 674
  // Local stack copies of front/back
  unsigned int _back, _front, _size;

  // Get back/front indexes exactly once and calculate current size
  _size = size( &_back, &_front );

  if ( _size == 0 )
    return false;

  // Copy queued message to the vector pointer argument and then "pop" it.
  msg->assign( ring[_front].bytes.begin(), ring[_front].bytes.end() );
  *timeStamp = ring[_front].timeStamp;

  // Update front
  front = (front+1)%ringSize;
  return true;
}



  //! Set an error callback function to be invoked when an error has occured.
  /*!
    The callback function will be called whenever an error has occured. It is best
    to set the error callback function before opening a port.
  */
//  virtual void setErrorCallback( MQTTMidiCallback errorCallback = NULL, void *userData = 0 );



//initialize is only present in the classes nearest to the hardware 
/*
void MQTTMidiIn :: initialize ( const std::string& clientName)
{
  
}
*/

/*
void MQTTMidiIn :: setClientName ( const std::string& )
{

  errorString_ = "MQTTMidiIn::setClientName: this function is not implemented for the MACOSX_CORE API!";
  //error( RtMidiError::WARNING, errorString_ );

}
*/
/*
void MQTTMidiIn :: setPortName ( const std::string& )
{

  errorString_ = "MQTTMidiIn::setPortName: this function is not implemented for the MACOSX_CORE API!";
  //error( RtMidiError::WARNING, errorString_ );

}
*/
/*
void MQTTMidiIn :: openVirtualPort( const std::string &portName )
{
  NimBLEMidiOutData *data = static_cast<NimBLEMidiOutData *> (apiData_);

  if ( data->endpoint ) {
    errorString_ = "MQTTMidiIn::openVirtualPort: a virtual output port already exists!";
    ESP_LOGE(TAG, "%s" , errorString); //error( RtMidiError::WARNING, errorString_ );
    return;
  }

  // Create a virtual MIDI output source.
  MIDIEndpointRef endpoint;
  CFStringRef portNameRef = CFStringCreateWithCString( NULL, portName.c_str(), kCFStringEncodingASCII );
  OSStatus result = MIDISourceCreate( data->client, portNameRef, &endpoint );
  CFRelease( portNameRef );

  if ( result != noErr ) {
    errorString_ = "MQTTMidiIn::initialize: error creating OS-X virtual MIDI source.";
    ESP_LOGE(TAG, "%s" , errorString); //error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Save our api-specific connection information.
  data->endpoint = endpoint;
}
*/


#ifdef BLOCK2
double MQTTMidiIn :: getMessage( const std::vector<unsigned char>  *message )
//void MQTTMidiIn :: sendMessage( const unsigned char *message, size_t size )
{
    ESP_LOGW(TAG, "sendMessage  TO BE IMPLEMENTED"); 
    /*
    ESP_LOGW(TAG, "message.size %d", message->size());
    for (unsigned i=0; i<message->size(); i++)
    ESP_LOGW(TAG, "message.size %d", message->at(i));
*/
    //raw version copied from main::sendToMIDIOut
        //prepare for sending to output
    //convert MIDIMessage to midiPacket
    static const char *TAG = "sendToMIDIOut";  
    //unsigned long int mididata;
    uint8_t midiPacket[8];
    unsigned char header;
    unsigned char timestamp_low;
    unsigned char timestamp_high;
    unsigned char midi_status;
    unsigned char byte1;
    unsigned char byte2;
    unsigned char byte3;
    
    ESP_LOGI(TAG,"Prepare for sending data to output");

    
    //check raw byte values, befor implementing playing to BLE notify (output)
    //write a notify procedure that accepts MIDIMessage as input

       //check raw byte values, befor implementing playing to BLE notify (output)
    //write a notify procedure that accepts MIDIMessage as input
    
 //   printMsgBytes(msg1); //for debugging/devel
    //convert to mididata = Process (?)  you also may want to filter some messages
    /***************************************************************************************************************************** 
    * Midi data over BLE are sent exchanged according to the Specification for MIDI over Bluetooth Low Energy (BLE-MIDI, see docs)
    * Currently only 5 byte packets with single messages are supported
    *
    * Currently NOT supported are:
    *   - running status messages
    *   - multi-message packets
    *
    * Also, only Note on and Note Off messages are supported by the receiving sound board
    * MIDI_BLE supports a 13-bit timestamp, but these are not (yet) interpreted by the receiver. This application intends to send messages that should be played
    * immediately ("real-time") by the sound board
    * see: https://h2zero.github.io/esp-nimble-cpp/index.html
    * see: https://h2zero.github.io/esp-nimble-cpp/md_docs__usage_tips.html
    *
    ******************************************************************************************************************************/


   for (unsigned i=0; i<message->size(); i++) {
    ESP_LOGW(TAG, "message.size %d", message->at(i));
    midiPacket[i] = message->at(i);
   };

     /*
    timestamp_high = 0;
    timestamp_low = 0;
    

    midi_status = msg1.GetStatus();
    byte1 = msg1.GetByte1();
    byte2 = msg1.GetByte2();
    byte3 = msg1.GetByte3();
    */
    /*
    header = 0b11000000 + timestamp_high;
    midiPacket[0] = header;
    midiPacket[1] = 0b1000000 + timestamp_low;
    midiPacket[2] = midi_status; //byte1? 
    midiPacket[3] = byte1;  //byte2?
    midiPacket[4] = byte2;  //byte3?
  */
  /*
    //test data
    midiPacket[0] = 0x00;
    midiPacket[1] = 0x01;
    midiPacket[2] = 0x02;
    midiPacket[3] = 0x03;
    midiPacket[4] = 0x04;  
  */
    //<code> here
    
    
    
    //send to "hardware" interface   //WORDT DUS: receive from
    //<code here>
    
    

    //FCKX FCKX FCKX
    //connectionData.pCharacteristic->setValue(midiPacket, message->size());
    //connectionData.pCharacteristic->notify(); 
    
    //ble_notify_midi(pCharacteristic, mididata); this was suitable for midi thru


//METHOD2....
     
  // We use the MIDISendSysex() function to asynchronously send sysex
  // messages.  Otherwise, we use a single nimBLEMidi MIDIPacket.
  /*
  unsigned int nBytes = static_cast<unsigned int> (size);
  if ( nBytes == 0 ) {
    errorString_ = "MQTTMidiIn::sendMessage: no data in message argument!";
    ESP_LOGE(TAG, "%s" , errorString); //error( RtMidiError::WARNING, errorString_ );
    return;
  }
  
  MIDITimeStamp timeStamp = AudioGetCurrentHostTime();
  NimBLEMidiOutData *data = static_cast<NimBLEMidiOutData *> (apiData_);
  OSStatus result;

  if ( message[0] != 0xF0 && nBytes > 3 ) {
    errorString_ = "MQTTMidiIn::sendMessage: message format problem ... not sysex but > 3 bytes?";
    ESP_LOGE(TAG, "%s" , errorString); //error( RtMidiError::WARNING, errorString_ );
    return;
  }
 
  Byte buffer[nBytes+(sizeof( MIDIPacketList ))];
  ByteCount listSize = sizeof( buffer );
  MIDIPacketList *packetList = (MIDIPacketList*)buffer;
  MIDIPacket *packet = MIDIPacketListInit( packetList );

  ByteCount remainingBytes = nBytes;
  while ( remainingBytes && packet ) {
    ByteCount bytesForPacket = remainingBytes > 65535 ? 65535 : remainingBytes; // 65535 = maximum size of a MIDIPacket
    const Byte* dataStartPtr = (const Byte *) &message[nBytes - remainingBytes];
    packet = MIDIPacketListAdd( packetList, listSize, packet, timeStamp, bytesForPacket, dataStartPtr );
    remainingBytes -= bytesForPacket;
  }

  if ( !packet ) {
    errorString_ = "MQTTMidiIn::sendMessage: could not allocate packet list";
    ESP_LOGE(TAG, "%s" , errorString); //error( RtMidiError::DRIVER_ERROR, errorString_ );
    return;
  }

  // Send to any destinations that may have connected to us.
  if ( data->endpoint ) {
    result = MIDIReceived( data->endpoint, packetList );
    if ( result != noErr ) {
      errorString_ = "MQTTMidiIn::sendMessage: error sending MIDI to virtual destinations.";
      //error( RtMidiError::WARNING, errorString_ );
    }
  }
*/
/*
  // And send to an explicit destination port if we're connected.
  if ( connected_ ) {
    result = MIDISend( data->port, data->destinationId, packetList );
    if ( result != noErr ) {
      errorString_ = "MQTTMidiIn::sendMessage: error sending MIDI message to port.";
      //error( RtMidiError::WARNING, errorString_ );
    }
  }
  */
  
}
#endif