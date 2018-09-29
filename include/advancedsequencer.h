/* Some code Copyright 1986 to 1998 By J.D. Koftinoff Software, Ltd.
 *
*/

#ifndef _JDKMIDI_ADVANCEDSEQUENCER_H
#define _JDKMIDI_ADVANCEDSEQUENCER_H

#include <vector>
#include <string>

#include "msg.h"
#include "driver.h"
#include "thru.h"
#include "multitrack.h"
#include "sequencer.h"
#include "manager.h"
#include "smpte.h"


/// \file
/// Contains the definition of the class AdvancedSequencer.



class AdvancedSequencer : public MIDISequencer {
    public:


                            AdvancedSequencer(MIDISequencerGUINotifier *n = 0);
                            AdvancedSequencer(MIDIMultiTrack* mlt, MIDISequencerGUINotifier *n = 0);
//                            AdvancedSequencer(MIDIManager* mg);
        virtual             ~AdvancedSequencer();

        void                Reset();

        bool                Load(const char *fname);
        void                UnLoad();
        bool                IsLoaded() const                { return file_loaded; }


        MIDIThru*           GetMIDIThru()                   { return thru; }

        int                 GetMIDIThruChannel() const      { return thru_rechannelizer.GetRechanMap(0); }
        void                SetMIDIThruTranspose (int amt);
        int                 GetMIDIThruTranspose() const    { return thru_transposer.GetTransposeChannel(0); }



        void                GoToZero()                      { GoToTime(0); }
        void                GoToMeasure(int measure, int beat = 0);
        void                GoToTime(MIDIClockTime t);

        void                Start();
        void                Stop();
        void                OutputMessage(MIDITimedMessage& msg, unsigned int port);

        void                SetRepeatPlay(bool f, int start_measure, int end_measure);

        void                SoloTrack(int trk);
        void                UnSoloTrack();
        bool                GetTrackSolo(int trk) const     { return track_processors[trk]->solo; }
        void                SetTrackMute(int trk, bool f);
        bool                GetTrackMute (int trk) const    { return track_processors[trk]->mute; }
        void                UnmuteAllTracks(void);
        void                SetTempoScale(double scale);

        /// Returns 'now' MIDI clock time.
        /// It is effective even during playback
        MIDIClockTime       GetCurrentMIDIClockTime() const;

        /// Returns 'now' time in milliseconds.
        /// When playing or jumping from one time to another, you can use this to feed a SMPTE
        tMsecs              GetCurrentTimeMs() const;

        /// Set MIDI ticks per beat (quarter note).
        /// \return **true** if clocks per beat are effectively changed
        /// \note  Currently the user is allowed to change this only when the sequencer is empty; default value is
        /// 120 clocks per quarter beat. However, LoadFile() can change this according to the file clock, and Unload()
        /// resets it to 120
        //bool                SetClksPerBeat ( unsigned int cpb );

        /// Returns the base MIDI ticks per beat of the internal MIDIMultiTrack. Default value is 120 clocks per
        /// quarter beat. However, LoadFile() can change this according to the file clock, and Unload()
        /// resets it to 120.
        int                 GetClksPerBeat() const              { return state.multitrack->GetClksPerBeat(); }

        /// Returns the number of measures of the sequencer.
        int                 GetNumMeasures() const              { return num_measures; }
        /// Returns the current measure number (first is 0).
        unsigned int        GetCurrentMeasure() const;
        /// Returns the number of current beat (first is 0).
        unsigned int        GetCurrentBeat() const;
        /// Returns the current MIDI time offset respect current beat.
        MIDIClockTime       GetCurrentBeatOffset() const;
        /// Returns the numerator of current time signature.
        int                 GetTimeSigNumerator() const;
        /// Returns the denominator of current time signature.
        int                 GetTimeSigDenominator() const;

        int                 GetKeySigSharpFlat() const;
        int                 GetKeySigMode() const;

        /// Sets the output MIDI port for the given track.
        void                SetTrackOutPort(int trk, unsigned int port);

