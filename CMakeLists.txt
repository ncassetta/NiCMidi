#The minimal component CMakeLists.txt file simply registers the component to the build #system using idf_component_register:

#idf_component_register(
#                      SRCS "foo.c" "bar.c"
#                       INCLUDE_DIRS "include"
#                       REQUIRES mbedtls
#)
#SRCS is a list of source files (*.c, *.cpp, *.cc, *.S). These source files will be #compiled into the component library.

#INCLUDE_DIRS is a list of directories to add to the global include search path for any #component which requires this component, and also the main source files.

#REQUIRES is not actually required, but it is very often required to declare what other #components this component will use. See Component Requirements.

#A library with the name of the component will be built and linked into the final app. #Directories are usually specified relative to the CMakeLists.txt file itself, although #they can be absolute.

#There are other arguments that can be passed to idf_component_register. These arguments #are discussed here.

#See example component requirements and example component CMakeLists for more complete #component CMakeLists.txt examples.
#-----------------------------------------------------------------------------


#set(EXTRA_COMPONENT_DIRS "../esp-nimble-ccp")  #FCKX


idf_component_register( SRCS    
                                "./src/advancedsequencer.cpp" 
                                "./src/driver.cpp" 
				"./src/dump_tracks.cpp"
				"./src/fileread.cpp" 
				"./src/filereadmultitrack.cpp"
				"./src/manager.cpp"
				"./src/matrix.cpp"
				"./src/multitrack.cpp"
				"./src/msg.cpp" 
				"./src/midi.cpp"
#for some examples              
				"./examples/functions.cpp"
#hardware driver for output via BLE (nimBLE)
				"./src/nimBLEdriver.cpp"
#hardware driver for input via MQTT
                                "./src/MQTTdriver.cpp"
				"./src/notifier.cpp"
 				"./src/processor.cpp"
 				"./src/recorder.cpp"
#does not belong to the lib, but is a dependency in driver.h
 				"./src/RtMidi.cpp"
				"./src/sequencer.cpp"
				"./src/sysex.cpp"
				"./src/thru.cpp"
				"./src/tick.cpp"
                                "./src/timer.cpp"
				"./src/track.cpp"
                                
#SYSTEM COMPLAINS THAT IT CANNOT C:/Users/Fred/esp_projects/MIDI-sequencer/fckx_sequencer_v06b_NEW_OK_NiCMidi_msg_track/components/NiCMidi/msg.cpp , but it is in src!
                       INCLUDE_DIRS 
				"./src" 
				"include" 
				"../esp-nimble-cpp/src"  #cross-link!
				#"./rtmidi-4.0.0"
                      #REQUIRES rtmidi-4.0.0 #is not a requirement because RtMidi.cpp/.h are in the src folder
                      REQUIRES  esp-nimble-cpp  #for NimBLE bluetooth Midi out  
			        pthread         #for esp_pthread in timer.cpp/.h
#YOU MAY HAVE TO ADD REQUIRES RtMidi etc or an equivalent
)