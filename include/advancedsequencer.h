/** Some code Copyright 1986 to 1998 By J.D. Koftinoff Software, Ltd.
**
*/

#ifndef _JDKMIDI_ADVANCEDSEQUENCER_H
#define _JDKMIDI_ADVANCEDSEQUENCER_H

#include <vector>
#include <string>




//#include "midi.h"
#include "msg.h"
#include "driver.h"
#include "multitrack.h"
#include "sequencer.h"
#include "manager.h"




class AdvancedSequencer {
    public:


                            AdvancedSequencer(MIDISequencerGUINotifier *n = 0);
                            AdvancedSequencer(MIDIMultiTrack* mlt, MIDISequencerGUINotifier *n = 0);
                            AdvancedSequencer(MIDIManager* mg);
        virtual             ~AdvancedSequencer();

        void                SetOutputPort( int p);
        int                 GetOutputPort() const           { return out_port; }
        void                SetInputPort( int p);
        int                 GetInputPort() const            { return in_port; }

        void                SetMIDIThruEnable (bool f)      { mgr->SetThruEnable (f); };
        bool                GetMIDIThruEnable() const       { return mgr->GetThruEnable(); };
        void                SetMIDIThruChannel (int chan);
        int                 GetMIDIThruChannel() const      { return thru_rechannelizer.GetRechanMap(0); };
        void                SetMIDIThruTranspose (int val);
        int                 GetMIDIThruTranspose() const    { return thru_transposer.GetTransposeChannel(0); };

        bool                Load(const char *fname);
        void                UnLoad(void);
        void                Reset(void);
        bool                IsLoaded(void) const            { return file_loaded; };

        MIDIMultiTrack*     GetMultiTrack()                 { return tracks; }
        const MIDIMultiTrack* GetMUltiTrack() const         { return tracks; }

        void                GoToZero();
        void                GoToMeasure(int measure, int beat = 0);
        void                GoToTime(MIDIClockTime t);

        void                Play();
        void                Stop();
        void                OutputMessage(MIDITimedMessage& msg);       // TODO: MIDIMessage ???
        void                SetRepeatPlay(bool enable, int start_measure, int end_measure);
        bool                GetRepeatPlay() const           { return repeat_play_mode; }
        int                 GetRepeatPlayStart() const      { return repeat_start_measure; }
        int                 GetRepeatPlayEnd() const        { return repeat_end_measure; }
        bool                IsPlay() const                  { return mgr->IsSeqPlay(); };

        void                SoloTrack(int trk);
        void                UnSoloTrack(void);
        bool                GetTrackSolo(int trk)           { return seq->GetTrackProcessor (trk)->solo; }
        void                SetTrackMute(int trk, bool f);
        bool                GetTrackMute (int trk)          { return seq->GetTrackProcessor (trk)->mute; }
        void                UnmuteAllTracks(void);
        void                SetTempoScale(double scale)
                                    { if (file_loaded) seq->SetCurrentTempoScale( (float)scale); }
        double              GetTempoWithoutScale(void) const
                                    { return seq->GetCurrentTempo(); }
        double              GetTempoWithScale(void) const
                                    { return seq->GetCurrentTempo() * seq->GetCurrentTempoScale(); }

        /// Returns 'now' MIDI clock time.
        /// It is effective even during playback
        MIDIClockTime       GetCurrentMIDIClockTime() const; /* NEW BY NC */

        /// Returns 'now' time in milliseconds.
        /// When playing or jumping from one time to another, you can use this to feed a SMPTE
        tMsecs              GetCurrentTimeInMs() const; /* NEW BY NC */

        /// Set MIDI ticks per beat (quarter note).
        /// \return **true** if clocks per beat are effectively changed
        /// \note  Currently the user is allowed to change this only when the sequencer is empty; default value is
        /// 120 clocks per quarter beat. However, LoadFile() can change this according to the file clock, and Unload()
        /// resets it to 120
        bool                SetClksPerBeat ( unsigned int cpb );

        /// Returns the base MIDI ticks per beat of the internal MIDIMultiTrack
        int                 GetClksPerBeat() const  /* NEW BY NC */
                                    { return tracks->GetClksPerBeat(); }

        /// Returns the number of tracks of the sequencer
        int GetNumTracks() const
        /* NOTE BY NC: actually this always returns 17, the number of tracks of the MIDIMultiTrack
        * TODO: modify the MIDIMultiTrack class to keep track of tracks actually used and implement
        * a function GetUsedTracks()
        */
                                    { return seq->GetNumTracks(); }


        int                 GetNumMeasures() const              { return num_measures; }
        int                 GetMeasure() const;
        int                 GetBeat() const;
        int                 GetTimeSigNumerator() const;
        int                 GetTimeSigDenominator() const;
        int                 GetKeySigSharpFlat() const;
        int                 GetKeySigMode() const;
        int                 GetTrackNoteCount(int trk) const;
        std::string         GetTrackName(int trk) const;
        int                 GetTrackVolume(int trk) const;
        int                 GetTrackProgram ( int trk ) const;
        void                SetTrackVelocityScale(int trk, double scale);
        double              GetTrackVelocityScale(int trk) const;
        void                SetTrackRechannelize(int trk, int chan);
        int                 GetTrackRechannelize(int trk) const;
        void                SetTrackTranspose(int trk, int trans);
        int                 GetTrackTranspose(int trk) const;
        std::string         GetCurrentMarker() const;

        int                 FindFirstChannelOnTrack(int trk);
        void                SetChanged();
        // new : added by me
        float               MIDItoMs(MIDIClockTime t) { return seq->MIDItoMs(t); }

    protected:

        static const int                    MEASURES_PER_WARP = 4;
        //static const int                    DEFAULT_CLK_PER_BEAT = 120;



        /*
        bool                                OpenMIDI ( int in_port, int out_port,
                                                       int timer_resolution = MIDIManager::DEFAULT_TIMER_RESOLUTION );
        void                                CloseMIDI();
        */


        void                                ExtractWarpPositions(void);
        void                                CatchEventsBefore(void);
        void                                CatchEventsBefore(int trk);

        MIDISequencerGUINotifier*           notifier;
        MIDIMultiTrack*                     tracks;
        MIDISequencer*                      seq;
        MIDIManager*                        mgr;

        MIDIMultiProcessor                  thru_processor;
        MIDIProcessorTransposer             thru_transposer;
        MIDIProcessorRechannelizer          thru_rechannelizer;

        int                                 num_measures;
        int                                 repeat_start_measure;
        int                                 repeat_end_measure;
        bool                                repeat_play_mode;
        bool                                file_loaded;

        std::vector<MIDISequencerState>     warp_positions;
    /* NOTE by NC: I realized that in an editing contest an array of pointers wasted much time
     * allocating and deallocating memory: so it's now a vector of objects
     */

        int                                 in_port;
        int                                 out_port;

private:
    enum { CTOR_1, CTOR_2, CTOR_3 };
    int ctor_type;
};

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
