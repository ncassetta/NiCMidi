/*
 *   NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2004  J.D. Koftinoff Software, Ltd.
 *   www.jdkoftinoff.com jeffk@jdkoftinoff.com
 *   Copyright (C) 2010 V.R.Madgazin
 *   www.vmgames.com vrm@vmgames.com
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
/// Contains the definition of the class MIDISystemExclusive.


#ifndef JDKSMIDI_SYSEX_H
#define JDKSMIDI_SYSEX_H

#include "midi.h"
#include <vector>
#include <cstring>          // for memcmp()


///
/// Stores a buffer of MIDI data bytes in a std::vector, plus a byte for the checksum.
/// It is used by the MIDIMessage class for keeping an arbitrary amount of data (tipically the data
/// attached to a MIDI sysex message, but also the text meta messages and some other type utilizes it).
///
class  MIDISystemExclusive {
    public:
        /// Creates a new object initially empty. If you know the length of the data it will contain
        /// you can specify it here.
                                    MIDISystemExclusive(unsigned int len = 0);
        /// The copy constructor.
                                    MIDISystemExclusive(const MIDISystemExclusive &e);
        /// Creates a new object from the given buffer of bytes. It allocates the appropriate memory amount and
        /// copies the buffer data, so every instance has its own buffer.
        /// \param buf the address of our data buffer
        /// \param len the length of the buffer
                                    MIDISystemExclusive(const unsigned char *buf, unsigned int len);
        /// The destructor.
        virtual	                    ~MIDISystemExclusive() {}
        /// The assignment operator. It allocates the appropriate memory amount and copies the buffer data, so every
        /// instance has its own buffer.
        MIDISystemExclusive&        operator= (const MIDISystemExclusive& se);
        /// The equal operator compares the data buffer and the checksum.
        bool                        operator== (const MIDISystemExclusive &se) const;

        /// Resets the buffer to 0 length data and the checksum to 0.
        void	                    Clear()				        { buffer.clear(); chk_sum = 0;	}
        /// Resets the checksum to 0.
        void	                    ClearChecksum()		        { chk_sum = 0; }
        /// Returns the checksum.
        unsigned char	            GetChecksum() const         { return (unsigned char)(chk_sum & 0x7f); }
        /// Returns the number of data bytes stored in the buffer.
        int		                    GetLength() const           { return buffer.size(); }
        /// Returns the i-th byte in the buffer.
        unsigned char	            GetData(int i) const        { return buffer[i]; }
        /// Returns a pointer to the data buffer.
        //unsigned char*          GetBuffer()                 { return buffer; }
        const unsigned char*        GetBuffer() const           { return buffer.data(); }
        /// Returns **true** if the buffer contains a GM Reset sysex message.
        bool                        IsGMReset() const;
        /// Returns **true** if the buffer contains a GS Reset sysex message.
        bool                        IsGSReset() const;
        /// Returns **true** if the buffer contains a XG Reset sysex message.
        bool                        IsXGReset() const;
        /// Appends a byte to the buffer, without adding it to checksum.
        void	                    PutSysByte(unsigned char b) { buffer.push_back(b); }
        /// Appends a byte to the buffer, adding it to checksum.
        void	                    PutByte(unsigned char b)    { PutSysByte(b); chk_sum += b; }
        /// Appends a System exclusive Start byte (0xF0) to the buffer, without affecting the checksum.
        void	                    PutEXC()                    { PutSysByte(SYSEX_START); }
        /// Appends a System exclusive End byte (0xF7) to the buffer, without affecting the checksum.
        void	                    PutEOX()	                { PutSysByte(SYSEX_END); }
        /// Appends two bytes, containing the low (1st) and the high nibble of b. Adds them to the checksum.
        void	                    PutNibblizedByteLH(unsigned char b)
                                    { PutByte((unsigned char)(b & 0xf)); PutByte((unsigned char)(b >> 4)); }
        /// Appends two bytes, containing the high (1st) and the low nibble of b. Adds them to the checksum.
        void	                    PutNibblizedByteHL(unsigned char b)
                                    { PutByte((unsigned char)(b >> 4)); PutByte((unsigned char)(b & 0xf)); }
        /// Appends the checksum to the buffer.
        void	                    PutChecksum()               { PutByte((unsigned char)(chk_sum & 0x7f)); }

    protected:
        /// \cond EXCLUDED
        std::vector<unsigned char>  buffer;
        unsigned char               chk_sum;

        static const unsigned char  GMReset_data[];
        static const unsigned char  GSReset_data[];
        static const unsigned char  XGReset_data[];
        static const int            GMReset_len = 6;
        static const int            GSReset_len = 11;
        static const int            XGReset_len = 9;
        /// \endcond
};



#endif
