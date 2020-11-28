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

#include "tick.h"
#include "multitrack.h"

#include <atomic>
#include <vector>



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


///
/// A MIDITickComponent which can record MIDI messages incoming from a MIDI in port, putting them into an internal
/// MIDIMultiTrack. You can select the port, channel and MIDI notes of the
/// metronome clicks; moreover you can have three types of click: the ordinary (beat) click, the measure click (first beat
/// of a measure) and a subdivision click. If you enable measure clicks the metronome can count the measures and the beats
/// of a measure (so you can represent them in a graphical interface).
///
class MIDIRecorder : public MIDITickComponent {
    public:
        /// \name Constructors, destructor and reset
        ///@{

        /// The constructor.
                                        MIDIRecorder();
        /// The destructor
        virtual                         ~MIDIRecorder();
        virtual void                    Reset();
        ///@}

        /// \name The get methods
        ///@{

        /// Returns a pointer to the internal MIDIMultiTrack.
        MIDIMultiTrack*                 GetMultiTrack() const           { return multitrack; }
        /// Returns the recording tempo in bpm.
        float                           GetTempo() const                { return tempobpm; }
        /// Returns the start MIDI time of the recording.
        MIDIClockTime                   GetStartTime() const            { return start_time; }
        ///@}

        ///@name The 'Set' methods
        ///@{

        /// Sets the recording tempo.
        /// \param t the tempo in bpm
        void                            SetTempo(float t);

        //void                            SetEnableRec(bool on_off)       { rec_on.store(on_off); }
        /// Sets the recording start time.
        /// \param t the MIDI clock time
        void                            SetStartTime(MIDIClockTime t)   { start_time = t; }
        ///@}

        /// Enables a MIDI in port in the system for recording.
        /// \param port the system port id (you can use MIDIManager::GetNumMIDIIns() and
        /// MIDIManager::GetMIDIInName() for inspecting them).
        /// \param en_chans if étrueé (default) enables recording from all channels, creating a MIDITrack in
        /// the multitrack for every one, otherwise you must set the recording channel with EnableChannel()
        void                            EnablePort(unsigned int port, bool en_chans = true);
        /// Enables a specific MIDI channel for recording, creating a track for it in the internal MIDIMultiTrack.
        /// \param port the system port id  (see EnablePort()); if it isn't enabled yet, enables it.
        /// \param ch the MIDI channel (in the range 0...15).
        void                            EnableChannel(unsigned int port, unsigned int ch);
        /// Disables a MIDI in port from recording, deleting all tracks associated with it in the internal
        /// MIDIMultiTrack. If the port is not enabled it does nothing.
        /// \param port the system port id (see EnablePort())
        void                            DisablePort(unsigned int port);
        /// Disables a specific MIDI channel from recording, deleting its track in the internal MIDIMultiTrack.
        /// \param port the system port id (see EnablePort())
        /// \param ch the MIDI channel (in the range 0...15).
        void                            DisableChannel(unsigned int port, unsigned int ch);

        /// Sets the current time to the beginning of the song, updating the internal status. This method is
        /// thread-safe and can be called during playback. Notifies the GUI a GROUP_ALL event to signify a
        /// full GUI reset.
        void                            GoToZero()                      { GoToTime(0); }
        /// Sets the current time to the given MIDI time, updating the internal status. This is as
        /// MIDISequencer::GoToTime() but uses a better algorithm and sends to the MIDI out ports all the
        /// appropriate sysex, patch, pitch bend and control change messages.
        bool                            GoToTime(MIDIClockTime time_clk);
        /// Same as GoToTime(), but the time is given in milliseconds.
        bool                            GoToTimeMs(float time_ms);

        /// Starts the recording from the enabled ports and channels.
        virtual void                    Start();
        /// Stops the recording from the enabled ports and channels.
        virtual void                    Stop();

    protected:
        /// Implements the static method inherited by MIDITickComponent and called at every timer tick.
        /// It only calls the member TickProc().
        static void                     StaticTickProc(tMsecs sys_time, void* pt);
        /// Implements the pure virtual method inherited from MIDITickComponent (you must not call it directly).
        virtual void                    TickProc(tMsecs sys_time);

        /// \cond EXCLUDED
        MIDIMultiTrack*                 multitrack;         // The internal MIDIMultiTrack
        std::vector<std::vector<MIDITrack*>*> en_ports;     // A vector which keeps track of the corrispondence between
                                                            // channels and tracks in the multitrack

        tMsecs                          rec_time_offset;    // The time between time 0 and sequencer start
        //tMsecs                          sys_time_offset;    ///< The time between the timer start and the sequencer start
        float                           tempobpm;           // The recording tempo
        MIDIClockTime                   start_time;         // The MIDIClockTime of the beginning of recording

        std::atomic<bool>               rec_on;
        /// \endcond
};



#endif // RECORDER_H_INCLUDED
