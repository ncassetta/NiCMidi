/*
 * ADAPTED FROM
 *
 * libjdkmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  *** RELEASED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) April 27, 2004 ***
 *
 *
 * BY NICOLA CASSETTA
 *
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// TODO: da rivedere! Adattarlo a jdksmidi porterebbe a modifiche nell'interfaccia (AddTrack, GetNumTracks e
// forse altre


#ifndef _JDKMIDI_SEQUENCER_H
#define _JDKMIDI_SEQUENCER_H

#include "track.h"
#include "multitrack.h"
#include "tempo.h"
#include "matrix.h"
#include "process.h"
#include "notifier.h"

#include <string>


class MIDISequencer;

///
/// This class inherits from the pure virtual MIDIProcessor and is a multi-purpose processor
/// implementing muting, soloing, rechanneling, velocity scaling and transposing.
/// Moreover, you can set a custom MIDIProcessor pointer which extra-processes messages.
/// The MIDISequencer class contains an independent MIDISequencerTrackProcessor for every MIDI Track.
/// Advanced classes like MIDISequencer and AdvancedSequencer allow you to set muting, tramsposing,
/// etc. without dealing with it: the only useful function for the user is the extra processing hook.
/// However, you could subclass this for getting new features.
///
class MIDISequencerTrackProcessor : public MIDIProcessor {
    public:
        /// The constructor. Default is no processing (MIDI messages leave the processor unchanged)
                        MIDISequencerTrackProcessor();
        /// The destructor
        virtual         ~MIDISequencerTrackProcessor() {}
        /// Resets all values to default state
        virtual void    Reset();
        /// Processes message msg, changing its parameters according to the state of the processor
        virtual bool    Process ( MIDITimedBigMessage *msg );

        bool            mute;                   ///< track is muted
        bool            solo;                   ///< track is soloed
        int             velocity_scale;         ///< current velocity scale value for note ons, 100=normal
        int             rechannel;              ///< rechannelization value, or -1 for none
        int             transpose;              ///< amount to transpose note values
        MIDIProcessor   *extra_proc;            ///< extra midi processing for this track
};


///
/// This class stores current MIDI parameters for a track.
/// It stores track name, program, volume, pan, chorus, reverb, pitch bend and a matrix with notes on and off.
/// The MIDISequencerState class contains a MIDISequencerTrackState for every MIDI Track, and it take care of
/// updating parameters. You can ask the MIDISequencerTrackState for knowing actual track parameters, however
/// advanced class AdvancedSequencer allows you to get them without dealing with it, so the use of this class
/// is mainly internal. However, you could subclass it if you want to keep track of other parameters.
///
class MIDISequencerTrackState {
    public:
        /// The constructor.
        /// Initial attributes are program = -1, volume = 100, pan = 64, chorus = 0, reverb = 0, bender_value = 0,
        /// track_name = "", all notes off.
                        MIDISequencerTrackState();
        /// The destructor.
        virtual         ~MIDISequencerTrackState() {}
        /// Resets default values, but not track_name and got_good_track_name
        virtual void    GoToZero();
        /// As above, but resets also track_name and got_good_track_name
        virtual void    Reset();

        char            program;		    ///< current program change, or -1
        char            volume;				///< current volume controller value
        char            pan;				///< current pan controller value
        char            rev;                ///< current reverb controller value
        char            chr;                ///< current chorus controller value
        int             bender_value;		///< last seen bender value
        std::string     track_name;	        ///< track name
        bool            notes_are_on;       ///< true if there are notes currently on
        MIDIMatrix      note_matrix;        ///< to keep track of all notes on

        bool            got_good_track_name;// true if we dont want to use generic text events for track name
};


///
/// This class stores current MIDI general parameters for a MIDISequencer object.
/// It contains a MIDIMultiTrackIterator, allowing to set a 'now' time: when current time changes (because
/// the sequencer is playing or time is changed by the user) the object keep tracks of current timesig, keysig,
/// tempo(BPM), marker, measure and beat data. Moreover it contains an independent MIDISequencerTrackState
/// for every MIDI Track of the MIDISequencer, and you can examine them for knowing actual track parameters.
/// It inherits from the pure virtual MIDIProcessor: the MIDISequencer sends to it MIDI messages and it
/// processes them remembering actual parameters and notifying changes to the GUI.
/// All methods and attributes are public because they are used by MIDISequencer class; the advanced class
/// AdvancedSequencer allows you to know actual parameters without directly examining them, so the use of this
/// class is mainly internal.
/// However, you could subclass it if you want to keep track of other parameters.
///
class MIDISequencerState {
    public:
        /// The constructor. Appropriate values are set by the sequencer when it creates the object.
                                MIDISequencerState(MIDIMultiTrack *multitrack_,
                                                   MIDISequencerGUINotifier *n = 0);
        /// The copy constructor.
                                MIDISequencerState(const MIDISequencerState &s);
        /// The destructor does nothing.
                                ~MIDISequencerState() {}
        /// The equal operator.
        const MIDISequencerState& operator= (const MIDISequencerState &s);

        /// Returns the number of tracks of the multitrack.
        unsigned int            GetNumTracks() const        {return multitrack->GetNumTracks();}
        /// Resets the state to default values. These are: cur_clock = 0, tempo = 120 bpm,
        /// time = 4/4, keysig = C Maj, no marker. Moreover resets all track states (see
        /// MIDISequencerTrackState::Reset()).
        void                    Reset();
        /// This is the process function inherited from MIDIProcessor. When you get a MIDI message
        /// from the sequencer, it is processed by the state, which update its parameters and
        /// notifies the GUI. The user hadn't to worry about this.
        bool                    Process( MIDITimedBigMessage* msg );

        /// These are used internally for notifying the GUI when a something happens (a parameter
        /// was changed, time is moved, etc.)
        void                    Notify(int group, int item = 0) const;
        void                    NotifyTrack(int item) const;

        MIDISequencerGUINotifier *notifier; // TODO: these should be protected but they are used
        MIDIMultiTrack*         multitrack; // by AdvancedSequencer constructors
        MIDIMultiTrackIterator  iterator;

        MIDIClockTime           cur_clock;          ///< The current MIDI clock in MIDI ticks
        float                   cur_time_ms;        ///< The current clock in milliseconds
        int                     cur_beat;           ///< The current beat in the measure (1st beat is 0)
        int                     cur_measure;        ///< The current measure (1st measure is 0)
        MIDIClockTime           next_beat_time;     ///< The MIDI time of the next beat (for internal use)

        float                   tempobpm;           ///< The current tempo in beats per minute
        char                    timesig_numerator;  ///< The numerator of current time signature
        char                    timesig_denominator;///< The denominator of current time signature
        char                    keysig_sharpflat;   ///< The current key signature accidents (
        char                    keysig_mode;        ///< Major mode (0) or minor (1)
        std::string             marker_text;        ///< The current marker
        std::vector<MIDISequencerTrackState>        // TODO: should we have a vector of pointers?
                                track_states;

        int                     last_event_track;   ///< Internal use
};


///
/// This class implements a complete sequencer. This class holds:
/// - a MIDIMultiTrack for storing MIDI messages
/// - a MIDISequencerTrackProcessor for every track, allowing muting, soloing, transposing, ecc.
/// - a MIDIMultiTrackIterator, allowing to set a 'now' time, moving it along
/// - a MIDISequencerGUINotifier, that notifies the GUI about MIDI events
/// - a MIDISequencerState (which embeds the multitrack, the iterator and the notifier) to keep track
/// of actual parameters (tempo, keysig, track parameters, etc.)
/// \note This class has no playing capacity. For playing MIDI content you must use it together with a
/// MIDIManager. See the example files for effective using. AdvancedSequencer is an all-in-one class for
/// sequencing and playing
///
class MIDISequencer {
    public:
                                        MIDISequencer(
                                            MIDIMultiTrack *m,
                                            MIDISequencerGUINotifier *n=0
                                        );
        virtual                         ~MIDISequencer();

        //void                            ResetTrack(int trk);
        void                            Reset();

        MIDIClockTime                   GetCurrentMIDIClockTime() const
                                                                { return state.cur_clock; }
        double                          GetCurrentTimeInMs() const
                                                                { return state.cur_time_ms; }
        int                             GetCurrentBeat() const  { return state.cur_beat; }
        int                             GetCurrentMeasure() const
                                                                { return state.cur_measure; }
        double                          GetCurrentTempoScale() const
                                                                { return ((double)tempo_scale)*0.01; }
        double                          GetCurrentTempo() const { return state.tempobpm; }
        MIDISequencerState*             GetState()              { return &state; }
        const MIDISequencerState*       GetState() const        { return &state; }

        MIDISequencerTrackState*        GetTrackState(int trk)  { return &state.track_states[trk]; }
        const MIDISequencerTrackState*  GetTrackState(int trk) const
                                                                { return &state.track_states[trk]; }
        MIDISequencerTrackProcessor*    GetTrackProcessor(int trk)
                                                                { return track_processors[trk]; }
        const MIDISequencerTrackProcessor* GetTrackProcessor(int trk) const
                                                                { return track_processors[trk]; }
        unsigned int                    GetNumTracks() const	{ return state.GetNumTracks(); }
        bool                            GetSoloMode() const     { return solo_mode; }

        void                            SetState(MIDISequencerState* s)
                                                                { state = *s; }
        void                            SetCurrentTempoScale(double scale)
                                                                { tempo_scale = (int)(scale*100); }
        void                            SetSoloMode(bool m, int trk = -1);

        void                            GoToZero();
        bool                            GoToTime(MIDIClockTime time_clk);
        bool                            GoToTimeMs(double time_ms);
        bool                            GoToMeasure(int measure, int beat = 0);
        bool                            GetNextEventTime(MIDIClockTime *time_clk);
        bool                            GetNextEventTimeMs(double *time_ms);
        bool                            GetNextEvent( int *trk, MIDITimedBigMessage *msg );

        double                          MIDItoMs(MIDIClockTime time_clk);  // new : added by me

    protected:

        void                            ScanEventsAtThisTime();

        MIDITimedBigMessage             beat_marker_msg;
        bool                            solo_mode;
        int                             tempo_scale;
        std::vector<MIDISequencerTrackProcessor*>
                                        track_processors;
        MIDISequencerState              state;
};


#endif





/*
 *  libjdksmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  *** RELEASED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) April 27, 2004 ***
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
//
// Copyright (C) 2010 V.R.Madgazin
// www.vmgames.com vrm@vmgames.com
//

//
// MODIFIED by N. Cassetta  ncassetta@tiscali.it
// search /* NC */ for modifies
//

