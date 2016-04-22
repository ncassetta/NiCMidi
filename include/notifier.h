#ifndef _JDKMIDI_NOTIFIER_H
#define _JDKMIDI_NOTIFIER_H


#include <cstdio>
#include "process.h"


//#include <mss121/mss.h>

class MIDISequencer;


class MIDISequencerGUIEvent {
    public:
                    MIDISequencerGUIEvent()                 { bits=0; }
                    MIDISequencerGUIEvent(unsigned long bits_) : bits(bits_) {}
                    MIDISequencerGUIEvent( const MIDISequencerGUIEvent &e ) : bits(e.bits) {}
                    MIDISequencerGUIEvent( int group, int subgroup, int item=0 ) {
                        bits = ((group&0xff)<<24) | ((subgroup&0xfff)<<12) | (item&0xfff); }
                    operator unsigned long () const         { return bits; }
        void        SetEvent( int group, int subgroup = 0, int item = 0 ) {
                        bits = ((group&0xff)<<24) | ((subgroup&0xfff)<<12) | (item&0xfff); }
        int         GetEventGroup() const                   { return (int)((bits>>24)&0xff); }
        int         GetEventSubGroup() const                { return (int)((bits>>12)&0xfff); }
        int         GetEventItem() const                    { return (int)((bits>>0)&0xfff); }

            // main groups
        enum {
            GROUP_ALL = 0,
            GROUP_CONDUCTOR,
            GROUP_TRANSPORT,
            GROUP_TRACK
        };

            // items in conductor group
        enum {
            GROUP_CONDUCTOR_ALL = 0,
            GROUP_CONDUCTOR_TEMPO,
            GROUP_CONDUCTOR_TIMESIG,
            GROUP_CONDUCTOR_KEYSIG,
            GROUP_CONDUCTOR_MARKER
        };

            // items in transport group
        enum {
            GROUP_TRANSPORT_ALL = 0,
            GROUP_TRANSPORT_MODE,
            GROUP_TRANSPORT_MEASURE,
            GROUP_TRANSPORT_BEAT,
            GROUP_TRANSPORT_ENDOFSONG
        };

            // items in TRACK group
        enum {
            GROUP_TRACK_ALL = 0,
            GROUP_TRACK_NAME,
            GROUP_TRACK_PG,
            GROUP_TRACK_NOTE,
            GROUP_TRACK_VOLUME
        };

    private:
        unsigned long bits;
};


class MIDISequencerGUIEventNotifier {
    public:
                        MIDISequencerGUIEventNotifier()     {}
        virtual         ~MIDISequencerGUIEventNotifier()    {}
        virtual void    Notify( const MIDISequencer *seq, MIDISequencerGUIEvent e ) = 0;
        virtual bool    GetEnable() const = 0;
        virtual void    SetEnable( bool f ) = 0;
};


class MIDISequencerGUIEventNotifierText : public MIDISequencerGUIEventNotifier {
    public:
                        MIDISequencerGUIEventNotifierText(FILE *f_) : f(f_), en(true) {}
        virtual         ~MIDISequencerGUIEventNotifierText()                          {}
        virtual void    Notify( const MIDISequencer *seq, MIDISequencerGUIEvent e );
        virtual bool    GetEnable() const               { return en; }
        virtual void    SetEnable( bool f )             { en = f; }

    private:
        FILE *f;
        bool en;
};


class MIDISequencerTrackNotifier : public MIDIProcessor {
    public:
                        MIDISequencerTrackNotifier(MIDISequencer*  seq_, int trk,
                            MIDISequencerGUIEventNotifier *n)  :
                            seq(seq_), track_num(trk), notifier(n) {}

        virtual         ~MIDISequencerTrackNotifier()   {}

        void            SetNotifier(MIDISequencer *seq_, int trk,
                            MIDISequencerGUIEventNotifier *n ) {
                                seq = seq_; track_num = trk; notifier = n; }

        void            Notify( int item );
        void            NotifyConductor( int item );

    protected:
        MIDISequencer   *seq;
        int             track_num;
        MIDISequencerGUIEventNotifier *notifier;
};


#ifdef _WIN32
#include "windows.h"
#include "mmsystem.h"


class MIDISequencerGUIEventNotifierWin32 : public MIDISequencerGUIEventNotifier {
public:

                            MIDISequencerGUIEventNotifierWin32 ( HWND w, DWORD wmmsg, WPARAM wparam_value_ = 0 );

        // Auto sets the Windows message id and wparam_value
                            MIDISequencerGUIEventNotifierWin32 ( HWND w );

    virtual                 ~MIDISequencerGUIEventNotifierWin32() {}

        // Gets the Window message id (so you can retrieve it if you used the 2nd constructor)
    DWORD                    GetMsgId() const            { return window_msg; }

        // Sends the MIDISequencerGUIEvent _e_ to the window
    virtual void             Notify ( const MIDISequencer *seq, MIDISequencerGUIEvent e );

        // Returns the on/off status
    virtual bool             GetEnable() const           { return en; }

        // Sets the on/off status
    virtual void             SetEnable ( bool f )        { en = f; }

private:

        // Returns a safe Windows message id, so we can create the notifier without worrying about this
    static UINT              GetSafeSystemMsgId()        { static UINT base = WM_APP; return base++; }

    HWND                     dest_window;
    DWORD                    window_msg;
    WPARAM                   wparam_value;
    bool                     en;
};

#endif // _WIN32




#endif // _JDKMIDI_NOTIFIER_H
