/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2010 V.R.Madgazin
 *   www.vmgames.com vrm@vmgames.com
 *   Copyright (C) 2021, 2022  Nicola Cassetta
 *   https://github.com/ncassetta/NiCMidi
 *
 *   This file is part of NiCMidi
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


/// \file
/// Contains the definition of the class SMPTE, plus some related enum.

#ifndef _NICMIDI_SMPTE_H
#define _NICMIDI_SMPTE_H


/// \addtogroup GLOBALS
///@{

/// The smpte rates (determine the frames/sec rate)
enum SMPTE_RATE {
    SMPTE_RATE_24 = 0,          ///< 24 frames/sec
    SMPTE_RATE_25,              ///< 25 frames/sec
    SMPTE_RATE_2997,            ///< 29.97 frames/sec
    SMPTE_RATE_2997DF,          ///< 29.97 frames/sec drop
    SMPTE_RATE_30,              ///< 30 frames/sec
    SMPTE_RATE_30DF             ///< 30 frames/sec drop
};


/// The sample rates (determine the sample/sec rate)
enum SAMPLE_RATE {
    SAMPLE_32000 = 0,           ///< 32000 samples/sec
    SAMPLE_44056,               ///< 44056 samples/sec
    SAMPLE_44100,               ///< 44100 samples/sec
    SAMPLE_47952,               ///< 47952 samples/sec
    SAMPLE_48000,               ///< 48000 samples/sec
    SAMPLE_48048                ///< 48048 samples/sec
};
///@}


///
/// Performs conversions between number of samples, milliseconds and smpte format
/// (hours::minutes::seconds::frames::subframes).
/// You can choose between several smpte formats and sample rates
/// (see \ref SMPTE_RATE and \ref SAMPLE_RATE). Moreover you can set an initial time offset to
/// be added to every value passed to the SMPTE.
/// \note Many get functions are NOT const because they can perform internal conversions
///
class  SMPTE {
    public:

        /// The constructor sets the SMPTE rate to SMPTE_30, the sample rate to SAMPLE_48000
        /// and the offset time to 0.
        /// \note There is no need for explicit copy ctor and operator=
                            SMPTE (SMPTE_RATE smpte_rate = SMPTE_RATE_30,
                                   SAMPLE_RATE sample_rate = SAMPLE_48000);

        /// Sets the smpte rate. See SMPTE_RATE for avalaible smpte rate formats.
        void                SetSMPTERate (SMPTE_RATE r)     { smpte_rate = r; sample_number_dirty = true; }

        /// Returns the smpte rate.
        SMPTE_RATE          GetSMPTERate() const            { return smpte_rate; }

        /// Sets the sample rate. See SAMPLE_RATES for avalaible sample rates formats.
        void                SetSampleRate (SAMPLE_RATE r)   { sample_rate = r; sample_number_dirty = true; }

        /// Returns the sample rate.
        SAMPLE_RATE         GetSampleRate() const           { return sample_rate; }

        /// \name To get and set the sample number
        ///@{
        /// Performs a smpte-to-samples or milliseconds-to-samples conversion.
        /// You must first load the SMPTE with the smpte values (using SetTime() or SetHours() ...)
        /// or the number of milliseconds (using SetMilliSeconds()) to convert; then you can call
        /// this to get the converted value.
        /// \note This is NOT const! May perform an internal conversion.
        unsigned long       GetSampleNumber();

        /// Loads the SMPTE with the given amount of samples.
        /// You can then call GetHour(), GetMinutes() etc.\ to perform a samples-to-smpte conversion or
        /// GetMilliseconds() to perform a sample-to-milliseconds conversion.
        void                SetSampleNumber (unsigned long n)
                                                            { sample_number = n; sample_number_dirty = true; }
        ///@}

