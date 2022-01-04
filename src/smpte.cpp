/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2021  Nicola Cassetta
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


#include "../include/smpte.h"



const unsigned char SMPTE::smpte_max_frames[] = {
    24, 25, 30, 30, 30, 30
};


const double SMPTE::smpte_rates[] = {
    24.0,
    25.0,
    30.0 / 1.001,
    30.0 / 1.001,
    30.0,
    30.0
};


const long SMPTE::smpte_rates_long[] = {
    2400,
    2500,
    (long) (3000.0 / 1.001),
    (long) (3000.0 / 1.001),
    3000,
    3000
};


const double SMPTE::sample_rates[] = {
    32000.0,
    44100.0 / 1.001,
    44100.0,
    48000.0 / 1.001,
    48000.0,
    48000.0 * 1.001
};


const long SMPTE::sample_rates_long[] = {
    320000,
    (long) (441000.0 / 1.001),
    441000,
    (long) (480000.0 / 1.001),
    480000,
    (long) (480000.0 * 1.001)
};



SMPTE::SMPTE(SMPTE_RATE smpte_rate_, SAMPLE_RATE sample_rate_) :
    smpte_rate(smpte_rate_),
    sample_rate(sample_rate_),
    sample_number(0),
    sample_offset(0),
    hours(0),
    minutes(0),
    seconds(0),
    frames(0),
    sub_frames(0),
    sample_number_dirty(false) {
}


unsigned long SMPTE::GetSampleNumber() {
    if (sample_number_dirty)
        TimeToSample();
    return sample_number;
}


unsigned long SMPTE::GetMilliSeconds () {
    if (sample_number_dirty)
        TimeToSample();
    return (unsigned long) (sample_number * 1000 / sample_rates[sample_rate] + 0.5);
}


void SMPTE::SetMilliSeconds (unsigned long msecs) {
    sample_number = (unsigned long) (msecs * sample_rates[sample_rate] / 1000);
    sample_number_dirty = true;
}


unsigned char SMPTE::GetHours() {
    if (sample_number_dirty)
        SampleToTime();
    return hours;
}


unsigned char SMPTE::GetMinutes() {
    if (sample_number_dirty)
        SampleToTime();
    return minutes;
}


unsigned char SMPTE::GetSeconds() {
    if (sample_number_dirty)
        SampleToTime();
    return seconds;
}


unsigned char SMPTE::GetFrames() {
    if (sample_number_dirty)
        SampleToTime();
    return frames;
}


unsigned char SMPTE::GetSubFrames() {
    if (sample_number_dirty)
        SampleToTime();
    return sub_frames;
}


void SMPTE::SetTime (unsigned char h, unsigned char m, unsigned char s, unsigned char f, unsigned char sf) {
    hours = h;
    minutes = m;
    seconds = s;
    frames = f;
    sub_frames = sf;
    sample_number_dirty = true;
}


void SMPTE::AddSamples (long n) {
    sample_number = GetSampleNumber() + n;
    sample_number_dirty = true;
}


void SMPTE::AddHours(signed char h) {
    AddSamples( sample_rates_long[sample_rate]  // samples per second times 10
                 * h *                          // number of hours
                 (  60		                    // seconds per minute
                    * 60		                // minutes per hour
                    / 10 )		                // compensate for freq*10
      );
  }


void SMPTE::AddMinutes(signed char m) {
    AddSamples( sample_rates_long[sample_rate]  // samples per second times 10
                * m *                           // number of minutes
                ( 60 		                    // seconds per minute
                  / 10 )	                    // compensate for freq*10
    );
}


void SMPTE::AddSeconds(signed char s) {
    AddSamples( sample_rates_long[sample_rate]  // samples per second times 10
                * s		                        // number of seconds
                / 10		                    // compensate for freq*10
    );
}


void SMPTE::AddFrames(signed char f) {
    AddSamples( sample_rates_long[sample_rate]  // samples per second times 10
                * f 			                // number of frames
                * 10			                // times 10
                / smpte_rates_long[smpte_rate]  // divide by smpte rate (frames per second) times 100
    );
}


void SMPTE::AddSubFrames(signed char sf) {
    AddSamples( sample_rates_long[sample_rate]	// samples per second times 10
                * sf			                // number of sub frames
                / smpte_rates_long[smpte_rate]  // divide by smpte rate (frames per second) times 100
                / 10			                // divide by 10 to get hundredths of a frame
    );
}


