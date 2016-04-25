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


// MANTENUTA LA VECCHIA INTERFACCIA PRE jdksmidi: num_tracks da il numero di traccie effettivamente usate
// le tracce vanno aggiunte con AddTrack()
// Aggiunte però le funzioni di editing di jdksmidi. Le funzioni di copia e incolla sono nuove ed esistono
// solo qui

#ifndef MIDI_MULTITRACK_H
#define MIDI_MULTITRACK_H

#include "world.h"
#include "track.h"


class MIDIEditMultiTrack;


class MIDIMultiTrack {
// due to the need of mantaining interface for other jdk files, this behaves differently from
// INTMultitrack.
// Constructor makes an empty MIDIMultiTrack so we must add master track with AddTrack().

    public:

                                MIDIMultiTrack(int cl_p_b = DEFAULT_CLKS_PER_BEAT,
                                              int max_tracks = DEFAULT_MAX_NUM_TRACKS);
        virtual	               ~MIDIMultiTrack();


        void                    SetClksPerBeat(int c_p_b)       { clks_per_beat = c_p_b; }
                                        // actually this is used only when a multitrack is loaded from a file
        MIDITrack*              GetTrack( int trk )             { return tracks[trk]; }
        const MIDITrack*        GetTrack( int trk ) const       { return tracks[trk]; }
        int	                    GetNumTracks()					{ return num_tracks; }
        int                     GetMaxNumTracks()               { return max_num_tracks; }
        int                     GetClksPerBeat()                { return clks_per_beat; }

        void                    Clear();                // deletes all tracks
        void                    ClearTracks(bool mantain_end = false);
                                        // clear track events but masntains tracks

        bool                    InsertTrack(int trk = -1);
                                        // if trk == -1 append track
        bool                    DeleteTrack(int trk);
        bool                    MoveTracks(int from, int to);
        bool                    InsertEvent(const MIDITimedBigMessage& msg, int trk);
        bool                    DeleteEvent(const MIDITimedBigMessage& msg, int trk);

        void                    EditCopy(MIDIClockTime start, MIDIClockTime end, int tr_start,
                                             int tr_end, MIDIEditMultiTrack* edit);
                                        // makes a new interval in *edit, with tracks from tr_start to tr_end
        void                    EditCut(MIDIClockTime start, MIDIClockTime end, MIDIEditMultiTrack* edit);
                                        // deletes events and shifts subsequents (only on entire multitrack)
        void                    EditClear(MIDIClockTime start, MIDIClockTime end, int tr_start, int tr_end);
                                        // erase events in tracks tr_start ... tr_end
        void                    EditInsert(MIDIClockTime start, int tr_start, int times,
                                           bool sysex, MIDIEditMultiTrack* edit);
                                        // insert interval <edit> in <time_start>
                                        // (if <edit> == 0 insert a blank interval)
        void                    EditReplace(MIDIClockTime start, int tr_start, int times,
                                            bool sysex, MIDIEditMultiTrack* edit);

                /// V. Magdazin functions (temporary)


        bool                    CreateObject ( int num_tracks_, bool deletable_ );
        bool                    ClearAndResize ( int num_tracks_ );
        bool                    AssignEventsToTracks ( const MIDITrack *src );
        int                     GetNumTracksWithEvents() const;

    private:

        int 	                clks_per_beat;      // not a const! When loading a multitrack it may change
        const int               max_num_tracks;
        int                     num_tracks;
        MIDITrack**             tracks;
        bool                    deletable;
};


class MIDIMultiTrackIteratorState {
    friend class MIDIMultiTrackIterator;

    public:
                                MIDIMultiTrackIteratorState(int n_tracks);
                                MIDIMultiTrackIteratorState(const MIDIMultiTrackIteratorState &m);
        virtual                ~MIDIMultiTrackIteratorState();

        const MIDIMultiTrackIteratorState& operator=(const MIDIMultiTrackIteratorState &m);
        int	                    GetNumTracks()		                    { return num_tracks; }
        int	                    GetCurEventTrack()						{ return cur_event_track; }
        MIDIClockTime           GetCurTime()				    	    { return cur_time; }
        void                    SetNumTracks(int n)                     { num_tracks = n; }
        void                    Reset();
        int                     FindTrackOfFirstEvent();

    private:
        int                     num_tracks;
        MIDIClockTime           cur_time;
        int                     cur_event_track;
        int*                    next_event_number;
        MIDIClockTime*          next_event_time;
};


class MIDIMultiTrackIterator {
    public:

                                MIDIMultiTrackIterator( MIDIMultiTrack *mlt );
        virtual                ~MIDIMultiTrackIterator();

        void                    Reset();    // syncs num_tracks with multitrack and goes to 0
        void                    GoToTime(MIDIClockTime time);
        bool                    GetCurEventTime(MIDIClockTime *t);
        bool                    GetCurEvent(int *track, MIDITimedBigMessage **msg);
        bool                    GoToNextEvent();
        bool                    GoToNextEventOnTrack(int track);
        MIDIMultiTrackIteratorState& GetState()                     { return state; }
        const MIDIMultiTrackIteratorState &GetState() const         { return state; }
        void                    SetState(const MIDIMultiTrackIteratorState& s) { state = s; }
        MIDIMultiTrack*         GetMultiTrack()  				    { return multitrack; }
        const MIDIMultiTrack*   GetMultiTrack() const 	            { return multitrack; }

    protected:
        MIDIMultiTrack*         multitrack;
        MIDIMultiTrackIteratorState state;
};


class MIDIEditMultiTrack : public MIDIMultiTrack {
// WARNING: this is different from INTEditSong, because it inherits directly from MIDIMultiTrack (see header IntSong.h)

    public:
                                MIDIEditMultiTrack(int cl_p_b = DEFAULT_CLKS_PER_BEAT,
                                              int max_tracks = DEFAULT_MAX_NUM_TRACKS) :
                                    MIDIMultiTrack(cl_p_b, max_tracks), start_track(0), end_track(0) {}
        virtual	               ~MIDIEditMultiTrack() {}

        void                    SetStartTrack(int trk)      { start_track = trk; }
        void                    SetEndTrack(int trk)        { end_track = trk; }
        int                     GetStartTrack() const       { return start_track; }
        int                     GetEndTrack() const         { return end_track; }

                    // these must be disabled in derived class: so they are declared but not implemented
        void                    EditCopy(MIDIClockTime start, MIDIClockTime end, int tr_start,
                                             int tr_end, MIDIEditMultiTrack* edit) {}
        void                    EditCut(MIDIClockTime start, MIDIClockTime end, MIDIEditMultiTrack* edit) {}
        void                    EditClear(MIDIClockTime start, MIDIClockTime end, int tr_start, int tr_end) {}
        void                    EditInsert(MIDIClockTime start, int tr_start, int times,
                                           bool sysex, MIDIEditMultiTrack* edit) {}
        void                    EditReplace(MIDIClockTime start, int tr_start, int times,
                                            bool sysex, MIDIEditMultiTrack* edit) {}
                    ///////////////////


        //void                    CopyTrack(int n, MIDIMultiTrack* trk);
                                // use operator= instead!
        void                    CopyAll(MIDIMultiTrack* m);
                                // does a Clear and copy

    private:

        int 	                start_track;
        int                     end_track;


};


#endif


