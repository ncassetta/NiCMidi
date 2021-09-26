lib_LIBRARIES = lib/libnicmidi.a
lib_libnicmidi_a_SOURCES = src/advancedsequencer.cpp  src/driver.cpp  src/dump_tracks.cpp  src/fileread.cpp        \
                       	   src/filereadmultitrack.cpp  src/filewrite.cpp  src/filewritemultitrack.cpp              \
                       	   src/manager.cpp  src/matrix.cpp  src/metronome.cpp  src/midi.cpp  src/multitrack.cpp    \
                       	   src/msg.cpp  src/notifier.cpp  src/processor.cpp src/recorder.cpp src/sequencer.cpp     \
                       	   src/smpte.cpp  src/sysex.cpp  src/thru.cpp  src/tick.cpp  src/timer.cpp  src/track.cpp  \
                           rtmidi-4.0.0/RtMidi.cpp                                                                 \
                       	   include/advancedsequencer.h  include/driver.h  include/dump_tracks.h                    \
                       	   include/fileread.h  include/filereadmultitrack.h  include/filewrite.h                   \
                           include/filewritemultitrack.h  include/manager.h  include/matrix.h  include/metronome.h \
                           include/midi.h  include/multitrack.h  include/msg.h  include/notifier.h                 \
                           include/processor.h include/recorder.h include/sequencer.h  include/smpte.h             \
                           include/sysex.h  include/thru.h  include/tick.h  include/timer.h  include/track.h       \
                           rtmidi-4.0.0/RtMidi.h

noinst_PROGRAMS = examples/test_advancedsequencer  examples/test_component  examples/test_metronome  \
                  examples/test_midiports examples/test_recorder  examples/test_sequencer            \
                  examples/test_stepsequencer examples/test_thru  examples/test_writefile

AM_CXXFLAGS = -Wall -I$(top_srcdir)

examples_test_advancedsequencer_SOURCES = examples/test_advancedsequencer.cpp examples/functions.cpp examples/functions.h
examples_test_advancedsequencer_LDADD = lib/libnicmidi.a

examples_test_component_SOURCES = examples/test_component.cpp
examples_test_component_LDADD = lib/libnicmidi.a

examples_test_metronome_SOURCES = examples/test_metronome.cpp examples/functions.cpp examples/functions.h
examples_test_metronome_LDADD = lib/libnicmidi.a

examples_test_midiports_SOURCES = examples/test_midiports.cpp
examples_test_midiports_LDADD = lib/libnicmidi.a

examples_test_recorder_SOURCES = examples/test_recorder.cpp examples/functions.cpp examples/functions.h
examples_test_recorder_LDADD = lib/libnicmidi.a

examples_test_sequencer_SOURCES = examples/test_sequencer.cpp examples/functions.cpp examples/functions.h
examples_test_sequencer_LDADD = lib/libnicmidi.a

examples_test_stepsequencer_SOURCES = examples/test_stepsequencer.cpp examples/functions.cpp examples/test_stepsequencer.h examples/functions.h
examples_test_stepsequencer_LDADD = lib/libnicmidi.a

examples_test_thru_SOURCES = examples/test_thru.cpp examples/functions.cpp examples/functions.h
examples_test_thru_LDADD = lib/libnicmidi.a

examples_test_writefile_SOURCES = examples/test_writefile.cpp
examples_test_writefile_LDADD = lib/libnicmidi.a

EXTRA_DIST = doc doxygen examples lib rtmidi-4.0.0 configure.ac NiCMidi_linux.cbp