        int                 GetTrackNoteCount(int trk) const;
        /// Returns the name of the given track.
        std::string         GetTrackName(int trk) const;
        /// Returns the current MIDI volume for the given track (-1 if volume wasn't set).
        char                GetTrackVolume(int trk) const;          // MIDI value or -1
        /// Returns the current MIDI program (patch) for the given track (-1 if the program wasn't set).
        char                GetTrackProgram ( int trk ) const;      // MIDI value or -1
        void                SetTrackVelocityScale(int trk, double scale);
        double              GetTrackVelocityScale(int trk) const;
        void                SetTrackRechannelize(int trk, int chan);
        int                 GetTrackRechannelize(int trk) const;
        int                 GetTrackChannel(int trk);   // NOT const!!!
        void                SetTrackTranspose(int trk, int trans);
        int                 GetTrackTranspose(int trk) const;
        void                SetTrackTimeShift(int trk, int time);
        int                 GetTrackTimeShift(int trk) const;
        std::string         GetCurrentMarker() const;

        void                SetSMPTE(SMPTE* s);

        void                SetChanged();

    protected:

        static const int                    MEASURES_PER_WARP = 4;

        void                                ExtractWarpPositions();
        void                                CatchEventsBefore();
        void                                CatchEventsBefore(int trk);
        static void                         AutoStopProc(void* p);

        //MIDIMultiTrack*                     multitrack;     ///< The sequencer multitrack
        //MIDISequencer*                      seq;            ///< The sequencer
        //MIDIManager*                        mgr;            ///< The MIDIManager for playing
        MIDIThru*                           thru;           ///> The MIDI thru

        MIDIMultiProcessor                  thru_processor; ///< Processes incoming MIDI messages for MIDI thru
        MIDIProcessorTransposer             thru_transposer;///< Transposes note messages while playing
        MIDIProcessorRechannelizer          thru_rechannelizer;
                                                            ///< Rechannelize messages while playing

        int                                 num_measures;   ///< Number of measures
        bool                                file_loaded;    ///< True if the multitrack is not empty

        std::vector<MIDISequencerState>     warp_positions; ///< Vector of MIDISequencerState objects for fast time moving

private:

        enum { CTOR_1, CTOR_2, CTOR_3 };
        int ctor_type;
};



