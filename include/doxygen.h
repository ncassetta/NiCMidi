#ifndef DOXYGEN_H_INCLUDED
#define DOXYGEN_H_INCLUDED


/// \mainpage
/// myjdkmidi is a MIDI library written in C++ by Nicola Cassetta, implementing objects for playing, recording, editing
/// and saving MIDI content. It was originally a fork of jdkmidi, an old similar library written by J. D. Koftinoff and
/// no more updated. The author has rewritten the old code, adding many new features and using a more modern C++ style.
///
/// + \subpage LOAD_PLAY
/// + \subpage MESS_TRACK_MULTI
/// + \subpage HOW_PLAYS
/// + \subpage MIDIENUM


/// \page LOAD_PLAY Loading and playing MIDI files
/// If you only want to load and play MIDI files you can use the AdvancedSequencer object. It is an all-in-one class easy to
/// use and with all the common features of a sequencer. These features include:
/// + Loading MIDI files
/// + Playing and stopping the file at any time
/// + Jumping from a time to another, even when playing
/// + Muting, soloing, transposing, velocity scaling for individual tracks
/// + MIDI port assign and time shifting for individual tracks
/// + Global tempo scaling
///
/// Here is a simple example of its usage:
/// \code
/// #include "advancedsequencer.h"
/// #include "timer.h"                  // for Wait()
///
/// int main() {
///    AdvancedSequencer seq;           // creates the AdvancedSequencer
///    seq.Load("twinlkle.mid");        // loads a MIDI file into it
///    seq.Play();                      // plays the file
///    while (seq.IsPlaying())
///       MIDITimer::Wait(1000);        // waits until the end (the sequencer auto stops)
/// }
/// \endcode
///
///


