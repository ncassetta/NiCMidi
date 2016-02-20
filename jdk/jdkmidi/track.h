/*  Adapted from
 *  libjdkmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  *** RELEASED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) April 27, 2004 ***
 *
 *  by Nicola Cassetta
 *
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


// Questo file e il relativo .cpp sono stati confrontati con l'equivalente in jdksmid ed aggiornati




#ifndef MIDI_TRACK_H_INCLUDED
#define MIDI_TRACK_H_INCLUDED


#include "midi.h"
#include "sysex.h"
#include "msg.h"


const int MIDITrackChunkSize = 512;
const int MIDIChunksPerTrack = 512;



class  MIDITrackChunk {
// low level buffer for avoiding memory fragmentation; no useful functions
    public:

        const MIDITimedBigMessage*  GetEventAddress(int num) const  { return &buf[num]; }
        MIDITimedBigMessage*        GetEventAddress(int num)        { return &buf[num]; }
        void                        Insert(int ev_num) { memmove((void *)(buf+ev_num+1), (void *)(buf+ev_num), (MIDITrackChunkSize - ev_num - 1)*sizeof(MIDITimedBigMessage));
                                                     memset((void *)(buf+ev_num), 0, sizeof(MIDITimedBigMessage)); }
        void                        Delete(int ev_num) { memmove((void *)(buf+ev_num), (void *)(buf+ev_num+1), (MIDITrackChunkSize - ev_num - 1)*sizeof(MIDITimedBigMessage)); }
        static void                 LastToFirst(const MIDITrackChunk& c1, MIDITrackChunk& c2)
                                                            { memcpy ((void *)(c2.buf), (void *)(c1.buf+MIDITrackChunkSize-1), sizeof(MIDITimedBigMessage));
                                                              memset((void *)(c1.buf+MIDITrackChunkSize-1), 0, sizeof(MIDITimedBigMessage)); }
        static void                 FirstToLast(const MIDITrackChunk& c1, MIDITrackChunk& c2)
                                                            { memcpy ((void *)(c2.buf+MIDITrackChunkSize-1), (void *)(c1.buf), sizeof(MIDITimedBigMessage));
                                                              memset((void *)(c1.buf), 0, sizeof(MIDITimedBigMessage)); }
    private:
        MIDITimedBigMessage         buf[MIDITrackChunkSize];

    // this is hazardous but works: we don't use operator= but memmove for copying messages
    // after moving bytes we put zero in old memory for preventing operator= to free
    // pointer data_p
};



// Defines the default behaviour of the methods MIDITrack::InsertEvent() and MIDITrack::InsertNote()
// when inserting events.
// If they are trying to insert an event into a track and find an equal or similar event at same MIDI time
// (see MIDITimedBigMessage::IsSameKind()) they can replace it with the new event or insert it
// without deleting the older. This is deternined by a static attribute of the class MIDITrack and can
// be changed by the MIDITrack::SetInsertMode() method (the default is INSMODE_INSERT_OR_REPLACE).
// When the above methods are called with default argoment *_ins_mode* they follow the default behaviour,
// this can be overriden giving them one of the other values as last parameter.
enum
{
    INSMODE_DEFAULT,    // follow the default behaviour (only used as default argument in methods MIDITrack::InsertEvent() and MIDITrack::InsertNote()
    INSMODE_INSERT,     // always insert events, if a same kind event was found  duplicate it.
    INSMODE_REPLACE,    // replace if a same kind event was found, otherwise do nothing.
    INSMODE_INSERT_OR_REPLACE,          // replace if a same kind event was found, otherwise insert.
    INSMODE_INSERT_OR_REPLACE_BUT_NOTE  // as above, but allow two same note events at same time (don't replace, insert a new note).
};

// Defines the behaviour of the method MIDITrack::FindEventNunber() when searching events.
enum
{
    COMPMODE_EQUAL,     // the method searches for an event matching equal operator.
    COMPMODE_SAMEKIND,  // the nethod searches for an event matching the MIDITimedBigMessage::IsSameKind() method.
    COMPMODE_TIME       // the method searches for the first event with time equal to the event time.
};


class  MIDITrack {
// sequence of MIDITimedBigMessage events, ended by an MIDI_END, with methods for editing them
    public:
                                    MIDITrack(MIDIClockTime end = 0);
                                    MIDITrack(const MIDITrack &t);
                                    ~MIDITrack();
        MIDITrack&                  operator=(const MIDITrack &t);

        void	                    Clear(bool mantain_end = false);
                                                    // empty the track, if mantain_end mantains
                                                    // MIDI_END time
        int                         GetBufferSize() const { return buf_size; }
                                                    // allocated buffer capacity (events)
        int	                        GetNumEvents() const { return num_events; }
                                                    // events in track
        MIDITimedBigMessage*        GetEventAddress(int ev_num);
        const MIDITimedBigMessage*  GetEventAddress(int ev_num) const;
        MIDITimedBigMessage&        GetEvent(int ev_num);
        const MIDITimedBigMessage&  GetEvent(int ev_num) const;
        MIDIClockTime               GetEndTime(void) const  { return GetEvent(num_events - 1).GetTime(); }
                                                    // safe: there's always MIDI_END event
        bool                        SetEndTime(MIDIClockTime end);
        bool                        EndOfTrack(int ev_num) const { return ev_num >= num_events; }
                                                    // true if ev_num is not valid
                                                    // newly compared with MIDITrack and revised in 0.7!!! WORKS???

        static void                 SetInsertMode( int m );
                // Sets the default behaviour for the methods InsertEvent() and InsertNote().

        bool                        InsertEvent( const MIDITimedBigMessage& msg, int _ins_mode = INSMODE_DEFAULT );
                // Inserts a single event into the track. ins_mode determines the behaviour of the method
                // if it finds an equal or similar event with same MIDI time in the track:

        bool                        InsertNote( const MIDITimedBigMessage& msg, MIDIClockTime len,
                                                int _ins_mode = INSMODE_DEFAULT );
                // Inserts a Note On and a Note Off event into the track.

        bool                        DeleteEvent( const MIDITimedBigMessage& msg );
                // Deletes an event from the track.

        bool                        DeleteNote( const MIDITimedBigMessage& msg );
                // Deletes a Note On and corresponding Note Off events from the track.

        void                        ChangeChannel(int ch);
                                                    // sets all channel events to channel ch
        void                        InsertInterval(MIDIClockTime start, MIDIClockTime length,
                                                   bool sysex = 0, const MIDITrack* src = 0);
                                                    // if src == 0 shift events of length clocks, insert or not sysex
        MIDITrack*                  MakeInterval(MIDIClockTime start, MIDIClockTime end, MIDITrack* interval) const;
                                                    // copies selected interval into another interval
        void                        DeleteInterval(MIDIClockTime start, MIDIClockTime end);
                                                    // deletes events and shifts subsequents
        void                        ClearInterval(MIDIClockTime start, MIDIClockTime end);
                                                    // deletes events leaving subsequents unchanged
        void                        ReplaceInterval(MIDIClockTime start, MIDIClockTime length,
                                                   bool sysex, const MIDITrack* src);
        void                        CloseOpenEvents(MIDIClockTime t);
                                                    // stops sounding notes, resets sustain and pitch bend
        bool                        FindEventNumber( const MIDITimedBigMessage& msg, int *event_num,
                                                     int _comp_mode = COMPMODE_EQUAL) const;
                // Finds an event in the track matching msg
                // event_num returns the event number in the track if the event was found; otherwise
                // -1 if event time was invalid, or the number of the first event with the same event time.
                // _comp_mode an enum value (see above)



        bool                        FindEventNumber ( MIDIClockTime time, int *event_num ) const;
                // Finds the first event in the track with the given time.

        /*
        int                         FindEventNumber(const MIDITimedBigMessage& m) const;
                                                     // returns -1 if not found
        int	                        FindEventNumber(MIDIClockTime time) const;
                                                     // as above; finds first event with time greater or eq
        */

        /*
        bool	PutEvent( const MIDITimedBigMessage &msg, MIDISysEx *sysex );
                // INCOHOERENT WITH THE REST, but needed by jdkmidi_filereadmultitrack!!!!!!
        */

    protected :

        //bool                        AppendEvent(const MIDITimedBigMessage& m);
        bool                        PutEvent(int ev_num, const MIDITimedBigMessage& m);
        bool	                    SetEvent(int ev_num, const MIDITimedBigMessage& m);
        bool                        RemoveEvent(int ev_num);
        void                        Shrink();
        bool	                    Expand(int increase_amount = MIDITrackChunkSize);

        int                         num_events;
        MIDITrackChunk*             chunk[MIDIChunksPerTrack];
        int                         buf_size;

        static int ins_mode;
};