#ifndef JDKSMIDI_SEQUENCER_H
#define JDKSMIDI_SEQUENCER_H

//#include "jdk/track.h"
//#include "jdksmidi/multitrack.h"
//#include "jdksmidi/tempo.h"
//#include "jdksmidi/matrix.h"
//#include "jdksmidi/process.h"

namespace jdksmidi
{


/* NOTE BY NC: I eliminated this class because its name was somewhat confusing: it is a notifier,
   but inherits by a MIDIProcessor and CONTAINS a notifier.
   The class was only inherited by the MIDISequencerTrackState class and had no utility for the end user,
   so its features are incorporated in the latter

class MIDISequencerTrackNotifier : public MIDIProcessor
{
public:
    MIDISequencerTrackNotifier (
        const MIDISequencer *seq_,
        int trk,
        MIDISequencerGUIEventNotifier *n
    );

    virtual ~MIDISequencerTrackNotifier();

    void SetNotifier (
        const MIDISequencer *seq_,
        int trk,
        MIDISequencerGUIEventNotifier *n
    )
    {
        seq = seq_;
        track_num = trk;
        notifier = n;
    }


    void Notify ( int item );
    void NotifyConductor ( int item );      // NOTE by NC: this is now unneeded: could be eliminated

private:
    const MIDISequencer *seq;
    int track_num;
    MIDISequencerGUIEventNotifier *notifier;
};
*/





/*
class MIDISequencerTrackState : public MIDIProcessor
{
public:

    /// The constructor.
    /// Initial attributes are pg = -1, volume = 100, bender_value = 0, all notes off, track_name = ""
    /// \param seq_ the seguencer that sends messages (used only for notifying)
    /// \param trk the number of the track
    /// \param n the notifier: if set to 0 no notifying
    MIDISequencerTrackState (
        const MIDISequencer *seq_,
        int trk,
        MIDISequencerGUIEventNotifier *n
    );

    /// The destructor
    virtual ~MIDISequencerTrackState()
    {
    }

    /// Resets default values, but not track_name and got_good_track_name
    virtual void GoToZero();

    /// As above, but resets also track_name and got_good_track_name
    virtual void Reset();

    /// Processes the message msg, notifying the GUI if needed
    virtual bool Process ( MIDITimedBigMessage *msg );

    /// Notifies events to the GUI
    void Notify ( int item );

    int program;                    ///< current program change, or -1 if not set
    int volume;                     ///< current volume controller value
    int bender_value;               ///< last seen bender value
    char track_name[256];           ///< track name
    bool got_good_track_name;       ///< true if we dont want to use generic text events for track name

    bool notes_are_on;              ///< true if there are any notes currently on
    MIDIMatrix note_matrix;         ///< to keep track of all notes on

    const MIDISequencer* seq;       ///< the sequencer to which the track belongs
    int track;                      ///< the number of the track
    MIDISequencerGUIEventNotifier* notifier;        ///< the notifier: if 0 there is no notifying
};
*/


///
/// This class stores current MIDI general parameters.
/// It contains a MIDIMultiTrackIterator, allowing to set a 'now' time: when the current time changes (because
/// the sequencer is playing or it is moved by the user) the class stores current timesig, keysig, tempo(BPM),
/// marker, measure and beat data. Furthermore it inherits from the pure virtual MIDIProcessor:
/// the MIDISequencer sends to it MIDI messages and it processes them  remembering actual parameters and
/// notifying chamges to the GUI.
/// This class contains an independent MIDISequencerTrackState for every MIDI Track and you can
/// ask it for knowing actual parameters. However, advanced class AdvancedSequencer allows you to know them
/// without dealing with this, so the use of this class is mainly internal.
/// However, you could subclass it if you want to keep track of other parameters.
///

class MIDISequencerState : public MIDIProcessor
{
public:

