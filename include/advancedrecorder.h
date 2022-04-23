/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2021  Nicola Cassetta
 *   https://github.com/ncassetta/NiCMidi
 *
 *   This file is part of NiCMidi.
 *
 *   NiCMidi is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation, either version 3 of
 *   the License, or (at your option) any later version.
 *
 *   NiCMidi is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with NiCMidi. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _NICMIDI_ADVANCEDRECORDER_H
#define _NICMIDI_ADVANCEDRECORDER_H

#include "sequencer.h"


class RecNotifier: public MIDISequencerGUINotifier {
    public:
                                        RecNotifier(const MIDISequencer* seq = 0);
        /// Returns the MIDI note number for the measure click.
        unsigned char                   GetMeasNote() const                 { return meas_note; }
        /// Returns the MIDI note number for the ordinary beat click.metronome
        unsigned char                   GetBeatNote() const                 { return beat_note; }
        /// Sets the MIDI note number for the measure click (the 1st beat of the measure). It is only effective if you
        /// have already set the timesig numerator (see SetTimesigNumerator()).
        void                            SetMeasNote(unsigned char note)     { meas_note = note; }
        /// Sets the MIDI note number for the ordinary beat click.
        void                            SetBeatNote(unsigned char note)     { beat_note = note; }
        /// Sets the MIDI out port for the metronome clicks.
        /// \param port The out MIDI port id number
        void                            SetOutPort(unsigned int p)          { port = p; }
        /// Sets the MIDI channel for the metronome clicks (channels are numbered 0 ... 15).
        void                            SetOutChannel(unsigned char ch)             { chan = ch; }
        void                            SetNotifier(MIDISequencerGUINotifier* n)    { other_notifier = n; }

        virtual void                    Notify(const MIDISequencerGUIEvent &ev);
    private:
        const unsigned char             DEFAULT_MEAS_NOTE = 67;
        const unsigned char             DEFAULT_BEAT_NOTE = 60;

        unsigned char                   meas_note;          // The MIDI note number for the measure click (1st note of a measure)
        unsigned char                   beat_note;          // The MIDI note number for the ordinary beat click
        unsigned int                    port;               // The out port id
        unsigned char                   chan;               // The MIDI channel for sound output
        MIDITimedMessage                msg;
        MIDISequencerGUINotifier*       other_notifier;
};


class AdvancedRecorder : public MIDISequencer {
    public:
                                    AdvancedRecorder(MIDIMultiTrack* m, MIDISequencerGUINotifier* n = 0);
        virtual                     ~AdvancedRecorder();

        virtual void                Reset();


        bool                        GetRecEnable(unsigned int trk_num) const    { return rec_enable[trk_num]; }
        unsigned int                GetTrackInputPort(unsigned int trk_num) const
                                                                                { return track_inputs[trk_num];}

        void                        SetRecEnable(unsigned int trk_num, bool f);
        void                        SetTrackInputPort(unsigned int trk_num, unsigned int port);

        // Inherited from MIDIRecorder
        /// Sets the current time to the beginning of the song, updating the internal status. This method is
        /// thread-safe and can be called during playback. Notifies the GUI a GROUP_ALL event to signify a
        /// full GUI reset.
        void                        GoToZero()                      { GoToTime(0); }
        /// Sets the current time to the given MIDI time, updating the internal status. This method is
        /// thread-safe and can be called during playback. Notifies the GUI a GROUP_ALL event to signify a
        /// full GUI reset.
        //bool                GoToTime(MIDIClockTime time_clk);
        /// Same as GoToTime(), but the time is given in milliseconds.
        //bool                GoToTimeMs(float time_ms);
        /// Sets the current time to the given measure and beat, updating the internal status. This is as
        /// MIDISequencer::GoToMeasure() but uses a better algorithm and sends to the MIDI out ports all the
        /// appropriate sysex, patch, pitch bend and control change messages.
        //bool                GoToMeasure(int measure, int beat = 0);


        /// Inserts into the internal MIDIMultiTrack a new empty track with default track parameters (transpose,
        /// time offset, etc.). This method is thread-safe and can be called during playback. Notifies the GUI a
        /// GROUP_ALL event to signify a full GUI reset.
        /// \param trk_num the track number (it must be in the range 0 ... GetNumTracks() - 1). If you leave the
        /// default value the track will be appended as last.
        /// \return **true** if the track was effectively inserted
        /// \note You should not use the corresponding method of MIDIMultiTrack class, as it does not sync the
        /// iterator and the sequencer internal arrays. If you change the number of tracks directly in the
        /// multitrack (for example when loading a MIDI file) you must then call MIDISequencer::Reset() for
        /// updating the sequencer parameters, but this will reset all track parameters to the default.
        bool                            InsertTrack(int trk_num = -1);
        /// Deletes a track and all its events from the internal MIDIMultiTrack. This method is thread-safe and can
        /// be called during playback (in this case the sequencer will send a MIDI AllNotesOff message to the old track
        /// port). Notifies the GUI a GROUP_ALL event to signify a full GUI reset.
        /// \param trk_num the track number (must be in the range 0 ... GetNumTracks() - 1).
        /// \return **true** if the track was effectively deleted
        /// \see note to InsertTrack()
        bool                            DeleteTrack(int trk_num);
        /// Moves a track from one position to another in the internal MIDIMultiTrack. This method is thread-safe and can
        /// be called during playback (in this case the sequencer will send a MIDI AllNotesOff message to the involved
        /// ports). Notifies the GUI a GROUP_ALL event to signify a full GUI reset.
        /// \param from, to the start and destination track numbers (both must be in the range 0 ... GetNumTracks() - 1).
        /// \return **true** if the track was effectively moved
        /// \see note to InsertTrack()
        bool                            MoveTrack(int from, int to);



        virtual void                Start();
        virtual void                Stop();

    protected:

        static void                 StaticTickProc(tMsecs sys_time, void* pt);
        void                        TickProc(tMsecs sys_time);

        tMsecs                      pre_count_delay;
        //int                         status;


        std::vector<bool>           en_tracks;              // Tracks enabled for recordingcorrispondence between

        MIDIMultiTrack              old_tracks;             // Old tracks before recording
        RecNotifier                 metro;

        //tMsecs                      rec_time_offset;        // The time between time 0 and sequencer start
        //tMsecs                          sys_time_offset;    ///< The time between the timer start and the sequencer start
        MIDIClockTime               rec_start_time;         // The MIDIClockTime of the beginning of recording
        MIDIClockTime               rec_end_time;           // The MIDIClockTime of the end of recording

        std::vector<unsigned int>   track_inputs;           // The input port id for every track
        std::vector<bool>           rec_enable;             // True if the track is enabled for recording

        std::atomic<bool>           pre_count;              // True if metronome pre count is on
        std::atomic<bool>           rec_on;                 // True if recording on
        /// \endcond
};



#endif // ADVANCEDRECORDER_H_INCLUDED