        /// \name To get and set the milliseconds
        ///@{
        /// Performs a smpte-to-millisecond or sample-to-millisecond conversion.
        /// You must first load the SMPTE with the number of samples (using SetSampleNumber())
        /// or with smpte values (using SetTime() or SetHours() ...) to convert; then you can call
        /// this to get the converted value.
        /// \note This is NOT const! May perform an internal conversion.
        unsigned long       GetMilliSeconds ();
        /// Loads the SMPTE with the given time in milliseconds.
        /// You can then call GetSampleNumber() to perform a milliseconds-to-sample conversion or GetHours(),
        /// GetMinutes() etc.\ to perform a milliseconds-to-smpte conversion.
        void                SetMilliSeconds (unsigned long msecs);
        ///@}

        /// \name To get and set the smpte values
        /// These perform a samples-to-smpte or milliseconds-to-smpte conversion.\ You must first
        /// load the SMPTE with the number of samples (using SetSampleNumber()) or the number of
        /// milliseconds (using SetMilliSeconds()) to convert; then you can call these to get the
        /// converted smpte values.
        /// \note These are NOT const! May perform an internal conversion.
        ///@{
        /// Returns the smpte hours.
        unsigned char       GetHours();             // TODO: is this right? perhaps hours could be an int
        /// Returns the smpte minutes.
        unsigned char       GetMinutes();
        /// Returns the smpte seconds.
        unsigned char       GetSeconds();
        /// Returns the smpte frames.
        unsigned char       GetFrames();
        /// Returns the smpte subframes.
        unsigned char       GetSubFrames();
        /// Loads the SMPTE with the given time (in smpte format).
        /// You can then call GetSampleNumber() to perform a smpte-to-sample conversion or GetMilliSeconds() to
        /// perform a smpte-to-milliseconds conversion.
        void                SetTime (unsigned char h, unsigned char m, unsigned char s,
                                     unsigned char f = 0, unsigned char sf = 0);
        /// See SetTime(). This only affect the  smpte hours, leaving unchanged other parameters.
        void                SetHours (unsigned char h)      { hours = h; sample_number_dirty = true; }
        /// See SetTime(). This only affect the  smpte minutes, leaving unchanged other parameters.
        void                SetMinutes (unsigned char m)    { minutes = m; sample_number_dirty = true; }
        /// See SetTime(). This only affect the  smpte seconds, leaving unchanged other parameters.
        void                SetSeconds (unsigned char s)    { seconds = s; sample_number_dirty = true; }
        /// See SetTime(). This only affect the  smpte frames, leaving unchanged other parameters.
        void                SetFrames (unsigned char f)     { frames = f; sample_number_dirty = true; }
        /// See SetTime(). This only affect the  smpte subframes, leaving unchanged other parameters.
        void                SetSubFrames (unsigned char sf) { sub_frames = sf; sample_number_dirty = true; }
        ///@}

        /// \name To add, increment and decrement samples
        /// These functions add, increment or decrement the current sample number.
        ///@{
        /// Adds n samples.
        void                AddSamples (long n);
        /// Adds one sample.
        void                IncSamples()                    { AddSamples (1); }
        /// Subtracts one sample.
        void                DecSamples()                    { AddSamples (-1); }
        ///@}

        /// \name To add, increment and decrement smpte
        /// These functions add, increment or decrement smpte time parameters.
        ///@{
        /// Adds hours to smpte time.
        void                AddHours (signed char h);
        /// Adds minutes to smpte time.
        void                AddMinutes (signed char m);
        /// Adds seconds to smpte time.
        void                AddSeconds (signed char s);
        /// Adds frames to smpte time.
        void                AddFrames (signed char f);
        /// Adds subframes to smpte time.
        void                AddSubFrames (signed char sf);
        /// Adds one hour to smpte time.
        void                IncHours()                      { AddHours (1); }
        /// Adds one minute to smpte time.
        void                IncMinutes()                    { AddMinutes (1); }
        /// Adds one second to smpte time.
        void                IncSeconds()                    { AddSeconds (1); }
        /// Adds one frame to smpte time.
        void                IncFrames()                     { AddFrames (1); }
        /// Adds one subframe to smpte time.
        void                IncSubFrames()                  { AddSubFrames (1); }
        /// Subtract one hour from smpte time
        void                DecHours()                      { AddHours (-1); }
        /// Subtract one minute from smpte time
        void                DecMinutes()                    { AddMinutes (-1); }
        /// Subtract one second from smpte time
        void                DecSeconds()                    { AddSeconds (-1); }
        /// Subtract one frame from smpte time
        void                DecFrames()                     { AddFrames (-1); }
        /// Subtract one subframe from smpte time
        void                DecSubFrames()                  { AddSubFrames (-1); }
        ///@}