    /// The constructor sets sequencer current time to 0. It allocates memory to hold a MIDISequencerTrackState
    /// for every track. Parameters are not owned.
    MIDISequencerState ( const MIDISequencer *s,
                         const MIDIMultiTrack *multitrack_,
                         MIDISequencerGUINotifier *n );

    /// The copy constructor
    MIDISequencerState ( const MIDISequencerState &s );

    /// The destructor frees allocated memory
    ~MIDISequencerState();

    /// The equal operator
    const MIDISequencerState & operator = ( const MIDISequencerState &s );

    /// Resets the state to current time = 0.
    void Reset();                                       /* NC */    // new

    /// Processes the message msg: if it is a channel message (or a track name meta) send it to a
    /// MIDISequencerTrackState, otherwise notify the GUI directly
    bool Process( MIDITimedBigMessage* msg );           /* NC */    // new

    /// Notifies events to GUI
    void Notify( int group, int item = 0 );             /* NC */    // new

    MIDISequencerGUINotifier *notifier;
    const MIDISequencer* seq;                           /* NC for notifying */
    const MIDIMultiTrack *multitrack;
    int num_tracks;                                 ///< nunber of tracks of the sequencer

    MIDISequencerTrackState *track_state[64];
    MIDIMultiTrackIterator iterator;
    MIDIClockTime cur_clock;                        ///< current time MIDI clock
    float cur_time_ms;                              ///< current time in ms
    int cur_beat;                                   ///< current beat
    int cur_measure;                                ///< current measure
    MIDIClockTime last_beat_time;   /* NC */        ///< used internally by Process()
    MIDIClockTime next_beat_time;                   ///< used internally by Process()
    float tempobpm;                                 ///< current tempo in beats per minute
    char timesig_numerator;                         ///< numerator of current time signature
    char timesig_denominator;                       ///< denominator of current time signature
    char keysig_sharpflat;                          ///< current key signature accidentals
    char keysig_mode;                               ///< major/minor mode
    char marker_name[40];                           ///< current marker name
    int last_event_track;                           ///< used internally by Process()
};


///
/// This class implements a complete sequencer. This class holds:
/// - a MIDIMultiTrack for storing MIDI messages
/// - a MIDISequencerTrackProcessor for every track, allowing muting, soloing, transposing, ecc.
/// - a MIDIMultiTrackIterator, allowing to set a 'now' time, moving it along
/// - a MIDISequencerGUIEventNotifier, that notifies the GUI about MIDI events
/// - a MIDISequencerState (which embeds the multitrack, the iterator and the notifier) to keep track
/// of actual parameters (tempo, keysig, track parameters, etc.)
/// \note This class has no playing capacity. For playing MIDI content you must use it together with a
/// MIDIManager. See the example files for effective using. AdvancedSequencer is an all-in-one class for
/// sequencing and playing
///

