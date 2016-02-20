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


class MIDISequencerTrackState;
class MIDISequencer;


class MIDISequencerTrackProcessor : public MIDIProcessor {
    public:
                        MIDISequencerTrackProcessor();
        virtual         ~MIDISequencerTrackProcessor() {}

        virtual void    Reset();
        virtual bool    Process( MIDITimedBigMessage *msg );
        bool            mute;				// track is muted
        bool            solo;				// track is solod
        int             velocity_scale;		// current velocity scale value for note ons, 100=normal
        int             rechannel;			// rechannelization value, or -1 for none
        int             transpose;			// amount to transpose note values
        MIDIProcessor   *extra_proc;	    // extra midi processing for this track
};


class MIDISequencerTrackState : public MIDISequencerTrackNotifier {
    public:

                        MIDISequencerTrackState( MIDISequencer *seq_, int trk, MIDISequencerGUIEventNotifier *n );
        virtual         ~MIDISequencerTrackState() {}

        virtual void    GoToZero();
        virtual void    Reset();
        virtual bool    Process( MIDITimedBigMessage *msg );


        char            pg;					// current program change, or -1
        char            volume;				// current volume controller value
        char            pan;				// current pan controller value
        char            rev;                // current reverber controller value
        char            chr;                // current chorus controller value

        int             bender_value;		// last seen bender value
        char            track_name[256];	// track name
        bool            got_good_track_name;// true if we dont want to use generic text events for track name
        bool            notes_are_on;		// true if there are any notes currently on
        MIDIMatrix      note_matrix;		// to keep track of all notes on
};


class MIDISequencerState : public MIDISequencerTrackNotifier {
    public:
                                MIDISequencerState(  MIDISequencer *s,
                                             MIDIMultiTrack *multitrack_,
                                             MIDISequencerGUIEventNotifier *n );

                                MIDISequencerState( const MIDISequencerState &s );
                                ~MIDISequencerState();

        const MIDISequencerState& operator= ( const MIDISequencerState &s );

        void                    Reset();             // new : added by me
        bool                    Process( MIDITimedBigMessage* msg );
        bool                    Process( MIDITimedBigMessage* msg, int trk);
        int                     GetNumTracks() const        { return multitrack->GetNumTracks(); }

        MIDISequencerGUIEventNotifier *notifier;
        MIDIMultiTrack*         multitrack;
        int                     last_event_track;

        MIDISequencerTrackState*track_state[DEFAULT_MAX_NUM_TRACKS];
        MIDIMultiTrackIterator  iterator;

        MIDIClockTime           cur_clock;
        float                   cur_time_ms;
        int                     cur_beat;
        int                     cur_measure;
        MIDIClockTime           next_beat_time;

        float                   tempobpm;           // current tempo in beats per minute
        char                    timesig_numerator;  // numerator of current time signature
        char                    timesig_denominator;// denominator of current time signature
        char                    keysig_sharpflat;   // current key signature accidents
        char                    keysig_mode;        // major/minor
        char                    markertext[50];     // current marker
};


class MIDISequencer {
    public:
                                        MIDISequencer(
                                            MIDIMultiTrack *m,
                                            MIDISequencerGUIEventNotifier *n=0
                                        );
        virtual                         ~MIDISequencer();

        void                            ResetTrack( int trk );
        void                            ResetAllTracks();

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

        MIDISequencerTrackState*        GetTrackState( int trk ){ return state.track_state[trk]; }
        const MIDISequencerTrackState*  GetTrackState( int trk ) const
                                                                { return state.track_state[trk]; }
        MIDISequencerTrackProcessor*    GetTrackProcessor( int trk )
                                                                { return track_processors[trk]; }
        const MIDISequencerTrackProcessor* GetTrackProcessor( int trk ) const
                                                                { return track_processors[trk]; }
        int                             GetNumTracks() const	{ return state.GetNumTracks(); }
        bool                            GetSoloMode() const     { return solo_mode; }

        void                            SetState( MIDISequencerState* s)
                                                                { state = *s; }
        void                            SetCurrentTempoScale( float scale )
                                                                { tempo_scale = (int)(scale*100); }
        void                            SetSoloMode( bool m, int trk=-1 );

        void                            GoToZero();
        bool                            GoToTime( MIDIClockTime time_clk );
        bool                            GoToTimeMs( float time_ms );
        bool                            GoToMeasure( int measure, int beat=0 );
        bool                            GetNextEventTimeMs( float *t );
        bool                            GetNextEventTime( MIDIClockTime *t );
        bool                            GetNextEvent( int *tracknum, MIDITimedBigMessage *msg );

        void                            ScanEventsAtThisTime();
        float                           MIDItoMs(MIDIClockTime t);  // new : added by me

    protected:

        MIDITimedBigMessage             beat_marker_msg;
        bool                            solo_mode;
        int                             tempo_scale;
        MIDISequencerTrackProcessor*    track_processors[DEFAULT_MAX_NUM_TRACKS];
        MIDISequencerState              state;
};


#endif