        /// \name The operators (these compare the current time)
        /// \note All these are NOT const! May perform an internal conversion
        ///@{

        /// Equal.
        bool                operator== (SMPTE &s)           { return Compare (s) == 0; }
        /// Not equal.
        bool                operator!= (SMPTE &s)           { return Compare (s) != 0; }
        /// Lesser.
        bool                operator< (SMPTE &s)            { return Compare (s) < 0; }
        /// Greater.
        bool                operator> (SMPTE &s)            { return Compare (s) > 0; }
        /// Lesser or equal.
        bool                operator<= (SMPTE &s)           { return Compare (s) <= 0; }
        /// Greater or equal.
        bool                operator>= (SMPTE &s)           { return Compare (s) >= 0; }
        /* These were eliminated: what should return if SMPTE rates are not equal?
        SMPTE&          operator+= (SMPTE &s)           { SetSampleNumber (sample_number + s.GetSampleNumber()); return *this; }
        SMPTE&          operator-= (SMPTE &s)           { SetSampleNumber (sample_number - s.GetSampleNumber()); return *this; }
        SMPTE&          operator+ (SMPTE &s);
        SMPTE&          operator- (SMPTE &s);
        */
        ///@}

    /* NEW */
        /// Sets an offset to be added to the current time to the given amount of samples.
        void                SetOffset (unsigned long n)     { sample_offset = n; }
        /// Sets an offset to be added to the current time to the given smpte time parameters.
        void                SetOffset (unsigned char h, unsigned char m, unsigned char s,
                                       unsigned char f = 0, unsigned char sf = 0);


    protected:
        /// Performs internal samples-to-smpte conversion.
        void                SampleToTime();
        /// Performs internal smpte-to-samples conversion.
        void                TimeToSample();
        /// Performs internal comparison between sample numbers.
        int                 Compare (SMPTE &s);

        /// \cond EXCLUDED
        // Max frames for every SMPTE_RATE.
        static const unsigned char  smpte_max_frames[];
        // Converts the SMPTE_RATE enum to a double frequency.
        static const double         smpte_rates[];
        // Converts the SMPTE_RATE enum to a long frequency times 100.
        static const long           smpte_rates_long[];
        // Converts the SAMPLE_RATE enum to a double frequency.
        static const double         sample_rates[];
        // Converts the SAMPLE_RATE enum to a long frequency times 10.
        static const long           sample_rates_long[];
        /// \endcond

    private:
        SMPTE_RATE          smpte_rate;         // The SMPTE rate
        SAMPLE_RATE         sample_rate;        // The sample rate
        unsigned long       sample_number;      // The sample number
    /* NEW */
        unsigned long       sample_offset;      // The initial time offset

        unsigned char       hours;              // SMPTE hours
        unsigned char       minutes;            // SMPTE minutes
        unsigned char       seconds;            // SMPTE seconds
        unsigned char       frames;             // SMPTE frames
        unsigned char       sub_frames;         // SMPTE subframes
        bool                sample_number_dirty;// True if we must perform a conversion

/* These were eliminated: what if SMPTE rates are not equal?
        friend SMPTE operator + ( SMPTE a, SMPTE b );
        friend SMPTE operator - ( SMPTE a, SMPTE b );
*/
};


#endif
