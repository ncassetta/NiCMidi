/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2020  Nicola Cassetta
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
/// Contains the definitions of classes MIDIMultiTrackCopier and MIDIRecorder.


#ifndef RECORDER_H_INCLUDED
#define RECORDER_H_INCLUDED

#include "sequencer.h"
#include "process.h"

#include <atomic>
#include <vector>
#include <set>

/*
class MIDIMultiTrackCopier {
    public:
                                        MIDIMultiTrackCopier();
        virtual                         ~MIDIMultiTrackCopier();

        void                            Reset();
        MIDITrack*                      GetTrackDest(unsigned int n) const;

        void                            SetTrackDest(unsigned int n, MIDITrack* trk);
        void                            ResetTrackDest(unsigned int n);

        void                            Copy();

    private:
        MIDIMultiTrack* source;
        std::vector<MIDITrack*> dest;
};
*/

/*
///
/// Stores current MIDI general parameters for a MIDISequencer object, embedding a MIDISequencerTrackState for each track.
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
class MIDIRecorderState : public MIDIProcessor {
// Doesn't inherit from MIDISequencerGUINotifier because notifier and sequencer must be independent objects
// (notifier is used also by the MIDIManager)
// All is public: used by various classes
    public:
        /// The constructor is called by the MIDISequencer class constructor, which sets appropriate
        /// values for parameters.
                                MIDIRecorderState(MIDIMultiTrack *multitrack_,
                                                  MIDISequencerGUINotifier *n = 0);
        /// The copy constructor. \note This only copies the pointers to the sequencer and the notifier,
        /// so you can use it only if you are copying different states of the same MIDISequencer
        /// class instance.
                                MIDIRecorderState(const MIDIRecorderState &s);
        /// The destructor. The sequencer and notifier pointers are not owned by the class and they
        /// won't freed.
        virtual                 ~MIDIRecorderState();
        /// The assignment operator. See the note to the copy constructor.
        const MIDIRecorderState& operator= (const MIDIRecorderState &s);

        /// Resets the state to default values. These are: cur_clock = 0, tempo = 120 bpm,
        /// time = 4/4, keysig = C Maj, no marker. Moreover resets all track states (see
        /// MIDISequencerTrackState::Reset()).
        void                    Reset();
        /// This is the process function inherited from MIDIProcessor. When you get a MIDI message
        /// from the sequencer, it is processed by the state, which updates its parameters and
        /// notifies the GUI if required.
        bool                    Process( MIDITimedMessage* msg );
        /// Notifies the GUI when something happens (a parameter was changed,
        /// current time is moved, etc.)
        void                    Notify(int group, int item = 0) const;
        /// These are used for notifying the GUI when something happens (a parameter was changed,
        /// current time is moved, etc.)
        void                    NotifyTrack(int item) const;

        MIDISequencerGUINotifier* notifier;         ///< The notifier
        MIDIMultiTrack*         multitrack;         ///< The MIDIMultiTrack holding MIDI messages
        MIDIMultiTrackIterator  iterator;           ///< The iterator for moving along the multitrack

        MIDIClockTime           cur_clock;          ///< The current MIDI clock in MIDI ticks
        float                   cur_time_ms;        ///< The current clock in milliseconds
        unsigned int            cur_beat;           ///< The current beat in the measure (1st beat is 0)
        unsigned int            cur_measure;        ///< The current measure (1st measure is 0)
        MIDIClockTime           beat_length;        ///< The duration of a beat
        MIDIClockTime           next_beat_time;     ///< The MIDI time of the next beat (for internal use)

        float                   tempobpm;           ///< The current tempo in beats per minute
        char                    timesig_numerator;  ///< The numerator of current time signature
        char                    timesig_denominator;///< The denominator of current time signature
        //char                    keysig_sharpflat;   ///< The current key signature accidents (
        //char                    keysig_mode;        ///< Major mode (0) or minor (1)
        //std::string             marker_text;        ///< The current marker
        //std::vector<MIDISequencerTrackState*>
        //                        track_states;       ///< A track state for every track
        int                     last_event_track;   ///< Internal use
        MIDIClockTime           last_beat_time;     ///< Internal use
        static int              metronome_mode;     ///< Flag affecting how metronome beat is calculated
};
*/


