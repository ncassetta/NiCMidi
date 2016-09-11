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
// Copyright (C) 2010 V.R.Madgazin
// www.vmgames.com vrm@vmgames.com
//

#ifndef JDKSMIDI_SYSEX_H
#define JDKSMIDI_SYSEX_H

#include "midi.h"
#include <cstring>                  // for memcmp()

class  MIDISystemExclusive {
    public:
                                MIDISystemExclusive(int size = 384);
                                MIDISystemExclusive(const MIDISystemExclusive &e);
                                MIDISystemExclusive(const unsigned char *buf, int max_len_, int cur_len_);
        virtual	               ~MIDISystemExclusive();

        MIDISystemExclusive&    operator= (const MIDISystemExclusive& se);
        bool                    operator== (const MIDISystemExclusive &se) const;

        void	                Clear()				        { cur_len = 0; chk_sum = 0;	}
        void	                ClearChecksum()		        { chk_sum = 0; }

        void	                PutSysByte(unsigned char b)	// does not add to chksum
                                    { if (cur_len < max_len) buffer[cur_len++] = b; }
        void	                PutByte(unsigned char b)    { PutSysByte(b); chk_sum += b; }
        void	                PutEXC()                    { PutSysByte(SYSEX_START); }
        void	                PutEOX()	                { PutSysByte(SYSEX_END); }

        void	                PutNibblizedByte(unsigned char b)       // low nibble first
                                    { PutByte((unsigned char)(b & 0xf)); PutByte((unsigned char)(b >> 4)); }
        void	                PutNibblizedByte2(unsigned char b)      // high nibble first
                                    { PutByte((unsigned char)(b >> 4)); PutByte((unsigned char)(b & 0xf)); }
        void	                PutChecksum()               { PutByte((unsigned char)(chk_sum & 0x7f)); }

        unsigned char	        GetChecksum() const         { return (unsigned char)(chk_sum & 0x7f); }
        int		                GetLength() const           { return cur_len; }
        unsigned char	        GetData(int i) const        { return buffer[i]; }
        bool	                IsFull() const              { return cur_len >= max_len; }
        unsigned char*          GetBuffer()                 { return buffer; }
        const unsigned char*    GetBuffer() const           { return buffer; }
        bool                    IsGMReset() const;
        bool                    IsGSReset() const;
        bool                    IsXGReset() const;

    private:
        unsigned char*          buffer;
        int                     max_len;
        int                     cur_len;
        unsigned char           chk_sum;

        static const unsigned char GMReset_data[];
        static const unsigned char GSReset_data[];
        static const unsigned char XGReset_data[];
        static const int           GMReset_len = 6;
        static const int           GSReset_len = 11;
        static const int           XGReset_len = 9;
};



#endif
