#ifndef _JDKMIDI_NOTIFIER_H
#define _JDKMIDI_NOTIFIER_H


#include <iostream>
#include "processor.h"



class MIDISequencer;


///
/// This class holds data for a message that the sequencer can send to the GUI to warn it when
/// something is happened.
/// A MIDISequencerGUIEvent belongs to one of these four groups:
/// - GROUP_ALL: generic group, used by the sequencer to request the GUI for a general refresh
/// - GROUP_CONDUCTOR: events as tempo, time, key change ...
/// - GROUP_TRANSPORT: start, stop ...
/// - GROUP_TRACK: note, program, control change ...
/// Every group has some items, denoting different events
/// For GROUP_TRACK is also used a subgroup parameter, i.e. the track of the event.
/// For effective sending messages see the MIDISequencerGUINotifier class.
///
class MIDISequencerGUIEvent {
    public:
            /// Default constructor: creates a generic event with all attributes set to 0
                    MIDISequencerGUIEvent()                     { bits = 0; }
            /// This constructor creates the object directly from its parameters.
                    MIDISequencerGUIEvent(unsigned long bits_) : bits(bits_) {}
            /// Copy constructor.
                    MIDISequencerGUIEvent( const MIDISequencerGUIEvent &e ) : bits(e.bits) {}
            /// This constructor creates the object starting from its group, subgroup, item
                    MIDISequencerGUIEvent( int group, int subgroup, int item=0 ) {
                        bits = ((group&0xff)<<24) | ((subgroup&0xfff)<<12) | (item&0xfff); }
            /// Converts the object into an unsigned long
                    operator unsigned long () const         { return bits; }
            /// Sets the event group, subgroup and item.
        void        SetEvent( int group, int subgroup = 0, int item = 0 ) {
                        bits = ((group&0xff)<<24) | ((subgroup&0xfff)<<12) | (item&0xfff); }
            /// Gets the event group.
        int         GetGroup() const                        { return (int)((bits>>24)&0xff); }
            /// Gets the event subgroup (only effective for GROUP_TRACK events, where it is
            /// the track of the event; it is 0 for other groups).
        int         GetSubGroup() const                     { return (int)((bits>>12)&0xfff); }
            /// Gets the event item (i.e. the kind of the event).
        int         GetItem() const                         { return (int)((bits>>0)&0xfff); }

            /// Main groups
        enum {
            GROUP_ALL = 0,              ///< Generic group: used by the MIDISequencer to request a full GUI reset
            GROUP_CONDUCTOR,            ///< Conductor group
            GROUP_TRANSPORT,            ///< Transport group
            GROUP_TRACK,                ///< Track group (the subgroup is the track of the event)
            GROUP_USER                  ///< User defined group
        };

            /// Items in conductor group
        enum {
            GROUP_CONDUCTOR_ALL = 0,    ///< Generic event (not currently used)
            GROUP_CONDUCTOR_TEMPO,      ///< Tempo change
            GROUP_CONDUCTOR_TIMESIG,    ///< Timesig change
            GROUP_CONDUCTOR_KEYSIG,     ///< Keysig change
            GROUP_CONDUCTOR_MARKER,     ///< Marker
            GROUP_CONDUCTOR_USER        ///< User defined item
    };

            /// Items in transport group
        enum {
            GROUP_TRANSPORT_ALL = 0,    ///< Generic event (not currently used)
            GROUP_TRANSPORT_MODE,       ///< Sequencer start and stop
            GROUP_TRANSPORT_MEASURE,    ///< Start of a measure
            GROUP_TRANSPORT_BEAT,       ///< Beat marker
            GROUP_TRANSPORT_ENDOFSONG,  ///< End of playback
            GROUP_TRANSPORT_USER        ///< User defined item
        };

