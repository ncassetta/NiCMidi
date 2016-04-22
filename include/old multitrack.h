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

#ifndef MIDI_MULTITRACK_H
#define MIDI_MULTITRACK_H

#include "track.h"



class MIDIMultiTrack {
// due to the need of mantaining interface for other jdk files, this is leaved VERY INCOHOERENT
// with the rest of classes!!!
// I use it with deletable_ = false so it doesn't alloc tracks (i.e. tracks[i] are initially all 0)
// and add and delete tracks with AddTrack() and DelTrack()
    public:

                                MIDIMultiTrack( int max_num_tracks_ = DEFAULT_MAX_NUM_TRACKS,
                                                bool deletable_ = true );
                                virtual	~MIDIMultiTrack();

        void                    SetTrack( int trk, MIDITrack *t )       { tracks[trk]=t; }
        MIDITrack               *GetTrack( int trk )                    { return tracks[trk]; }
        const MIDITrack         *GetTrack( int trk ) const              { return tracks[trk]; }
        int	                    GetNumTracks()							{ return num_tracks; }
                // THIS IS REALLY MAX_NUM_TRACKS not used tracks
        int                     GetNumUsedTracks()                      { return num_used_tracks; }
        int                     GetClksPerBeat()                        { return clks_per_beat; }
        void                    SetClksPerBeat(int c)                   { clks_per_beat = c; }
        void                    Clear();

        bool                    InsertTrack(int trk = -1);
                                                        // if trk == -1 append track
        bool                    DeleteTrack(int trk);
        bool                    InsertEvent(const MIDITimedBigMessage& msg, int trk);
        bool                    DeleteEvent(const MIDITimedBigMessage& msg, int trk);

        static const int        DEFAULT_MAX_NUM_TRACKS  = 64;
        static const int        DEFAULT_CLKS_PER_BEAT = 480;
                                                    // ADDED BY ME! ctor sets clks_per_beat to this so
                                                    // multitrack is in a coherent state
    protected:
        MIDITrack**             tracks;
        const int               num_tracks;         // THIS IS REALLY THE NUM OF ALLOC TRACKS
        int                     num_used_tracks;    // ADDED BY ME! This is the real number of tracks
        bool	                deletable;
        int 	                clks_per_beat;

    private:
};


class MIDIMultiTrackIteratorState {
    friend class MIDIMultiTrackIterator;

    public:
                                MIDIMultiTrackIteratorState(int num_tracks_=64);
                                MIDIMultiTrackIteratorState(const MIDIMultiTrackIteratorState &m);
        virtual                 ~MIDIMultiTrackIteratorState();

        const MIDIMultiTrackIteratorState& operator=(const MIDIMultiTrackIteratorState &m);
        int	                    GetNumTracks()		                    { return num_tracks; }
        int	                    GetCurEventTrack()						{ return cur_event_track; }
        MIDIClockTime           GetCurTime()				    	    { return cur_time; }
        void                    Reset();
        int                     FindTrackOfFirstEvent();

    private:
        MIDIClockTime           cur_time;
        int                     cur_event_track;
        int                     num_tracks;
        int*                    next_event_number;
        MIDIClockTime*          next_event_time;
};


class MIDIMultiTrackIterator {
    public:

                                MIDIMultiTrackIterator( MIDIMultiTrack *mlt );
        virtual                 ~MIDIMultiTrackIterator();

        void                    GoToTime(MIDIClockTime time);
        bool                    GetCurEventTime(MIDIClockTime *t);
        bool                    GetCurEvent(int *track, MIDITimedBigMessage **msg);
        bool                    GoToNextEvent();
        bool                    GoToNextEventOnTrack(int track);
        const MIDIMultiTrackIteratorState &GetState() const         { return state; }
        MIDIMultiTrackIteratorState& GetState()                     { return state; }
        void                    SetState(const MIDIMultiTrackIteratorState& s) { state = s; }
        MIDIMultiTrack*         GetMultiTrack()  				    { return multitrack; }
        const MIDIMultiTrack*   GetMultiTrack() const 	            { return multitrack; }

    protected:
        MIDIMultiTrack*         multitrack;
        MIDIMultiTrackIteratorState state;
};


#endif

/* ********




class INTMultiTrack {
    public:

                                INTMultiTrack(int cl_p_b = DEFAULT_CLKS_PER_BEAT,
                                              int max_tracks = DEFAULT_MAX_NUM_TRACKS);
        //virtual	                ~INTMultiTrack();

        INTTrack*               GetMainTrack()              { return tracks[0]; }
        const INTTrack*         GetMainTrack() const        { return tracks[0]; }
        //INTTrack*               GetTrack(int trk)           { return tracks[trk]; }
        //const INTTrack*         GetTrack(int trk) const     { return tracks[trk]; }
        //int	                    GetNumTracks() const  	    { return num_tracks; }
        int                     GetMaxNumTracks() const     { return max_num_tracks; }
        //int                     GetClksPerBeat() const      { return clks_per_beat; }
        MIDIClockTime           GetEndTime() const          { return tracks[0]->GetEndTime(); }

        void                    Clear();                // deletes all tracks and clear maintrack
        void                    ClearTracks();          // clears all tracks (but mantains them)
        void                    ClearTrack(int trk);    // clears tracks[trk].
        bool                    InsertTrack(const INTTrackHeader& h, int trk = -1);
                                                        // if trk == -1 append track
        bool                    DeleteTrack(int trk);

        //bool                    InsertEvent(const INTTimedMessage& msg, int trk);
        //bool                    DeleteEvent(const INTTimedMessage& msg, int trk);

        void                    InsertInterval(MIDIClockTime start, MIDIClockTime interval, const INTMultiTrack* src = 0);
                                                    // if interval == 0 shift events of interval clocks
        void	                Merge(const INTMultiTrack *src);
        INTMultiTrack&          MakeInterval(MIDIClockTime start, MIDIClockTime end, INTMultiTrack& interval) const;
        void                    DeleteInterval(MIDIClockTime start, MIDIClockTime end);
                                                    // deletes events and shifts subsequents
        void                    ClearInterval(MIDIClockTime start, MIDIClockTime end);


        bool                    Load(std::ifstream& ifs);
        bool                    Save(std::ofstream& ofs);

    private :

        const int 	            clks_per_beat;
        const int               max_num_tracks;
        int                     num_tracks;
        INTTrack**              tracks;
};


*/