/// \page MESS_TRACK_MULTI MIDI messages, tracks and multitrack
/// Let's examine how myjdkmidi stores and manipulates the MIDI content.
///
/// The %MIDIMessage and %MIDITimedMessage classes
/// ---------------------------------------------
///
/// According to the MIDI standard, a MIDI message is a group of bytes containing musical data: the first is the **status
/// byte**, giving information on what type of message it is, and the others are the data bytes (only real-time MIDI messages
/// consists of an unique byte). Moreover, MIDI System Exclusive (SysEx) messages can be followed by an undefined number of
/// bytes allowing, for example, the dumping of voice data from a machine to another. You should be familiar with the various
/// types of messages and their codify in hexadecimal values.
/// The MIDIMessage class allows you to create, edit or inspect MIDI messages without worrying about hexadecimal values: it
/// consists of a status byte, three data bytes for MIDI data, and a pointer to a MIDISystemExclusive object (a buffer that
/// can store any amount of bytes), with lots of methods for setting and inspecting data.
/// Here is a simple example of its usage:
/// \code
/// #include "msg.h"
///
/// int main() {
///    MIDIMessage msg1, msg2, msg3;    // creates three MIDIMessage
///    msg1.SetNoteOn(0, 60, 100);      // msg1 becomes a Note On, channel 1, note 60, velocity 100
///    msg2.SetVolumeChange(0, 127);    // msg2 becomes a Volume Change (CC 7), channel 1, volume 127
///    msg3.SetTimeSig(4, 4);           // msg 3 becomes a system Time Signature, 4/4
///    msg1.SetChannel(msg1.GetChannel() + 1);
///                                     // increments the msg1 channel by one
///    msg2.SetControllerValue(msg2.GetControllerValue() - 10);
///                                     // decrements the msg2 volume by 10
///    std::cout << msg1.MsgToText();   // prints a description of msg1
///    std::cout << msg2.MsgToText();   // prints a description of msg2
///    std::cout << msg3.MsgToText();   // prints a description of msg3
/// }
/// \endcode
///
/// MIDIMessage objects can be sent to an hardware MIDI port by the MIDIOutDriver::OutputMessage() method.
/// \code
/// #include "msg.h"
/// #include "manager.h"
///
/// MIDIOutDriver* port = MIDIManager::GetOutPort(0);
///                                     // gets a pointer to the driver of the 1st hardware out
///                                     // port in the system
/// port->OpenPort();                   // you must open the port before sending MIDI messages
/// MIDIMessage msg;
/// msg.SetNoteOn(0, 60, 100);          // makes msg1 a Note On message
/// port->OutputMessage(&msg1);         // outputs the message (the note should be heard)
/// MIDITimer::Wait(2000);              // waits two seconds
/// msg.SetNoteOff(0, 60);              // makes msg the corresponding Note Off
/// port->OutputMessage(&msg);          // outputs the message (the note should stop)
/// port->ClosePort();                  // closes the port
/// \endcode
///
/// The MIDITimedMessage class inherits from MIDIMessage and adds the ability to associate a MIDIClockTime to the
/// message, so MIDITimedMessage objects can be ordered by time and queued into a MIDI track. Time is counted
/// in MIDI ticks (the MIDIClockTime type is a typedef for unsigned long) and the class has SetTime(), GetTime(),
/// AddTime() and SubTime() methods for dealing with it. A newly created MIDITimedMessage has its time set to 0.
///
/// The %MIDITrack class
/// --------------------
///
/// The MIDITrack is basically a stl vector of MIDITimedMessage objects, ordered by time. It has methods for editing the track
/// adding, retrieving and deleting messages. Due to the SMF format a MIDITrack always contains almost a message, the system
/// End of Data (or End of Track, EOT) message as last message. This message is automatically handled by the library, and the
/// user cannot insert or delete it; every time you insert a MIDITimedMessage into the track the MIDIClockTime of the EOT is
/// examined and eventually updated.
///
/// The constructor creates an empty track, with only the EOT; you can then edit it inserting or deleting MIDITimedMessage
/// objects or entire time intervals. When you insert a message of the same type of an old message at the same time you
/// can control if the new message will replace the old (see the
///
/// \code
/// #include "track.h"
/// #include "dump_tracks.h"            // contains helper functions to print track content
///
/// int main() {
///    MIDITrack track;
///    MIDITimedMessage msg;            // a new MIDITimedMessage has time set to 0
///    msg.SetProgramChange(0, 49);     // msg becomes a Program Change, channel 1, program 49, time 0
///    track.InsertEvent(msg);          // inserts the event into the track
///    msg.SetVolumeChange(0, 127);     // msg becomes a Volume Change (CC 7), channel 1, volume 127, time 0
///    track.InsertEvent(msg);
///    msg.SetNoteOn(0, 60, 100);       // msg becomes a Note On, channel 1, note 60, velocity 100
///    msg3.SetTime(480);               // sets the time of msg to 480 MIDI ticks
///    track.InsertNote(msg, 240);      // inserts the Note On and the corresponding Note Off after 240 ticks
///    DumpMIDITrack(&track);           // prints the contents of the track
/// }
/// \endcode
///
/// The %MIDIMultiTrack class
/// -------------------------
///
/// The MIDIMultiTrack is an array of MIDITrack objects to be played simultaneously. Typically track 0 is the master track,
/// containing meta events (time and key signature, tempo ...) and other system messages, while other tracks are channel
/// tracks, every one containing events with the same channel number. The constructor creates an empty multitrack with no
/// tracks: you can the add, delete or move tracks. Alternatively you can fill a MIDIMultitrack with the LoadMIDIFile()
/// function (declared in filereadmultitrack.h), which loads the contents of a SMF into the multitrack creating the
/// needed tracks.
///
/// The MIDIMultiTrack has no playing capacity, it must be embedded into a MIDITickComponent derived class (as MIDISequencer
/// or AdvancedSequencer) which picks up its content and sends it to the MIDI ports with the appropriate timing. Here is an
/// example:
/// \code
/// #include "multitrack.h"
/// #include "dump_tracks.h"            // contains helper functions to print track content
///
/// int main() {
///    MIDIMultiTrack multi;
///    LoadMIDIFile("twinkle.mid", &multi);
///    DumpAllTracks(&multi);
///    MIDITrack track(*multi.GetTrack(1));
///    for (unsigned int i = 0; i < track.GetNumEvents(); i++) {
///       MIDITimedMessage msg = track.GetEvent(i);
///       if (msg.IsNote())
///       msg.SetNote(msg.GetNote() + 12):
///    }
///    multi.InsertTrack(track);
///    DumpAllTracks(&multi);
/// }
/// \endcode

/// \page HOW_PLAYS How mjdkmidi plays MIDI
/// For playing the contents of a MIDIMultiTrack we must send them to an hardware MIDI port with accurate timing. This
/// is done by mean of some library classes.
///
/// The %MIDITimer class
/// --------------------
///
/// The MIDITimer is a static class which can start a background thread and call the callback MIDITimer::ThreadProc() at a
/// regular pace. The timing is supplied by the <std::chrono> classes and so you must compile the library according to the
/// c++ 0x11 standard. The default time resolution is 10 msecs but you can change it with the MIDITimer::SetResolution()
/// method.
///
/// When playing MIDI the timer is usually managed by the MIDIManager class, so you don't need to deal with it directly.
///
/// The %MIDIOutDriver and %MIDIInDriver classes
/// --------------------------------------------
///
/// These communicate between the library software and the hardware MIDI ports regardless the underlying OS. mjdkmid uses
/// the RTMidi library of Gary Scavone
///
/// The %MIDIManager class
/// ---------------------
///
/// The MIDIManager is a static class which handles the communications between the software and the MIDI hardware ports and
/// the general timing. At the start of the program it enumerates all hardware ports, then creates a MIDIOutDriver for every
/// out port, and a MIDIInDriver for every in port. Moreover it takes care of the starting and stopping of the (also static)
/// MIDITimer


/// \page MIDIENUM The MIDI enumerations
/// \include midi.h



























































































/// \defgroup INTERNALS Internals
/// Classes and functions used internally, the knowledge of which may be omitted by the end user.

/// \defgroup GLOBALS   Globals
/// Global functions, typedef or preprocessor labels.


#endif // DOXYGEN_H_INCLUDED
