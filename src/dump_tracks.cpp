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

// FILE AGGIORNATO IN AutoComp 0.10

#include "../include/world.h"
#include "../include/track.h"
#include "../include/multitrack.h"


void DumpMIDITimedBigMessage ( MIDITimedBigMessage *msg )
{
  if ( msg )
  {
    char msgbuf[1024];

    printf ( "%8ld : %s\n", msg->GetTime(), msg->MsgToText ( msgbuf ) );
    if ( msg->IsSysEx() )
    {
      printf ( "\tSYSEX length: %d\n", msg->GetSysEx()->GetLength() );
    }

  }

}

void DumpMIDITrack ( MIDITrack *t )
{
  MIDITimedBigMessage *msg;

  printf ( "Track dump\n");
  for ( int i=0; i<t->GetNumEvents(); ++i )
  {
    msg = t->GetEventAddress ( i );
    printf("#%4d", i + 1);
    DumpMIDITimedBigMessage ( msg );

  }

}

void DumpAllTracks ( MIDIMultiTrack *mlt )
{

  printf ( "DUMP OF MIDI MULTITRACK\n");
  printf ( "Clocks per beat: %d\n\n", mlt->GetClksPerBeat() );

  for ( int i=0; i<mlt->GetNumTracks(); ++i )
  {
    if ( mlt->GetTrack ( i )->GetNumEvents() > 0 )
    {
      printf ( "DUMP OF TRACK #%2d:\n", i );
      for (int j=0; j<mlt->GetTrack( i )->GetNumEvents(); j++)
        DumpMIDITimedBigMessage( mlt->GetTrack ( i )->GetEventAddress(j) );
      printf ( "\n" );
    }

  }

}


void DumpMIDIMultiTrack ( MIDIMultiTrack *mlt )
{
  MIDIMultiTrackIterator i ( mlt );
  MIDITimedBigMessage *msg;
  int trk_num;

  fprintf ( stdout, "DUMP OF MIDI MULTITRACK\n");
  fprintf ( stdout , "Clocks per beat: %d\n\n", mlt->GetClksPerBeat() );

  i.GoToTime ( 0 );

  do
  {
    if ( i.GetCurEvent ( &trk_num, &msg )  )
    {
      fprintf ( stdout, "#%2d - ", trk_num );
      DumpMIDITimedBigMessage ( msg );
    }
  }
  while ( i.GoToNextEvent() );

}
