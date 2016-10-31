/*
 *  libjdkmidi-2004 C++ Class Library for MIDI
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
**	Copyright 1986 to 1998 By J.D. Koftinoff Software, Ltd.
**
**	All rights reserved.
**
**	No one may duplicate this source code in any form for any reason
**	without the written permission given by J.D. Koftinoff Software, Ltd.
**
*/
//
// Copyright (C) 2010 V.R.Madgazin
// www.vmgames.com vrm@vmgames.com
//


#ifndef _JDKMIDI_SMPTE_H
#define _JDKMIDI_SMPTE_H


/// \defgroup rates "The smpte and sample rates"
/// These are the allowed smpte rates and sample rates allowed for SMPTE class
//@{
/// The smpte rates
enum SMPTE_RATE
{
    SMPTE_RATE_24 = 0,          ///< 24 frames/sec
    SMPTE_RATE_25,              ///< 25 frames/sec
    SMPTE_RATE_2997,            ///< 29.97 frames/sec
    SMPTE_RATE_2997DF,          ///< 29.97 frames/sec drop
    SMPTE_RATE_30,              ///< 30 frames/sec
    SMPTE_RATE_30DF             ///< 30 frames/sec drop
};


/// The sample rates
enum SAMPLE_RATE
{
    SAMPLE_32000 = 0,           ///< 32000 samples/sec
    SAMPLE_44056,               ///< 44056 samples/sec
    SAMPLE_44100,               ///< 44100 samples/sec
    SAMPLE_47952,               ///< 47952 samples/sec
    SAMPLE_48000,               ///< 48000 samples/sec
    SAMPLE_48048                ///< 48048 samples/sec
};
//@}





///
/// This class performs conversions between number of samples, milliseconds and smpte format
/// (hours::minutes::seconds::frames::subframes).
/// You can choose between several smpte formats and sample rates
/// (see \ref rates "SMPTE and sample rates"). Moreover you can set an initial time offset to
/// be added to every value passed to the SMPTE.
/// \note Many get functions are NOT const because they can perform internal conversions
class  SMPTE {
    public:

        /// The constructor sets the SMPTE rate to SMPTE_30, the sample rate to SAMPLE_48000
        /// and the offsey to 0.
        /// \note Tere's no need for explicit copy ctor and operator=
                            SMPTE (SMPTE_RATE smpte_rate = SMPTE_RATE_30,
                                   SAMPLE_RATE sample_rate = SAMPLE_48000);

        /// Sets the smpte rate. See \ref rates "SMPTE and sample rates" for avalaible smpte rates
        void                SetSMPTERate (SMPTE_RATE r)     { smpte_rate = r; sample_number_dirty = true; }

        /// Returns the smpte rate
        SMPTE_RATE          GetSMPTERate() const            { return smpte_rate; }

        /// Sets the sample rate. See \ref rates "SMPTE and sample rates" for avalaible sample rates
        void                SetSampleRate (SAMPLE_RATE r)   { sample_rate = r; sample_number_dirty = true; }

        /// Returns the sample rate
        SAMPLE_RATE         GetSampleRate() const           { return sample_rate; }

        /// To perform a samples-to-smpte or samples-to-milliseconds conversion.
        /// You must first load the SMPTE with the number of samples to convert using this;
        /// then you can call GetMilliseconds(), GetHours(), GetMinutes() etc. to get the
        /// converted values.
        void                SetSampleNumber (unsigned long n)
                                                            { sample_number = n; sample_number_dirty = true; }

        /// To perform a smpte-to-samples or milliseconds-to-samples conversion.
        /// You must first load the SMPTE with the smpte values (using SetTime() or SetHours() ...)
        /// or the number of milliseconds (using SetMilliseconds()) to convert; then you can call
        /// this to get the converted value.
        /// \note This is NOT const! May perform an internal conversion.
        unsigned long       GetSampleNumber();

        /// To perform a smpte-to-samples or smpte-to-milliseconds conversion.
        /// You must first load the SMPTE with the number of of hours, minutes, seconds, frames and
        /// subframes to convert using this (or other functions setting individual items); then you
        /// can call GetSampleNumber() or GetMilliseconds() to get the converted values.
        //@{
        void                SetTime (unsigned char h, unsigned char m, unsigned char s,
                                     unsigned char f = 0, unsigned char sf = 0);
        void                SetHours (unsigned char h)      { hours = h; sample_number_dirty = true; }
        void                SetMinutes (unsigned char m)    { minutes = m; sample_number_dirty = true; }
        void                SetSeconds (unsigned char s)    { seconds = s; sample_number_dirty = true; }
        void                SetFrames (unsigned char f)     { frames = f; sample_number_dirty = true; }
        void                SetSubFrames (unsigned char sf) { sub_frames = sf; sample_number_dirty = true; }
        //@}

        /// To perform a samples-to-smpte or milliseconds-to-smpte conversion.
        /// You must first load the SMPTE with the number of samples (using SetSampleNumber())
        /// or the number of milliseconds (using SetMilliseconds()) to convert; then you can call
        /// this to get the converted smpte values.
        /// \note These are NOT const! May perform an internal conversion.
        //@{
        unsigned char       GetHours();
        unsigned char       GetMinutes();
        unsigned char       GetSeconds();
        unsigned char       GetFrames();
        unsigned char       GetSubFrames();
        //@}