            /// Items in track group (the track is in the subgroup)
        enum {
            GROUP_TRACK_ALL = 0,        ///< Generic event (not currently used)
            GROUP_TRACK_NAME,           ///< Track got its name
            GROUP_TRACK_PROGRAM,        ///< Program change
            GROUP_TRACK_NOTE,           ///< Note
            GROUP_TRACK_VOLUME,         ///< Volume change
            GROUP_TRACK_PAN,            ///< Pan change
            GROUP_TRACK_CHR,            ///< Chorus change
            GROUP_TRACK_REV,            ///< Reverb change
            GROUP_TRACK_USER            ///< User defined item
        };


        static const char group_names[][10];
        static const char conductor_items_names[][10];
        static const char transport_items_names[][10];
        static const char track_items_names[][10];

    private:
        unsigned long bits;
};


///
/// This class sends MIDISequencerGUIEvent messsages to the GUI.
/// The base class is pure virtual, because we need GUI details for really sending messages;
/// currently there are two implementations: a text notifier which prints messages on an ostream
/// (cout as default) and a WIN32 specific GUI notifier (see MIDISequencerGUINotifierText,
/// MIDISequencerGUINotifierWin32).
/// This class is embedded in many other classes that need to communicate with the GUI (see
/// MIDIManager, MIDISequencer, MIDISequencerTrackState, MIDISequencerState, AdvancedSequencer)
///
class MIDISequencerGUINotifier {
    public:
            /// The constructor. To actually send messages, the notifier must know the sequencer which
            /// sends them; you can set it in the constructor or later.
                        MIDISequencerGUINotifier(const MIDISequencer* seq = 0) :
                            sequencer(seq), en(true)                {}
            /// The destructor.
        virtual         ~MIDISequencerGUINotifier()                 {}
            /// This sets the sequencer which generates messages sent to the GUI.
            /// @warning Don't use this if you want to join the notifier with a sequencer
        virtual void    SetSequencer(const MIDISequencer* seq)      { sequencer = seq; }
            /// Notifies the MIDISequencerGUIEvent _e_, originated from the MIDISequencer _seq_.
        virtual void    Notify(MIDISequencerGUIEvent e) = 0;
            /// Returns the enable/disable status.
        virtual bool    GetEnable() const                           { return en;};
             /// Sets message sending on/off.
        virtual void    SetEnable(bool f)                           { en = f; }
    protected:
        const MIDISequencer*  sequencer;
        bool en;
};


///
/// This class inherits from the pure virtual MIDISequencerGUIEventNotifier, and notifies text messages
/// to an ostream object.
///
class MIDISequencerGUINotifierText : public MIDISequencerGUINotifier {
    public:
                        MIDISequencerGUINotifierText(const MIDISequencer* seq = 0, std::ostream& o = std::cout) :
                                MIDISequencerGUINotifier(seq), ost(o)   {}
        virtual         ~MIDISequencerGUINotifierText()                 {}
        virtual void    Notify(MIDISequencerGUIEvent e);

    private:
        std::ostream& ost;
};


#ifdef _WIN32
#include "windows.h"
#include "mmsystem.h"


class MIDISequencerGUINotifierWin32 : public MIDISequencerGUINotifier {
public:

                            MIDISequencerGUINotifierWin32 (HWND w, DWORD wmmsg, WPARAM wparam_value_ = 0);

        // Auto sets the Windows message id and wparam_value
                            MIDISequencerGUINotifierWin32 (HWND w);

    virtual                 ~MIDISequencerGUINotifierWin32() {}

        // Gets the Window message id (so you can retrieve it if you used the 2nd constructor)
    DWORD                    GetMsgId() const            { return window_msg; }

        // Sends the MIDISequencerGUIEvent _e_ to the window
    virtual void             Notify (MIDISequencerGUIEvent e);

 private:

        // Returns a safe Windows message id, so we can create the notifier without worrying about this
    static UINT              GetSafeSystemMsgId()        { static UINT base = WM_APP; return base++; }

    HWND                     dest_window;
    DWORD                    window_msg;
    WPARAM                   wparam_value;
};

#endif // _WIN32




#endif // _JDKMIDI_NOTIFIER_H
