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


/// \file
/// Contains the definition of the classes MIDISequencerGUIEvent, MIDISequencerGUINotifier (abstract),
/// MIDISequencerGUINotifierText and  MIDISequencerGUINotifierWin32.


#ifndef _JDKMIDI_NOTIFIER_H
#define _JDKMIDI_NOTIFIER_H


#include <iostream>
#include "processor.h"



class MIDISequencer;


///
/// Holds data for a message that the sequencer can send to the GUI to warn it when something happens.
/// A MIDISequencerGUIEvent belongs to one of these four groups:
/// - GROUP_ALL: generic group, used by the sequencer to request the GUI for a general refresh
/// - GROUP_CONDUCTOR: events as tempo, time, key change ...
/// - GROUP_TRANSPORT: start, stop ...
/// - GROUP_TRACK: note, program, control change ...
///
/// Every group has some items, denoting different events, for GROUP_TRACK is also used a subgroup parameter, i.e.
/// the track of the event. Messages are sent by the MIDISequencerGUINotifier class.
///
class MIDISequencerGUIEvent {
    public:
        /// Default constructor: creates a generic event with all attributes set to 0
                    MIDISequencerGUIEvent()                     { bits = 0; }
        /// This constructor creates the object directly from its parameters, packed into an unsigned long.
                    MIDISequencerGUIEvent(unsigned long bits_) : bits(bits_) {}
        /// This constructor creates the object starting from its group, subgroup, item
                    MIDISequencerGUIEvent( int group, int subgroup, int item ) {
                        bits = ((group&0xff)<<24) | ((subgroup&0xfff)<<12) | (item&0xfff); }
                        // leave unchanged! overloading trouble, too many ctors
        /// Copy constructor.
                    MIDISequencerGUIEvent(const MIDISequencerGUIEvent &ev) : bits(ev.bits) {}
        /// Converts the object into an unsigned long
                    operator unsigned long () const             { return bits; }
        /// Returns the event group.
        int         GetGroup() const                        { return (int)((bits>>24)&0xff); }
        /// Returns the event subgroup (only effective for GROUP_TRACK events, where it is
        /// the track of the event; it is 0 for other groups).
        int         GetSubGroup() const                     { return (int)((bits>>12)&0xfff); }
        /// Returns the event item (i.e. the kind of the event).
        int         GetItem() const                         { return (int)((bits>>0)&0xfff); }
        /// Sets the event group, subgroup and item.
        void        SetEvent( int group, int subgroup = 0, int item = 0 ) {
                        bits = ((group&0xff)<<24) | ((subgroup&0xfff)<<12) | (item&0xfff); }

        /// Main groups
        enum {
            GROUP_ALL = 0,              ///< Generic group: used by the MIDISequencer to request a full GUI reset
            GROUP_CONDUCTOR,            ///< Conductor events (time, tempo, etc)
            GROUP_TRANSPORT,            ///< Transport events (start, stop, etc)
            GROUP_TRACK,                ///< Track events (the subgroup is the track of the event)
            GROUP_RECORDER,             ///< Recorder events
            GROUP_USER                  ///< User defined group
        };

        /// Items in conductor group
        enum {
            GROUP_CONDUCTOR_TEMPO = 0,  ///< Tempo change
            GROUP_CONDUCTOR_TIMESIG,    ///< Timesig change
            GROUP_CONDUCTOR_KEYSIG,     ///< Keysig change
            GROUP_CONDUCTOR_MARKER,     ///< Marker
            GROUP_CONDUCTOR_USER        ///< User defined item
        };

        /// Items in transport group
        enum {
            GROUP_TRANSPORT_START = 0,  ///< Sequencer start
            GROUP_TRANSPORT_STOP,       ///< Sequencer stop
            GROUP_TRANSPORT_MEASURE,    ///< Start of a measure
            GROUP_TRANSPORT_BEAT,       ///< Beat marker
            GROUP_TRANSPORT_COUNTIN,    ///< Countin start
            GROUP_TRANSPORT_USER        ///< User defined item
        };      // TODO: add a GROUP_TRANSPORT_ENDOFSONG for midi type 3 files?

        /// Items in track group (the track is in the subgroup)
        enum {
            GROUP_TRACK_NAME = 0,       ///< Track got its name
            GROUP_TRACK_PROGRAM,        ///< Program change
            GROUP_TRACK_NOTE,           ///< Note
            GROUP_TRACK_VOLUME,         ///< Volume change
            GROUP_TRACK_PAN,            ///< Pan change
            GROUP_TRACK_CHR,            ///< Chorus change
            GROUP_TRACK_REV,            ///< Reverb change
            GROUP_TRACK_USER            ///< User defined item
        };

        /// Items in recorder group
        enum {
            GROUP_RECORDER_RESET,       ///< Recorder reset
            GROUP_RECORDER_START,       ///< Recording start
            GROUP_RECORDER_STOP,        ///< Recording stop
            GROUP_RECORDER_USER         ///< User defined item
        };

        /// Item in user group
        enum {
            GROUP_USER_USER = 0         ///< User defined item
        };

