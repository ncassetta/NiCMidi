/*
 *  libjdkmidi-2004 C++ Class Library for MIDI
 *
 *  Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *  www.jdkoftinoff.com
 *  jeffk@jdkoftinoff.com
 *
 *  *** RELEASED UNDER THE GNU GENERAL PUBLIC LICENSE (GPL) April 27, 2004 ***
 *
 *  MODIFIED BY NICOLA CASSETTA
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
**	Copyright 2019 By N. Cassetta.
**
**	All rights reserved.
**
*/


/// \file
/// Contains the definitions of the class Metronome.


#ifndef METRONOME_H_INCLUDED
#define METRONOME_H_INCLUDED


#include "midi.h"
#include "notifier.h"
#include "tick.h"



///
/// A MIDITickComponent implementing a metronome. You can select the port, channel and MIDI notes of the
/// metronome clicks; moreover you can have three types of click: the ordinary (beat) click, the measure click (first beat
/// of a measure) and a subdivision click. If you enable measure clicks the metronome can count the measures and the beats
/// of a measure (so you can represent them in a graphical interface).
///
class Metronome : public MIDITickComponent {
    public:
        /// The constructor.
        /// \param n a pointer to a MIDISequencerGUIEventNotifier. If you leave 0 the sequencer will not notify
        /// the GUI.
                                        Metronome(MIDISequencerGUINotifier* n = 0);
        /// The destructor. The MIDISequencerGUINotifier is not owned by the MIDISequencer.
        virtual                         ~Metronome() { Stop(); }
        /// Stops the metronome and resets it to its default values
        virtual void                    Reset();
        /// Returns the number of MIDIticks for a quarter note
        /// Returns current MIDIClockTime in MIDI ticks from the start of the metronome.
        MIDIClockTime                   GetCurrentMIDIClockTime() const     { return cur_clock; }
        /// Returns current time in milliseconds from the start of the metronome.
        float                           GetCurrentTimeMs() const;
        /// Returns current measure (1st measure is 0). Measures count is suspended if you disable the measure
        /// click (see SetTimeSigNumerator()).
        unsigned int                    GetCurrentMeasure() const           { return cur_measure; }
        /// Returns current beat in the measure (1st beat is 0).
        unsigned int                    GetCurrentBeat() const              { return cur_beat; }
        /// Returns current tempo scale in percentage (100 = no scaling, 200 = twice faster, etc.).
        unsigned int                    GetTempoScale() const               { return new_tempo_scale; }
        /// Returns current tempo (BPM) without scaling.
        float                           GetTempoWithoutScale() const        { return new_tempobpm; }
        /// Returns current tempo (BPM) taking into account scaling (this is the true actual tempo).
        float                           GetTempoWithScale() const           { return new_tempobpm * new_tempo_scale * 0.01; }
        /// Returns the number of the MIDI out port assigned to the metronome.
        int                             GetOutPort() const                  { return new_port; }
        /// Returns the number of the MIDI channel assigned to the metronome.
        unsigned char                   GetOutChannel() const               { return new_chan; }
        /// Returns the MIDI note number for the measure click.
        unsigned char                   GetMeasNote() const                 { return new_meas_note; }
        /// Returns the MIDI note number for the ordinary beat click.
        unsigned char                   GetBeatNote() const                 { return new_beat_note; }
        /// Returns the MIDI note number for the subdivision click.
        unsigned char                   GetSubdNote() const                 { return new_subd_note; }
        /// Returns the subdivision type. It can be 0 (subd clicks disabled), 2, 3, 4, 5, 6.
        unsigned char                   GetSubdType() const                 { return new_subd_type; }
        /// Returns the numerator of the current timesig. If it is 0 the measure clicks are disabled, otherwise the 1st click
        /// of a measure will sound with different note and volume.
        unsigned char                   GetTimeSigNumerator() const         { return timesig_numerator; }
        ///
        unsigned char                   GetTimeSigDenominator() const       { return timesig_denominator; }

        void                            SetTempo(float t);