class RecNotifier: public MIDISequencerGUINotifier {
    public:
                                        RecNotifier(MIDISequencer* seq = 0);
        /// Returns the MIDI note number for the measure click.
        unsigned char                   GetMeasNote() const                 { return meas_note; }
        /// Returns the MIDI note number for the ordinary beat click.metronome.
        unsigned char                   GetBeatNote() const                 { return beat_note; }
        /// Returns the attached sequencer notifier.
        MIDISequencerGUINotifier*       GetOtherNotifier() const            { return other_notifier; }
        /// Sets the MIDI note number for the measure click (the 1st beat of the measure).
        void                            SetMeasNote(unsigned char note)     { meas_note = note; }
        /// Sets the MIDI note number for the ordinary beat click.
        void                            SetBeatNote(unsigned char note)     { beat_note = note; }
        /// Sets the MIDI out port for the metronome clicks.
        /// \param port The out MIDI port id number
        /// \warning This doesn't check if _p_ is a valid port number.
        void                            SetOutPort(unsigned int p)          { port = p; }
        /// Sets the MIDI channel for the metronome clicks (channels are numbered 0 ... 15).
        void                            SetOutChannel(unsigned char ch)     { chan = ch & 0x0f; }
        /// Remembers the original sequencer notifier.
        void                            SetOtherNotifier(MIDISequencerGUINotifier* n)
                                                                            { other_notifier = n; }
        virtual void                    Notify(const MIDISequencerGUIEvent &ev);
    private:
        const unsigned char             DEFAULT_MEAS_NOTE = 67;
        const unsigned char             DEFAULT_BEAT_NOTE = 60;

        unsigned char                   meas_note;          // The MIDI note number for the measure click (1st note of a measure)
        unsigned char                   beat_note;          // The MIDI note number for the ordinary beat click
        unsigned int                    port;               // The out port id
        unsigned char                   chan;               // The MIDI channel for sound output
        MIDIMessage                     on_msg;
        MIDIMessage                     off_msg;
        MIDISequencerGUINotifier*       other_notifier;
};


///
/// A MIDITickComponent which can record MIDI messages incoming from a MIDI in port, putting them into an internal
/// MIDIMultiTrack. You can select the port, channel and MIDI notes of the metronome clicks; moreover you can have
/// three types of click: the ordinary (beat) click, the measure click (first beat of a measure) and a subdivision
/// click. If you enable measure clicks the metronome can count the measures and the beats of a measure (so you can
/// represent them in a graphical interface).
///
class MIDIRecorder : public MIDITickComponent {
    public:
        /// The constructor.
                                        MIDIRecorder(MIDISequencer* const s);
        /// The destructor
        virtual                         ~MIDIRecorder();
        virtual void                    Reset();

        /// Returns a pointer to the internal MIDIMultiTrack.
        MIDIMultiTrack*                 GetMultiTrack() const           { return tracks; }
        /// Returns the start MIDI time of the recording.
        MIDIClockTime                   GetStartRecTime() const         { return rec_start_time; }
        /// Returns the end MIDI time of the recording.
        MIDIClockTime                   GetEndRecTime() const           { return rec_end_time; }
        /// Returns the pointer to the track
        /// \param trk_num The track number
        MIDITrack*                      GetTrack(int trk_num)           { return tracks->GetTrack(trk_num); }
        /// Returns the pointer to the track
        /// \param trk_num The track number
        const MIDITrack*                GetTrack(int trk_num) const     { return tracks->GetTrack(trk_num); }

        /// Returns the number of the out port assigned to a track.
        /// \param trk_num the track number
        unsigned int                    GetTrackInPort(unsigned int trk_num) const
                                                                { return tracks->GetTrack(trk_num)->GetInPort(); }
        /// Returns the recording channel for the given track, or -1 if it is not defined. You can
        /// force a track to receive a given channel with SetTrackChannel().
        char                            GetTrackRecChannel(unsigned int trk_num)
                                                            { return tracks->GetTrack(trk_num)->GetRecChannel(); }
        /// Sets the MIDI in port for a track. This method is thread-safe, however changing
        /// a port during playback can lead to unexpected results in recording.
        /// \param trk_num the track number
        /// \param port the id number of the port (see MIDIManager::GetOutPorts())
        /// \return **true** if parameters are valid (and the port has been changed), **false** otherwise.
        bool                            SetTrackInPort(unsigned int trk_num, unsigned int port);
        /// Sets the recording channel for the given track.
        /// \param trk_num the track number
        /// \param chan the channel:You can specify a number between 0 ... 15 or -1 for any channel.
        /// \return **true** if parameters are valid (and the channel has been changed), **false** otherwise.
        bool                            SetTrackRecChannel(unsigned int trk_num, char chan);
        /// Sets the recording start time.
        /// \param t the MIDI clock time
        void                            SetStartRecTime(MIDIClockTime t)   { rec_start_time = t; }
        void                            SetStartRecTime(unsigned int meas, unsigned int beat = 0);

