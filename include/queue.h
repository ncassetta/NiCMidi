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
** Copyright 2016 By N. Cassetta
** myjdkmidi library
**
** CHECKED with jdksmidi. CHANGES:
**  - moved CanPut() and CanGet() implementations here
*/


#ifndef _JDKMIDI_QUEUE_H
#define _JDKMIDI_QUEUE_H

#include "msg.h"
#include "sysex.h"


// TODO: perhaps we can drop this file


  class MIDIQueue
    {
    public:
      MIDIQueue( int num_msgs );
      virtual ~MIDIQueue();

      void Clear();

      bool CanPut() const
        {
          return next_out != ((next_in+1)%bufsize);
        }

      bool CanGet() const
        {
          return next_in != next_out;
        }

      bool IsFull() const
        {
          return !CanPut();
        }


/* Put assign messages by operator= so sysex are copied into queue messages and eventual old sysex
   pointers are freed
*/

      void Put( const MIDITimedMessage &msg )
        {
          buf[next_in] = msg;

          next_in = (next_in+1)%bufsize;
        }

      MIDITimedMessage Get() const
        {
          return MIDITimedMessage( buf[next_out] );
        }

      void Next()
        {
          next_out = (next_out+1) % bufsize;
        }

      const MIDITimedMessage *Peek()  const
        {
          return &buf[next_out];
        }

    protected:
      MIDITimedMessage *buf;
      int bufsize;
      volatile int next_in;
      volatile int next_out;
    };


#endif