class MIDITrackIterator {
// iterator for moving along a MIDITrack. When moving, it keeps track of
// program, bender, control changes and notes on

    public:
                                    MIDITrackIterator(MIDITrack* trk);

        void                        Reset();
        MIDITrack*                  GetTrack()                  { return track; }
        const MIDITrack*            GetTrack() const 	        { return track;  }
        void                        SetTrack(MIDITrack* trk);
        MIDIClockTime               GetCurTime() const          { return cur_time; }

        char                        GetProgram() const          { return program; }
        char                        GetControl(uchar c) const   { return controls[c]; }
        short                       GetBender() const           { return bender_value; }
        char                        GetMIDIChannel() const      { return channel; }
        int                         NotesOn() const             { return num_notes_on; }
        char                        IsNoteOn(uchar n) const     { return notes_on[n]; }
        bool                        IsPedalOn() const           { return controls[64] > 64; }
        bool                        FindNoteOff(uchar note, MIDITimedBigMessage** msg);
        bool                        FindPedalOff(MIDITimedBigMessage** msg);
        bool                        GetCurEvent(MIDITimedBigMessage** msg, MIDIClockTime end = TIME_INFINITE);
                                        // in **msg next event on track,
                                        // true if msg is valid and cur_time <= end
        MIDIClockTime               GetCurEventTime() const;
                                        // returns TIME_INFINITE if we are at end of track
        bool                        EventIsNow(const MIDITimedBigMessage& msg);
                                        // true if at cur_time there is msg

        bool                        GoToNextEvent();
        bool                        GoToTime(MIDIClockTime time);


    private:

        void                        FindChannel();
        bool                        Process(const MIDITimedBigMessage *msg);
        void                        ScanEventsAtThisTime();
                                        // warning: this can be used only when we reach the first
                                        // event at a new time!

        MIDITrack*                  track;
        char                        channel;
        int                         cur_ev_num;
        MIDIClockTime               cur_time;

        char                        program;        // current program change, or -1
        char                        controls[128];  // value of ervery control change, or -1
        short                       bender_value;	// last seen bender value
        uchar                       num_notes_on;	// number of notes currently on
        char		                notes_on[128];  // 0 if off, or velocity
};



//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////








/*
    /// Returns the length in MIDI clocks of the given note. _msg_ must be a Note On event present in the track
    /// (otherwise the function will return 0).
    MIDIClockTime NoteLength ( const MIDITimedBigMessage& msg ) const;


    /// Cuts note and pedal events at the time t. All sounding notes and held pedals are truncated, the
    /// corresponding off events are properly managed.
    void CloseOpenEvents( MIDIClockTime t );

*/



#endif