/*    O L D
///
/// A high level, all-in-one object capable to load an play MIDI files.
/// Its features include:
/// - Loading of MIDI files with a single call to a class method
/// - Play, Repeat Play (i.e. loop) and Stop commands
/// - Allows jumping from one time to another (even during playback) with correct responding
/// to wheel, patch, controls and sysex changes
/// - Individual solo, mute, velocity scale, transpose and rechannelize for every track
/// - Global tempo scale
/// - MIDI thru: you can play along with your MIDI instrument while the sequencer is playing
///
/// This class embeds many library objects: a MIDISequencer (with its MIDIMultiTrack) for
/// storing MIDI data, a MIDIDriver to communicate with hardware MIDI ports, a MIDIManager for
/// handling sequencer playing, some MIDIProcessor for thru transposing, rechannelizing, etc.
///
class AdvancedSequencer {
    public:


                            AdvancedSequencer(MIDISequencerGUINotifier *n = 0);
                            AdvancedSequencer(MIDIMultiTrack* mlt, MIDISequencerGUINotifier *n = 0);
                            AdvancedSequencer(MIDIManager* mg);
        virtual             ~AdvancedSequencer();


        MIDIThru*           GetMIDIThru()                   { return thru; }

        int                 GetMIDIThruChannel() const      { return thru_rechannelizer.GetRechanMap(0); }
        void                SetMIDIThruTranspose (int amt);
        int                 GetMIDIThruTranspose() const    { return thru_transposer.GetTransposeChannel(0); }

        bool                Load(const char *fname);
        void                UnLoad();
        void                Reset();
        bool                IsLoaded() const                { return file_loaded; }

        MIDIMultiTrack*     GetMultiTrack()                 { return multitrack; }
        const MIDIMultiTrack* GetMultiTrack() const         { return multitrack; }


        void                GoToZero()                      { GoToTime(0); }
        void                GoToMeasure(int measure, int beat = 0);
        void                GoToTime(MIDIClockTime t);

        void                Play();
        void                Stop();
        void                OutputMessage(MIDITimedMessage& msg, unsigned int port);

        void                SetRepeatPlay(bool f, int start_measure, int end_measure);
        bool                GetRepeatPlay() const           { return seq->GetRepeatPlay(); }
        int                 GetRepeatPlayStart() const      { return seq->GetRepeatPlayStart(); }
        int                 GetRepeatPlayEnd() const        { return seq->GetRepeatPlayEnd(); }
        bool                IsPlaying() const               { return MIDIManager::IsSeqPlay(); }

        void                SoloTrack(int trk);
        void                UnSoloTrack();
        bool                GetTrackSolo(int trk) const     { return seq->GetTrackProcessor (trk)->solo; }
        void                SetTrackMute(int trk, bool f);
        bool                GetTrackMute (int trk) const    { return seq->GetTrackProcessor (trk)->mute; }
        void                UnmuteAllTracks(void);
        void                SetTempoScale(double scale);
        double              GetTempoWithoutScale(void) const{ return seq->GetTempo(); }
        double              GetTempoWithScale(void) const   { return seq->GetTempo() * seq->GetTempoScale(); }

        /// Returns 'now' MIDI clock time.
        /// It is effective even during playback
        MIDIClockTime       GetCurrentMIDIClockTime() const;

        /// Returns 'now' time in milliseconds.
        /// When playing or jumping from one time to another, you can use this to feed a SMPTE
        tMsecs              GetCurrentTimeMs() const;

        /// Set MIDI ticks per beat (quarter note).
        /// \return **true** if clocks per beat are effectively changed
        /// \note  Currently the user is allowed to change this only when the sequencer is empty; default value is
        /// 120 clocks per quarter beat. However, LoadFile() can change this according to the file clock, and Unload()
        /// resets it to 120
        //bool                SetClksPerBeat ( unsigned int cpb );

        /// Returns the base MIDI ticks per beat of the internal MIDIMultiTrack. Default value is 120 clocks per
        /// quarter beat. However, LoadFile() can change this according to the file clock, and Unload()
        /// resets it to 120.
        int                 GetClksPerBeat() const              { return multitrack->GetClksPerBeat(); }

        /// Returns the number of tracks of the sequencer.
        int                 GetNumTracks() const                { return seq->GetNumTracks(); }

        /// Returns the number of measures of the sequencer.
        int                 GetNumMeasures() const              { return num_measures; }
        /// Returns the current measure number (first is 0).
        int                 GetCurrentMeasure() const;
        /// Returns the number of current beat (first is 0).
        int                 GetCurrentBeat() const;
        /// Returns the current MIDI time offset respect current beat.
        int                 GetCurrentBeatOffset() const;
        /// Returns the numerator of current time signature.
        int                 GetTimeSigNumerator() const;
        /// Returns the denominator of current time signature.
        int                 GetTimeSigDenominator() const;

        int                 GetKeySigSharpFlat() const;
        int                 GetKeySigMode() const;

        /// Sets the output MIDI port for the given track.
        void                SetTrackOutPort(int trk, unsigned int port);
        /// Returns the output MIDI port for the given track.
        int                 GetTrackOutPort(int trk) const    { return seq->GetTrackPort(trk); }

        int                 GetTrackNoteCount(int trk) const;
        /// Returns the name of the given track.
        std::string         GetTrackName(int trk) const;
        /// Returns the current MIDI volume for the given track (-1 if volume wasn't set).
        char                GetTrackVolume(int trk) const;          // MIDI value or -1
        /// Returns the current MIDI program (patch) for the given track (-1 if the program wasn't set).
        char                GetTrackProgram ( int trk ) const;      // MIDI value or -1
        void                SetTrackVelocityScale(int trk, double scale);
        double              GetTrackVelocityScale(int trk) const;
        void                SetTrackRechannelize(int trk, int chan);
        int                 GetTrackRechannelize(int trk) const;
        int                 GetTrackChannel(int trk) const;
        void                SetTrackTranspose(int trk, int trans);
        int                 GetTrackTranspose(int trk) const;
        void                SetTrackTimeShift(int trk, int time);
        int                 GetTrackTimeShift(int trk) const;
        std::string         GetCurrentMarker() const;

        void                SetSMPTE(SMPTE* s);

        void                SetChanged();
        /// Converts a MIDI clock time into milliseconds from the beginning of the song,
        /// taking into account tempo changes. You can use calculated time to feed a SMPTE.
        double              MIDItoMs(MIDIClockTime t) { return seq->MIDItoMs(t); }

    protected:

        static const int                    MEASURES_PER_WARP = 4;

        void                                ExtractWarpPositions();
        void                                CatchEventsBefore();
        void                                CatchEventsBefore(int trk);
        static void                         AutoStopProc(void* p);

        MIDIMultiTrack*                     multitrack;     ///< The sequencer multitrack
        MIDISequencer*                      seq;            ///< The sequencer
        //MIDIManager*                        mgr;            ///< The MIDIManager for playing
        MIDIThru*                           thru;           ///> The MIDI thru

        MIDIMultiProcessor                  thru_processor; ///< Processes incoming MIDI messages for MIDI thru
        MIDIProcessorTransposer             thru_transposer;///< Transposes note messages while playing
        MIDIProcessorRechannelizer          thru_rechannelizer;
                                                            ///< Rechannelize messages while playing

        int                                 num_measures;   ///< Number of measures
        bool                                file_loaded;    ///< True if the multitrack is not empty

        std::vector<MIDISequencerState>     warp_positions; ///< Vector of MIDISequencerState objects for fast time moving

private:

        enum { CTOR_1, CTOR_2, CTOR_3 };
        int ctor_type;
};
*/