void SMPTE::SetOffset (unsigned char h, unsigned char m, unsigned char s,
                       unsigned char f, unsigned char sf) {
    // resets the offset
    sample_offset = 0;
    // converts the SMPTE values to samples
    SetTime(h, m, s, f, sf);
    TimeToSample();
    // copies the corresponding samples to the offset
    sample_offset = sample_number;
    // resets the sample number
    sample_number = 0;
}


void SMPTE::SampleToTime() {

    // make a temporary copy of the sample number
    unsigned long tmp_sample = sample_number + sample_offset;

    // keep track of the actual rates in use in doubles.
    double the_smpte_rate = smpte_rates[ smpte_rate ];
    double the_sample_rate = sample_rates[ sample_rate ];

    // keep track of the maximum frame number for this smpte format.
    unsigned char max_frame = smpte_max_frames[ smpte_rate ];

    // Calculate the number of samples per frame
    double samples_per_frame = sample_rates[sample_rate] / smpte_rates[smpte_rate];

    // If the smpte rate is a drop frame type, calculate the number
    // of frames that must be dropped.
    if(smpte_rate == SMPTE_RATE_30DF || smpte_rate == SMPTE_RATE_2997DF) {

        // Calculate number of minutes that have gone by
        // short num_minutes = (short)((double)tmp_sample/(smpte_sample_rates[sample_rate]))/60;
        int num_minutes = tmp_sample/(48000 * 60);

        // Calculate the number of tens of minutes that have gone by, including minute 00
        int ten_minutes = num_minutes / 10;

        // Calculate the number of frames that are dropped by this time.
        int drops = (num_minutes - ten_minutes) * 2;

        // Offset the tmp_sample number by this amount of frames.
        tmp_sample += (unsigned long)(drops * samples_per_frame);
    }

    // Calculate the time in sub frames, frames, seconds, minutes, hours
    unsigned long rounded_sub_frames= (unsigned long)((tmp_sample*the_smpte_rate * 100)/the_sample_rate +.5);

    sub_frames = (unsigned char) ((rounded_sub_frames) % 100);
    frames 	   = (unsigned char) ((rounded_sub_frames / 100) % max_frame);
    seconds	   = (unsigned char) ((rounded_sub_frames / (100L * max_frame)) % 60 );
    minutes	   = (unsigned char) ((rounded_sub_frames / (100L * 60L * max_frame)) % 60 );
    hours	   = (unsigned char) ((rounded_sub_frames / (100L * 60L * 24L *max_frame)) % 24 );

    sample_number_dirty = false;
}


void SMPTE::TimeToSample() {

    // keep track of the actual rates in use in doubles.
    double the_smpte_rate = smpte_rates[smpte_rate];
    double the_sample_rate = sample_rates[sample_rate];

    // optimize a coupla similiar double divides by calculating it once
    double samples_per_frame = the_sample_rate / the_smpte_rate;

    // calculate the sample number
    double tmp_sample = (double) (
      (	(hours * the_sample_rate * (60 * 60))
      +	(minutes * the_sample_rate * 60)
      +	(seconds * the_sample_rate)
      + (frames * samples_per_frame)
      +	(sub_frames * samples_per_frame * (1.0/100.0) ) +.5)
      );

    // Now compensate for Drop Frame mode if we are in drop frame mode.
    if(smpte_rate == SMPTE_RATE_30DF || smpte_rate==SMPTE_RATE_2997DF) {

        // Calculate number of minutes that have gone by
        int num_minutes = (int)((double)tmp_sample / (sample_rates[sample_rate] * 60));

        // Calculate the number of tens of minutes that have gone by, including minute 00
        int ten_minutes = num_minutes / 10;

        // Calculate the number of frames that are dropped by this time.
        int drops = (num_minutes - ten_minutes) * 2;

        // Offset the tmp_sample number by this amount of frames.
        tmp_sample -= drops * samples_per_frame;
    }

    // save the calculated sample number in self.
    sample_number = (unsigned long)tmp_sample + sample_offset;

    sample_number_dirty = false;
}


int	SMPTE::Compare(SMPTE &s) {
    unsigned long a = GetSampleNumber();
    unsigned long b = s.GetSampleNumber();

    if(a < b)
       return -1;
    if(a > b)
       return 1;
    return 0;
}