        /// An array of strings with readable group names.
        static const char group_names[][10];
        /// An array of strings with readable conductor group items names.
        static const char conductor_items_names[][10];
        /// An array of strings with readable transport group items names.
        static const char transport_items_names[][10];
        /// An array of strings with readable track group items names.
        static const char track_items_names[][10];
        /// An array of strings with readable recording group items names.
        static const char recording_items_names[][10];
        /// An array of strings with readable user group item names.
        static const char user_items_names[][10];

    protected:
        /// \cond EXCLUDED
        unsigned long bits;         // Storage for group, subgroup and item
        /// \endcond
};


///
/// A pure virtual class implementing a device that can send MIDISequencerGUIEvent messages to a GUI.
/// It is pure virtual because we need GUI details for really sending messages; currently there are two
/// implementations: a text notifier which prints text messages to a std::ostream and a WIN32 specific
/// GUI notifier, (see MIDISequencerGUINotifierText, MIDISequencerGUINotifierWin32).
/// If we want to notify our GUI about events happening while the sequencer is playing we must create
/// an instance of the derived class and give its pointer to the MIDISequencer (or AdvancedSequencer)
/// constructor. When the sequencer is playing, its MIDISequencerState examines all MIDI messages output
/// by the sequencer and uses this to send appropriate MIDISequencerGUIEvent messages to the GUI.
///
class MIDISequencerGUINotifier {
    public:
        /// The constructor. To actually send messages, the notifier must know the sequencer which
        /// sends them; you can set it in the constructor or later.
                        MIDISequencerGUINotifier(const MIDISequencer* seq = 0) :
                            sequencer(seq), en(true)                {}
        // (no need for destructor)
        /// This sets the sequencer which generates messages sent to the GUI.
        /// \warning If you use the notifier in conjunction with an AdvancedSequencer class,
        /// this is automatically set by the class
        virtual void    SetSequencer(const MIDISequencer* seq)      { sequencer = seq; }
        /// Notifies the MIDISequencerGUIEvent _ev_.
        virtual void    Notify(const MIDISequencerGUIEvent &ev) = 0;
        /// Returns the enable/disable status.
        virtual bool    GetEnable() const                           { return en;};
        /// Sets message sending on/off.
        virtual void    SetEnable(bool f)                           { en = f; }
    protected:
        /// \cond EXCLUdED
        const MIDISequencer*  sequencer;            ///< The sequencer which will send the messages
        bool en;                                    ///< Enable/disable message sending
        /// \endcond
};


///
/// A MIDISequencerGUINotifier which sends text messages to a std::ostream (std::cout as default).
///
class MIDISequencerGUINotifierText : public MIDISequencerGUINotifier {
    public:
        /// The constructor.
        /// \param seq the sequencer from which messages originate
        /// \param os the std::ostream which will print the messages
                        MIDISequencerGUINotifierText(const MIDISequencer* seq = 0, std::ostream& os = std::cout) :
                                MIDISequencerGUINotifier(seq), start_from(0), ost(os)   {}

        /// Gets the numbering of measures and beats. See SetStartFromone().
        /// \return 0 or 1.
        char            GetStartFrom() const             { return start_from; }
        /// Sets the numbering of measures and beats (starting from 0 or from 1)
        /// \param f 0 or 1.
        /// \return **true** if the parameter is correct, **false** otherwise
        bool            SetStartFrom(char f);

        /// Notifies the event _ev_, printing to it a readable event description.
        virtual void    Notify(const MIDISequencerGUIEvent &ev);

    protected:
        /// \cond EXCLUDED
        char            start_from;
        std::ostream&   ost;
        /// \endcond
};


#ifdef _WIN32
#include "windows.h"
#include "mmsystem.h"


///
/// A MIDISequencerGUINotifier which sends messages to a Win32 window using the Windows PostMessage() function.
///
class MIDISequencerGUINotifierWin32 : public MIDISequencerGUINotifier {
    public:
            /// In this form of the constructor you must give the Windows parameters to the notifier.
            /// \param w the id of the window to whom sending messages
            /// \param msg a Windows message id used by the notifier to communicate with the window
            /// \param param_value an optional parameter sent by the message.
                            MIDISequencerGUINotifierWin32 (HWND w, DWORD msg, WPARAM param_value = 0);

            /// This form auto sets the Windows message id and wparam_value, so you don't have to worry
            /// about them.
                            MIDISequencerGUINotifierWin32 (HWND w);
            /// The destructor.
            virtual         ~MIDISequencerGUINotifierWin32() {}

            /// Returns the Window message id.
            DWORD           GetMsgId() const            { return window_msg; }

            /// Sends the MIDISequencerGUIEvent _ev_ to the window.
            virtual void    Notify (const MIDISequencerGUIEvent &ev);

    protected:
            /// \cond EXCLUDED
            // Returns a safe Windows message id, so we can create the notifier without worrying about this
            static UINT     GetSafeSystemMsgId()        { static UINT base = WM_APP; return base++; }

            HWND            dest_window;
            DWORD           window_msg;
            WPARAM          wparam_value;
            /// \endcond
};

#endif // _WIN32

#endif // _JDKMIDI_NOTIFIER_H