#endif // JDKSMIDI_ADVANCEDSEQUENCER_H

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
/*
** Copyright 1986 to 1998 By J.D. Koftinoff Software, Ltd.
**
** All rights reserved.
**
** No one may duplicate this source code in any form for any reason
** without the written permission given by J.D. Koftinoff Software, Ltd.
**
*/

//
// MODIFIED by N. Cassetta ncassetta@tiscali.it
//

/*
#ifndef JDKSMIDI_ADVANCEDSEQUENCER_H
#define JDKSMIDI_ADVANCEDSEQUENCER_H

#include "world.h"
#include "midi.h"
#include "msg.h"
#include "sysex.h"
#include "multitrack.h"
#include "filereadmultitrack.h"
#include "sequencer.h"
#include "manager.h"
#include "driver.h"


#ifdef _WIN32
#include "driverwin32.h"
#else
#include "driverdump.h"
#endif // _WIN32



#include <vector>

// #define MAX_WARP_POSITIONS (128)
// #define MEASURES_PER_WARP (4)    NC this is now a static const class attribute


///
/// A high level, all-in-one object capable to load an play MIDI files.
/// Its features include:
/// - Loading of MIDI files with a single call to a class method
/// - Play, Repeat Play (i.e. loop) and Stop commands
/// - Allows jumping from one time to another (even during playback) with correct responding
/// to wheel, patch, controls and sysex changes
/// - Individual solo, mute, velocity scale, transpose and rechannelize for every track
/// - Global tempo scale
/// - MIDI thru: you can play along with your MIDI instrument while the sequencer is playing
///
/// This class embeds many jdksmidi objects: a MIDISequencer (with its MIDIMultiTrack) for
/// storing MIDI data, a MIDIDriver to communicate with hardware MIDI ports, a MIDIManager for
/// handling sequencer playing, some MIDIProcessor for transposing, rechannelizing, etc.
/// At current time, the class can manage MIDI playback only on WIN32; for other OS it prints to
/// the console a dump of sent messages.
///

class AdvancedSequencer
{
public:





    /// Returns 'now' MIDI clock time.
    /// It is effective even during playback
    unsigned long GetCurrentMIDIClockTime() const; / * NEW BY NC * /

    /// Returns 'now' time in milliseconds.
    /// When playing or jumping from one time to another, you can use this to feed a SMPTE
    unsigned long GetCurrentTimeInMs() const; / * NEW BY NC * /

    /// Set MIDI ticks per beat (quarter note).
    /// \return **true** if clocks per beat are effectively changed
    /// \note  Currently the user is allowed to change this only when the sequencer is empty; default value is
    /// 120 clocks per quarter beat. However, LoadFile() can change this according to the file clock, and Unload()
    /// resets it to 120
    bool SetClksPerBeat ( unsigned int cpb );

    /// Returns the base MIDI ticks per beat of the internal MIDIMultiTrack
    int GetClksPerBeat() const  / * NEW BY NC * /
    {
        return tracks->GetClksPerBeat();
    }



protected:

    bool file_loaded;                   ///< true if the MIDIMultiTrack is not empty
    / * NOTE by NC: I mantained this name, but its meaning is now different:
     * it is nonzero if tracks is nonempty (maybe by an user editing)
     * /



};



#endif // JDKSMIDI_ADVANCEDSEQUENCER_H
*/