        /// To perform a millisecond-to-samples or millisecond-to-smpte conversion.
        /// You must first load the SMPTE with the number of milliseconds to convert using this;
        /// then you can call GetSampleNumber() or GetHours(), GetMinutes() etc. to get the
        /// converted values.
        void                SetMilliSeconds (unsigned long msecs);

        /// To perform a smpte-to-millisecond or sample-to-millisecond conversion.
        /// You must first load the SMPTE with the number of samples (using SetSampleNumber())
        /// or with smpte values (using SetTime() or SetHours() ...) to convert; then you can call
        /// this to get the converted value.
        /// \note This is NOT const! May perform an internal conversion.
        unsigned long       GetMilliSeconds ();

        /// \name To add, increment and decrement samples.
        /// These functions add, increment or decrement the current sample number./ You can use them
        /// instead of SetSampleNunber() to perform a samples-to-smpte conversion
        //@{
        void                AddSamples (long n);
        void                IncSamples()                    { AddSamples (1); }
        void                DecSamples()                    { AddSamples (-1); }
        //@}

        /// \name To add, increment and decrement smpte
        /// These functions add, increment or decrement smpte time parameters./ You can use them instead of
        /// SetTime() to perform a smpte-to-samples conversion
        //@{
        void                AddHours (char h);
        void                AddMinutes (char m);
        void                AddSeconds (char s);
        void                AddFrames (char f);
        void                AddSubFrames (char sf);
        void                IncHours()                      { AddHours (1); }
        void                IncMinutes()                    { AddMinutes (1); }
        void                IncSeconds()                    { AddSeconds (1); }
        void                IncFrames()                     { AddFrames (1); }
        void                IncSubFrames()                  { AddSubFrames (1); }
        void                DecHours()                      { AddHours (-1); }
        void                DecMinutes()                    { AddMinutes (-1); }
        void                DecSeconds()                    { AddSeconds (-1); }
        void                DecFrames()                     { AddFrames (-1); }
        void                DecSubFrames()                  { AddSubFrames (-1); }
        //@}

        /// \name The operators (these compare the current time).
        /// \note All these are NOT const! May perform an internal conversion
        //@{

        bool                operator== (SMPTE &s)           { return Compare (s) == 0; }
        bool                operator!= (SMPTE &s)           { return Compare (s) != 0; }
        bool                operator< (SMPTE &s)            { return Compare (s) < 0; }
        bool                operator> (SMPTE &s)            { return Compare (s) > 0; }
        bool                operator<= (SMPTE &s)           { return Compare (s) <= 0; }
        bool                operator>= (SMPTE &s)           { return Compare (s) >= 0; }
        /* These were eliminated: what should return if SMPTE rates are not equal?
        SMPTE&          operator+= (SMPTE &s)           { SetSampleNumber (sample_number + s.GetSampleNumber()); return *this; }
        SMPTE&          operator-= (SMPTE &s)           { SetSampleNumber (sample_number - s.GetSampleNumber()); return *this; }
        SMPTE&          operator+ (SMPTE &s);
        SMPTE&          operator- (SMPTE &s);
        */
        //@}

    /* NEW */
        /// These set an offset to be added to the current time.
        void                SetOffset (unsigned long n)     { sample_offset = n; }
        void                SetOffset (unsigned char h, unsigned char m, unsigned char s,
                                       unsigned char f = 0, unsigned char sf = 0);


    protected:
        /// Performs internal samples-to-smpte conversion.
        void                SampleToTime();
        /// Performs internal smpte-to-samples conversion.
        void                TimeToSample();
        /// Performs internal comparison between sample numbers.
        int                 Compare (SMPTE &s);


        /*

        double              GetSMPTERateFrequency()     { return smpte_rates[(int)smpte_rate]; }

        long                GetSMPTERateFrequencyLong() { return smpte_rates_long[(int)smpte_rate]; }

        double              GetSampleRateFrequency()    { return sample_rates[(int)sample_rate]; }

        long                GetSampleRateFrequencyLong(){ return sample_rates_long[(int)sample_rate]; }

        */



        /// Max frames for every SMPTE_RATE.
        static const unsigned char  smpte_max_frames[];
        /// Converts the SMPTE_RATE enum to a double frequency.
        static const double         smpte_rates[];
        /// Converts the SMPTE_RATE enum to a long frequency times 100.
        static const long           smpte_rates_long[];
        /// Converts the SAMPLE_RATE enum to a double frequency.
        static const double         sample_rates[];
        /// Converts the SAMPLE_RATE enum to a long frequency times 10.
        static const long           sample_rates_long[];

    private:
        SMPTE_RATE          smpte_rate;         ///< The SMPTE rate
        SAMPLE_RATE         sample_rate;        ///< The sample rate
        unsigned long       sample_number;      ///< The sample number
    /* NEW */
        unsigned long       sample_offset;      ///< The initial time offset

        unsigned char       hours;              ///< SMPTE hours
        unsigned char       minutes;            ///< SMPTE minutes
        unsigned char       seconds;            ///< SMPTE seconds
        unsigned char       frames;             ///< SMPTE frames
        unsigned char       sub_frames;         ///< SMPTE subframes
        bool                sample_number_dirty;///< True if we must perform a conversion

/* These were eliminated: what if SMPTE rates are not equal?
        friend SMPTE operator + ( SMPTE a, SMPTE b );
        friend SMPTE operator - ( SMPTE a, SMPTE b );
*/
};


#endif