 class MIDISequencer
{
public:

    /// The constructor.
    /// \param m a pointer to a MIDIMultitrack that will hold MIDI messages
    /// \param n a pointer to a MIDISequencerGUIEventNotifier. If you leave 0 the sequencer will not notify
    /// the GUI.
    MIDISequencer (const MIDIMultiTrack *m, MIDISequencerGUINotifier *n = 0);

    /// The destructor frees allocated memory. The MIDIMultitrack and the MIDISequencerGUIEventNotifier are not
    /// owned by the MIDISequencer
    virtual ~MIDISequencer();

    /// Resets the corresponding MIDISequencerTrackState and MIDISequencerTrackProcessor. See
    /// MIDISequencerTrackState::Reset() and MIDISequencerTrackProcessor::Reset()
    void ResetTrack ( int trk );

    /// Call ResetTrack() for all tracks
    void ResetAllTracks();

    /// Returns current MIDIClockTime
    MIDIClockTime GetCurrentMIDIClockTime() const { return state.cur_clock; }

    /// Returns curremt time im milliseconds
    double GetCurrentTimeInMs() const { return state.cur_time_ms; }

    /// Returns current beat
    int GetCurrentBeat() const { return state.cur_beat;
    }

    /// Returns current measure
    int GetCurrentMeasure() const
    {
        return state.cur_measure;
    }