        /// Sets the global tempo scale (_scale_ is the percentage: 100 = no scaling, 200 = twice faster, etc.).
        void                            SetTempoScale(unsigned int scale);
        /// Sets the MIDI out port for the metronome clicks.
        void                            SetOutPort(unsigned int p);
        /// Sets the MIDI channel for the metronome clicks.
        void                            SetOutChannel(unsigned char ch);
        /// Sets the MIDI note number for the measure click (the 1st beat of the measure). It is only effective if you
        /// have already set the timesig numerator (see SetTimesigNumerator()).
        void                            SetMeasNote(unsigned char note);
        /// Sets the MIDI note number for the ordinary beat click.
        void                            SetBeatNote(unsigned char note);
        /// Sets the MIDI note number for the subdivision click.
        void                            SetSubdNote(unsigned char note);
        ///
        void                            SetSubdType(unsigned char type);
        ///
        void                            SetTimeSigNumerator(unsigned char n);
        /// TODO:
        void                            SetTimeSigDenominator(unsigned char d);

        // Inherited from MIDITICK

        virtual void Start();
        virtual void Stop();


/*


        static void                     SetMetronomeMode(int mode)
                                                        { metronome_mode = mode; }
*/

    protected:
        /// Copies the temp values assigned by the set methods into the effective ones. It is
        /// called immediately if the metronome is not running, otherwise at every click, so
        /// the metronome parameters are updated in sync.
        void                            UpdateValues();
        /// Inherited by MIDITICK
        static void                     StaticTickProc(tMsecs sys_time, void* pt);
        virtual void                    TickProc(tMsecs sys_time);

        MIDISequencerGUINotifier*       notifier;           ///< The (optional) notifier

        int                             port;               ///< The out port id (-1 if unassigned)
        unsigned char                   chan;               ///< The MIDI channel for sound output
        unsigned char                   meas_note;          ///< The MIDI note number for the measure click (1st note of a measure)
        unsigned char                   beat_note;          ///< The MIDI note number for the ordinary beat click
        unsigned char                   subd_note;          ///< The MIDI note number for subdivision click
        unsigned char                   subd_type;          ///< Number of subdivisions (can be 2, 3, 4, 5, 6, 0 = disable subd click)
        unsigned char                   timesig_numerator;  ///< The numerator of current time signature (0 = disable measure click)
        unsigned char                   timesig_denominator;///< The denominator of current time signature (can be 2, 4, 8, 16)
        float                           tempobpm;           ///< The current tempo in beats per minute
        unsigned int                    tempo_scale;        ///< The tempo scale in percentage (100 = true time)

        MIDIClockTime                   cur_clock;          ///< The current MIDI clock in MIDI ticks from the metronme start
        float                           cur_time_ms;        ///< The current clock in milliseconds from the metronoe start
        unsigned int                    cur_beat;           ///< The current beat in the measure (1st beat is 0)
        unsigned int                    cur_measure;        ///< The current measure (1st measure is 0)

        MIDIClockTime                   beat_length;        ///< The duration of a beat
        float                           msecs_per_beat;     ///< Milliseconds per beat (for internal use)
        float                           onoff_time;         ///< Milliseconds between note on and off
        float                           next_time_on;       ///< The time of the next Note On message (for internal use)
        float                           next_time_off;      ///< The time of the next Note Off message (for internal use)


        /* UNUSED ????
        static int                      metronome_mode;     ///< Flag affecting how metronome beat is calculated
        MIDITimedMessage                msg_beat;
        */


        static const MIDIClockTime      QUARTER_LENGTH = 120;
        static const unsigned char      DEFAULT_CHAN = 9;
        static const unsigned char      DEFAULT_MEAS_NOTE = 60;
        static const unsigned char      DEFAULT_BEAT_NOTE = 58;
        static const unsigned char      DEFAULT_SUBD_NOTE = 56;
        static const unsigned char      MEAS_NOTE_VEL = 120;
        static const unsigned char      BEAT_NOTE_VEL = 100;
        static const unsigned char      SUBD_NOTE_VEL = 80;
        static const int                MIN_NOTE_LEN = 30;      // C++ trouble if you declare it float!

    private:
        // These are set by our set methods, and are used as buffer values, because real values are updated only at a
        // metronome (or measure) click
        int                             new_port;
        unsigned char                   new_chan;
        unsigned char                   new_meas_note;
        unsigned char                   new_beat_note;
        unsigned char                   new_subd_note;
        unsigned char                   new_subd_type;
        unsigned char                   new_timesig_numerator;
        unsigned char                   new_timesig_denominator;
        float                           new_tempobpm;
        unsigned int                    new_tempo_scale;
};


#endif // METRONOME_H_INCLUDED