        void                            SetEndRecTime(MIDIClockTime t)      { rec_start_time = t; }
        void                            SetEndRecTime(unsigned int meas, unsigned int beat = 0);

        /// Inserts into the internal MIDIMultiTrack a new empty track with default track parameters (transpose,
        /// time offset, etc.). This method is thread-safe and can be called during playback.
        /// \param trk_num the track number (it must be in the range 0 ... GetNumTracks() - 1). If you leave the
        /// default value the track will be appended as last.
        /// \return **true** if the track was effectively inserted
        /// \note If you insert a track in the attached MIDISequencer you **must** call this with the same parameter
        /// for syncronizing the recorder with its sequencer.If you change the number of tracks of the sequencer in
        /// a more drastic mode (for example loading a MIDI file) you must call MIDIRecorder::Reset() (which disables
        /// all tracks for recording).
        bool                            InsertTrack(int trk_num = -1);
        /// Deletes a track and all its events from the internal MIDIMultiTrack. This method is thread-safe and can
        /// be called during playback.
        /// \param trk_num the track number (must be in the range 0 ... GetNumTracks() - 1). If you leave he default
        /// value the last track wil be deleted.
        /// \return **true** if the track was effectively deleted
        /// \see note to InsertTrack()
        bool                            DeleteTrack(int trk_num = -1);
        /// Moves a track from one position to another in the internal MIDIMultiTrack. This method is thread-safe and can
        /// be called during playback.
        /// \param from, to the start and destination track numbers (both must be in the range 0 ... GetNumTracks() - 1).
        /// \return **true** if the track was effectively moved
        /// \see note to InsertTrack()
        bool                            MoveTrack(int from, int to);
        /// Enables a sequencer track for recording. If the track was already enabled it does nothing.
        /// \param trk_num the track number
        /// \return **true** if _trk_num_ is valid (and the track has been enabled), **false** otherwise.
        bool                            EnableTrack(unsigned int trk_num);
        /// Disables a sequencer track for recording. If the track was not enabled it does nothing.
        /// \param trk_num the track number
        /// \return **true** if _trk_num_ is valid (and the track has been enabled), **false** otherwise.
        bool                            DisableTrack(unsigned int trk_num);

        /// Starts the recording from the enabled ports and channels.
        virtual void                    Start();
        /// Stops the recording from the enabled ports and channels.
        virtual void                    Stop();

    protected:
        /// Internal function used in Start() to set the metronome. You shouldn't use it directly.
        void                            SetSeqNotifier();
        /// Internal function used in Stop() to reset the metronome. You shouldn't use it directly.
        void                            ResetSeqNotifier();
        /// Implements the static method inherited by MIDITickComponent and called at every timer tick.
        /// It only calls the member TickProc().
        static void                     StaticTickProc(tMsecs sys_time, void* pt);
        /// Implements the pure virtual method inherited from MIDITickComponent (you must not call it directly).
        virtual void                    TickProc(tMsecs sys_time);

        /// \cond EXCLUDED
        MIDISequencer* const            seq;                // The attached sequencer
        MIDIMultiTrack*                 tracks;             // The internal MIDIMultiTrack
        MIDIMultiTrack*                 seq_tracks;         // The sequencer multitrack
        std::vector<bool>               en_tracks;          // True if the corresponding track isenabled
        std::set<unsigned int>          en_ports;           // Enabled input ports

        tMsecs                          rec_time_offset;    // The time between time 0 and sequencer start
        //tMsecs                          sys_time_offset;    ///< The time between the timer start and the sequencer start
        MIDIClockTime                   rec_start_time;     // The MIDIClockTime of the beginning of recording
        MIDIClockTime                   rec_end_time;
        RecNotifier                     notifier;           // A notifier used as a metronome
        std::atomic<bool>               rec_on;
        /// \endcond
};



#endif // RECORDER_H_INCLUDED