    /// Returns curremt tempo scale (1.00 = no scaling, 2.00 = twice faster, etc.)
    double GetCurrentTempoScale() const
    {
        return ( ( double ) tempo_scale ) * 0.01;
    }

    ///< Returns current tempo (BPM) without scaling ( actual tempo is GetCurrentTempo() * GetCurrentTempoScale() )
    double GetCurrentTempo() const
    {
        return state.tempobpm;
    }

    /// Returns current MIDISequencerState (i.e. the global sequencer state at current time). You can easily
    /// jump from a time to another saving and retrieving sequencer states.
    MIDISequencerState *GetState()
    {
        return &state;
    }

    const MIDISequencerState *GetState() const
    {
        return &state;
    }

    /// Returns the MIDISequencerTrackState for track trk
    MIDISequencerTrackState * GetTrackState ( int trk )
    {
        return state.track_state[trk];
    }

    const MIDISequencerTrackState * GetTrackState ( int trk ) const
    {
        return state.track_state[trk];
    }

    /// Returns the MIDISequencerTrackProcessor for track trk
    MIDISequencerTrackProcessor * GetTrackProcessor ( int trk )
    {
        return track_processors[trk];
    }

    const MIDISequencerTrackProcessor * GetTrackProcessor ( int trk ) const
    {
        return track_processors[trk];
    }

    /// Returns the number of tracks of the MIDIMultiTrack
    int GetNumTracks() const
    {
        return state.num_tracks;
    }

    /// Returns the solo mode on/off
    bool GetSoloMode() const
    {
        return solo_mode;
    }

    /// Copies the MIDISequencerState s into the internal sequencer state. You can easily
    /// jump from a time to another saving and retrieving sequencer states.
    void SetState ( MIDISequencerState *s )
    {
        state = *s;
    }

    /// Sets the tempo scale (1.00 = no scaling, 2.00 = twice faster, etc)
    void SetCurrentTempoScale ( float scale )
    {
        tempo_scale = ( int ) ( scale * 100 );
    }

    /// Soloes/unsoloes a track
    /// \param m on/off
    /// \param trk the nunber of the track if m is true, otherwhise you can leave default value
    void SetSoloMode ( bool m, int trk = -1 );

    /// Sets the 'now' time to the beginning of the song, upgrading the internal status.
    /// Notifies the GUI a GROUP_ALL notifier event to signify a GUI reset
    void GoToZero();

    /// Sets the 'now' time to the MIDI time time_clk, upgrading the internal status.
    /// Notifies the GUI a GROUP_ALL notifier event to signify a GUI reset
    /// \return _true_ if the time time_clk is effectively reached, _false_ otherwise (if time_clk is after
    /// the end of the song)
    bool GoToTime ( MIDIClockTime time_clk );

    /// Same as GoToTime(), but the time is given in milliseconds
    bool GoToTimeMs ( float time_ms );

    /// Sets the 'now' time to the given measure and beat, upgrading the internal status.
    /// \return see GoToTime()
    bool GoToMeasure ( int measure, int beat = 0 );


    bool GetNextEventTimeMs ( float *t );
    bool GetNextEventTimeMs ( double *t );

    /// Get the time of the next event (respect to the 'now' time) in MIDI ticks
    /// \return _true_ if there is a next event (and *t is a valid time) _false_ otherwise (*t is undefined)
    bool GetNextEventTime ( MIDIClockTime *t );

    /// Get the next event respect to the 'now' time.
    /// \param[out] tracknum: the track of the next event
    /// \param[out] msg the MIDI event
    /// \return _true_ if there is a next event (and the paraneters are valid), _false_ otherwise (parameters undefined)
    bool GetNextEvent ( int *tracknum, MIDITimedBigMessage *msg );



    /// Returns the total duration of the song (i.e.\ the time of last not end of track midi event)
    double GetMisicDurationInSeconds();

protected:

    /// Internal use: scans events at 'now' time upgrading the sequencer status
    void ScanEventsAtThisTime();

    MIDITimedBigMessage beat_marker_msg;

    bool solo_mode;
    int tempo_scale;

    int num_tracks;
    MIDISequencerTrackProcessor *track_processors[64];

    MIDISequencerState state;
} ;

}

#endif
