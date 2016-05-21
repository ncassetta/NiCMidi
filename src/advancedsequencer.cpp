#include "../include/world.h"
#include "../include/advancedsequencer.h"
#include "../include/fileread.h"
#include "../include/filereadmultitrack.h"
#include "../include/driverwin32.h"

#include <iostream>


AdvancedSequencer::AdvancedSequencer(MIDISequencerGUINotifier *n) :
    notifier( n ),
    tracks ( new MIDIMultiTrack ( 17 ) ),
    seq ( new MIDISequencer ( tracks, notifier ) ),
    mgr ( new MIDIManager ( notifier, seq ) ),

    thru_processor ( 2 ),
    thru_transposer(),
    thru_rechannelizer(),

    num_measures ( 0 ),
    repeat_start_measure ( 0 ),
    repeat_end_measure ( 0 ),
    repeat_play_mode ( false ),
    file_loaded ( false ),

    in_port (),
    out_port (),

    ctor_type ( CTOR_1 )    // remembers what objects are owned

// chain_mode ( false ) OLD (see header)

{
/* NOTE BY NC: currently we open midi (and start timer) in the ctor and close it in the dtor:
 * is this right? perhaps we should open/close only in Play/Stop (but what if thru is enabled?)
 * What is better?
 */
    //OpenMIDI(in_port, out_port);
    SetClksPerBeat ( DEFAULT_CLK_PER_BEAT );
    mgr->SetOpenPolicy(MIDIManager::EXT_OPEN);
}


AdvancedSequencer::AdvancedSequencer(MIDIMultiTrack* mlt, MIDISequencerGUINotifier *n) :
    notifier( n ),
    tracks ( mlt ),
    seq ( new MIDISequencer ( tracks, notifier ) ),
    mgr ( new MIDIManager ( notifier, seq ) ),

    thru_processor ( 2 ),
    thru_transposer(),
    thru_rechannelizer(),

    num_measures ( 0 ),
    repeat_start_measure ( 0 ),
    repeat_end_measure ( 0 ),
    repeat_play_mode ( false ),
    file_loaded ( false ),

    in_port (),
    out_port (),

    ctor_type ( CTOR_2 )    // remembers what objects are owned

{
/* NOTE BY NC: currently we open midi (and start timer) in the ctor and close it in the dtor:
 * is this right? perhaps we should open/close only in Play/Stop (but what if thru is enabled?)
 * What is better?
 */
    mgr->SetOpenPolicy(MIDIManager::EXT_OPEN);
}


AdvancedSequencer::AdvancedSequencer(MIDIManager *mg) :
    notifier( mg->GetSeq()->GetState()->notifier ),
    tracks ( ( MIDIMultiTrack * ) ( mg->GetSeq()->GetState()->multitrack ) ),
    seq ( mgr->GetSeq() ),
    mgr ( mg ),

    thru_processor ( 2 ),
    thru_transposer(),
    thru_rechannelizer(),

    num_measures ( 0 ),
    repeat_start_measure ( 0 ),
    repeat_end_measure ( 0 ),
    repeat_play_mode ( false ),
    file_loaded ( false ),

    in_port(),
    out_port(),

    ctor_type ( CTOR_3 )    // remembers what objects are owned

{
/* NOTE BY NC: currently we open midi (and start timer) in the ctor and close it in the dtor:
 * is this right? perhaps we should open/close only in Play/Stop (but what if thru is enabled?)
 * What is better?
 */
    mgr->SetOpenPolicy(MIDIManager::EXT_OPEN);
}


AdvancedSequencer::~AdvancedSequencer()
{
    Stop();
    //CloseMIDI();
    if ( ctor_type != CTOR_3)
    {
        delete mgr;
        delete seq;
    }
    if ( ctor_type == CTOR_1 )
    {
        delete tracks;
    }
}


void AdvancedSequencer::SetOutputPort( int p)
{
    Stop();
    //CloseMIDI();
    out_port = p;
    //OpenMIDI(in_port, out_port);
}


void AdvancedSequencer::SetInputPort( int p)
{
    Stop();
    //CloseMIDI();
    in_port = p;
    //OpenMIDI(in_port, out_port);
}


void AdvancedSequencer::SetMIDIThruChannel ( int chan )
{
    thru_rechannelizer.SetAllRechan ( chan );
    mgr->AllNotesOff();
}


void AdvancedSequencer::SetMIDIThruTranspose ( int val )
{
    thru_transposer.SetAllTranspose ( val );
    mgr->AllNotesOff();
}


bool AdvancedSequencer::Load ( const char *fname )
{
    char realname[1024];
    strcpy ( realname, fname );
    int orignamelen = ( int ) strlen ( fname );
    // chain_mode = false; OLD (see header)

    if ( orignamelen > 0 )
    {
        if ( realname[orignamelen-1] == '+' )
        {
            realname[orignamelen-1] = 0;
            // chain_mode = true; OLD (see header)
        }
    }

    Stop();
    mgr->AllNotesOff();
    tracks->Clear();

    MIDIFileReadStreamFile mfreader_stream ( realname );
    MIDIFileReadMultiTrack track_loader ( tracks );
    MIDIFileRead reader ( &mfreader_stream, &track_loader );
    if ( reader.Parse() )
    {
        file_loaded = true;
        Reset();
        // GoToMeasure ( 0 ); OLD: it used warp_positions, not even initialized!!! However,
        // this is already done by Reset();
        ExtractWarpPositions();
    }

    else
    {
        file_loaded = false;
    }

    return file_loaded;
}


void AdvancedSequencer::UnLoad()    /* NEW BY NC */
{
    Reset();
    tracks->Clear();
    warp_positions.clear();
    num_measures = 0;
    file_loaded = false;
    SetClksPerBeat(DEFAULT_CLK_PER_BEAT);
}


void AdvancedSequencer::Reset()
{
    Stop();
    MIDITimer::Wait(500);    // pauses for 0.5 sec (TROUBLE WITHOUT THIS!!!! I DON'T KNOW WHY)
    UnmuteAllTracks();
    UnSoloTrack();
    SetTempoScale ( 1.00 );
    SetRepeatPlay(false, 0, 0 );
    seq->Reset();
    seq->GoToZero();
    mgr->Reset();    // clear queues
}


void AdvancedSequencer::GoToZero() {
    if ( !file_loaded )
    {
        return;
    }

    Stop();     // always stops if playing
    seq->GoToZero();
}


void AdvancedSequencer::GoToTime (MIDIClockTime t) {
    if ( !file_loaded )
    {
        return;
    }

    // figure out which warp item we use
    // try warp to the last warp point BEFORE the
    // requested measure

    unsigned int warp_to_item = 0;
    for ( ; warp_to_item < warp_positions.size(); warp_to_item++ )
    {
        if ( warp_positions[warp_to_item].cur_clock > t )
        {
            break;
        }
    }
    if ( warp_to_item == warp_positions.size() && warp_to_item != 0 )
    {
        warp_to_item--;
    }

    if (mgr->IsSeqPlay())
    {
        Stop();
        seq->SetState (&warp_positions[warp_to_item]);
        seq->GoToTime (t);
        Play();
    }
    else
    {
        seq->SetState (&warp_positions[warp_to_item]);
        seq->GoToTime (t);
        for (int i = 0; i < seq->GetNumTracks(); ++i)
        {
            seq->GetTrackState ( i )->note_matrix.Clear();
        }
    }
}


void AdvancedSequencer::GoToMeasure ( int measure, int beat )
{
    if ( !file_loaded )
    {
        return;
    }

    // figure out which warp item we use
    // try warp to the last warp point BEFORE the
    // requested measure
    unsigned int warp_to_item = measure / MEASURES_PER_WARP;

    if ( warp_to_item >= warp_positions.size() )
        warp_to_item = warp_positions.size() - 1;

    if ( mgr->IsSeqPlay() )
    {
        Stop();
        seq->SetState ( &warp_positions[warp_to_item] );
        seq->GoToMeasure ( measure, beat );
        Play();
    }

    else
    {
        seq->SetState ( &warp_positions[warp_to_item] );
        seq->GoToMeasure ( measure, beat );
        for ( int i = 0; i < seq->GetNumTracks(); ++i )
        {
            seq->GetTrackState ( i )->note_matrix.Clear();
        }
    }
}


void AdvancedSequencer::Play ()
{
    if ( !file_loaded )
    {
        return;
    }

    Stop();
    if ( repeat_play_mode )
    {
        GoToMeasure ( repeat_start_measure );
    }

    CatchEventsBefore();
    // this intercepts any CC, SYSEX and TEMPO messages and send them to the out port
    // allowing to start with correct values; we could incorporate this in the
    // sequencer state, but it would track even CC (not difficult) and SYSEX messages

    mgr->SeqPlay();
}


void AdvancedSequencer::Stop()
{
    if ( !file_loaded )
    {
        return;
    }

    if ( !mgr->IsSeqStop() )
    {
        mgr->SeqStop();
        mgr->AllNotesOff();
        mgr->Reset();
        GoToMeasure(seq->GetState()->cur_measure, seq->GetState()->cur_beat);
        // stops on a beat (and clear midi matrix)
    }
}


/* NEW BY NC */
void AdvancedSequencer::OutputMessage( MIDITimedBigMessage& msg ) {
    bool was_open = true;
    MIDIOutDriver* driver = mgr->GetDriver(0);
    if (!driver->IsPortOpen()) {
        was_open = false;
        driver->OpenPort();
    }
    driver->OutputMessage( msg );
    if (!was_open)
        mgr->GetDriver(0)->ClosePort();
}


void AdvancedSequencer::SetRepeatPlay ( bool enable, int start_measure, int end_measure )
{
    if ( !file_loaded )
    {
        return;
    }

    if ( start_measure < end_measure && start_measure >= 0 )
    {
        repeat_play_mode = enable;
        repeat_start_measure = start_measure;
        repeat_end_measure = end_measure;
    }

    else
    {
        repeat_play_mode = false;
    }

    mgr->SetRepeatPlay (
        repeat_play_mode,
        repeat_start_measure,
        repeat_end_measure
    );
    /* NEW */
    if (IsPlay())
    {
        Play();     // restarts from start_measure
    }
}

/* NOTE BY NC: soloing and muting has been enhanced to keep count of muted CC, SYSEX changes previously muted
 * so when we unmute a track it sounds with correct parameters even if they weren't tramsmitted before
 * So member function are changed
 */


/* NEW BY NC */
void AdvancedSequencer::SoloTrack (int trk) {
    if (!file_loaded)
        return;
        // unsoloing done by UnSoloTrack()
    if (IsPlay())
        // track could be muted before soloing: this set appropriate CC, PC, etc
        // not previously sent
        CatchEventsBefore(trk);
    seq->SetSoloMode (true, trk);
    for (int i = 0; i < seq->GetNumTracks(); ++i) {
        if (i == trk) continue;
        mgr->GetDriver(0)->AllNotesOff(FindFirstChannelOnTrack(i));
        seq->GetTrackState (i)->note_matrix.Clear();
    }
}


void AdvancedSequencer::UnSoloTrack()  {
    if (!file_loaded)
        return;
    if (IsPlay())
        // this set appropriate CC, PC, etc for previously muted tracks
        CatchEventsBefore();
    seq->SetSoloMode (false);
}


void AdvancedSequencer::SetTrackMute (int trk, bool f) {
    if (!file_loaded)
        return;
    seq->GetTrackProcessor (trk)->mute = f;
    if (IsPlay()) {
        if (f)
            mgr->GetDriver(0)->AllNotesOff( FindFirstChannelOnTrack(trk) );  // TODO: tieni conto del rechannelize
        else
            // track was muted: this set appropriate CC, PC, etc not previously sent
            CatchEventsBefore(trk);
    }
}


void AdvancedSequencer::UnmuteAllTracks() {
    if (!file_loaded)
        return;
    for ( int i = 0; i < seq->GetNumTracks(); ++i) {
        if (seq->GetTrackProcessor(i)->mute) {
            seq->GetTrackState ( i )->note_matrix.Clear();
            seq->GetTrackProcessor (i)->mute = false;
        }
    }
    mgr->AllNotesOff();
    if (IsPlay())
        // this set appropriate CC, PC, etc for previously muted tracks
        CatchEventsBefore();
}


MIDIClockTime AdvancedSequencer::GetCurrentMIDIClockTime() const {   // new by NC
    MIDIClockTime time = seq->GetCurrentMIDIClockTime();
    if (mgr->IsSeqPlay()) {
        double ms_offset = mgr->GetCurrentTimeInMs() - seq->GetCurrentTimeInMs();
        double ms_per_clock = 60000.0 / (seq->GetState()->tempobpm *
                                seq->GetCurrentTempoScale() * tracks->GetClksPerBeat());
        time += (MIDIClockTime)(ms_offset / ms_per_clock);
    }
    return time;
}


unsigned long AdvancedSequencer::GetCurrentTimeInMs() const {
// NEW: this is now effective also during playback
    if (mgr->IsSeqPlay())
        return mgr->GetCurrentTimeInMs();
    else
       return seq->GetCurrentTimeInMs();
}


bool AdvancedSequencer::SetClksPerBeat (unsigned int cpb) {
    if (file_loaded)
        // you can change this only when the multitrack is empty
        return false;
    tracks->SetClksPerBeat(cpb);
    seq->GetState()->Reset();
    return true;
}


int AdvancedSequencer::GetMeasure() const {
    if (!file_loaded)
        return 0;
    return seq->GetCurrentMeasure();
}


int AdvancedSequencer::GetBeat() const {
    if (!file_loaded)
        return 0;
    return seq->GetCurrentBeat();
}


int AdvancedSequencer::GetTimeSigNumerator() const {
    if (!file_loaded)
        return 4;
    return seq->GetState ()->timesig_numerator;
}


int AdvancedSequencer::GetTimeSigDenominator() const {
    if (!file_loaded)
        return 4;
    return seq->GetState ()->timesig_denominator;
}


int AdvancedSequencer::GetKeySigSharpFlat() const {
    if (!file_loaded)
        return 0;
    return seq->GetState ()->keysig_sharpflat;
}


int AdvancedSequencer::GetKeySigMode() const {
    if (!file_loaded)
        return 0;
    return seq->GetState ()->keysig_mode;           /* NC */
}


int AdvancedSequencer::GetTrackNoteCount (int trk) const {
    if (!file_loaded || mgr->IsSeqStop())
        return 0;
    else
        return seq->GetTrackState ( trk )->note_matrix.GetTotalCount();
}


std::string AdvancedSequencer::GetTrackName (int trk) const {
    if (!file_loaded)
        return "";
    return seq->GetTrackState (trk)->track_name;
}


int AdvancedSequencer::GetTrackVolume (int trk) const {
    if (!file_loaded)
        return 100;
    return seq->GetTrackState (trk)->volume;
}


int AdvancedSequencer::GetTrackProgram (int trk) const {
    if (!file_loaded)
        return 0;
    return seq->GetTrackState ( trk )->program;
}


void AdvancedSequencer::SetTrackVelocityScale (int trk, double scale) {
    if (!file_loaded)
        return;
    scale *= 100;
    seq->GetTrackProcessor (trk)->velocity_scale = (int) scale;
}


double AdvancedSequencer::GetTrackVelocityScale (int trk) const {
    if (!file_loaded)
        return 1.0;
    return seq->GetTrackProcessor (trk)->velocity_scale * 0.01;
}


void AdvancedSequencer::SetTrackRechannelize (int trk, int chan) {
    if (!file_loaded)
        return;
    seq->GetTrackProcessor (trk)->rechannel = chan;
    mgr->AllNotesOff();
    seq->GetTrackState (trk)->note_matrix.Clear();
}


int AdvancedSequencer::GetTrackRechannelize (int trk) const {
    if (!file_loaded)
        return -1;
    return seq->GetTrackProcessor (trk)->rechannel;
}


void AdvancedSequencer::SetTrackTranspose (int trk, int trans) {
    if (!file_loaded)
        return;
    bool was_playing = mgr->IsSeqPlay();
    if (was_playing)
        mgr->SeqStop();         // TODO: this should be buggy if we close the ports

    if (trk == -1)
        for ( trk = 0; trk < seq->GetNumTracks(); ++trk )
            seq->GetTrackProcessor ( trk )->transpose = trans;
    else
        seq->GetTrackProcessor ( trk )->transpose = trans;

    if (was_playing) {
        mgr->AllNotesOff();
        seq->GetTrackState (trk)->note_matrix.Clear();
        mgr->SeqPlay();
    }
}


int AdvancedSequencer::GetTrackTranspose (int trk) const {
    if (!file_loaded)
        return 0;
    return seq->GetTrackProcessor (trk)->transpose;
}


std::string AdvancedSequencer::GetCurrentMarker() const {
    if (!file_loaded)
        return "";
    return seq->GetState()->marker_text;
}

void AdvancedSequencer::SetChanged() {
        // IMPORTANT: REWRITTEN: WAS BUGGY!!!!!
    bool was_playing = false;
    if (IsPlay()) {
        was_playing = true;
        Stop();     // however you should avoid to edit the MIDIMultiTrack during playback!
    }
    file_loaded = true;
    ExtractWarpPositions();
    if (was_playing)
        Play();
}



//
// protected members
//


int AdvancedSequencer::FindFirstChannelOnTrack ( int trk )
{
    int first_channel = -1;
    if ( !file_loaded )
    {
        return first_channel;
    }

    MIDITrack *t = tracks->GetTrack ( trk );

    if ( t )
    {
        // go through all events
        // until we find a channel message
        // and then return the channel number plus 1
        for ( int i = 0; i < t->GetNumEvents(); ++i )
        {
            MIDITimedBigMessage *m = t->GetEventAddress ( i );

            if ( m )
            {
                if ( m->IsChannelMsg() )
                {
                    first_channel = m->GetChannel();
                    break;
                }
            }
        }
    }
    return first_channel;
}

/* OLD
void AdvancedSequencer::ExtractWarpPositions()
{
    if ( !file_loaded )
    {
        for ( int i = 0; i < num_warp_positions; ++i )
        {
            jdks_safe_delete_object ( warp_positions[i] );
        }

        num_warp_positions = 0;
        return;
    }

    Stop();
    // delete all our current warp positions

    for ( int i = 0; i < num_warp_positions; ++i )
    {
        jdks_safe_delete_object ( warp_positions[i] );
    }

    num_warp_positions = 0;

    while ( num_warp_positions < MAX_WARP_POSITIONS )
    {
        if ( !seq.GoToMeasure ( num_warp_positions * MEASURES_PER_WARP, 0 ) )
        {
            break;
        }

        // save current sequencer state at this position
        warp_positions[num_warp_positions++] =
            new MIDISequencerState (
            *seq.GetState()
        );
    }

    seq.GoToMeasure ( 0, 0 );
}
*/
/* NEW by NC */
void AdvancedSequencer::ExtractWarpPositions()
{
    if ( !file_loaded )
    {
        warp_positions.clear();
        return;
    }

    Stop();
    // warp_positions is now a vector of objects ( not pointers ) so we can minimize memory dealloc/alloc

    MIDIClockTime cur_time = seq->GetCurrentMIDIClockTime();

    // temporarily disable the gui notifier
    bool notifier_mode = false;

    if ( notifier )
    {
        notifier_mode = notifier->GetEnable();
        notifier->SetEnable ( false );
    }

    unsigned int num_warp_positions = 0;
    while ( seq->GoToMeasure ( num_warp_positions * MEASURES_PER_WARP ) )
    {

        // save current sequencer state at this position
        if ( num_warp_positions < warp_positions.size() )
        {   // copy if it's already contained ...
            warp_positions[num_warp_positions] = *seq->GetState();
        }
        else
        {   // ... or push back
            warp_positions.push_back( MIDISequencerState( *seq->GetState() ) );
        }
        num_warp_positions++;
    }
    while ( warp_positions.size() > num_warp_positions )
    {   // adjust vector size if it was greater than actual (currently num_warp_positions is the last
        // vector index plus 1, so the comparison is OK)
        warp_positions.pop_back();
    }

    // now find the actual number of measures
    num_measures = (num_warp_positions - 1) * MEASURES_PER_WARP;
    while (seq->GoToMeasure( num_measures + 1 ))
    {
        num_measures++;
    }

    seq->GoToTime( cur_time );

    // re-enable the gui notifier if it was enabled previously
    if ( notifier )
    {
        notifier->SetEnable ( notifier_mode );
        // cause a full gui refresh now
        notifier->Notify ( MIDISequencerGUIEvent::GROUP_ALL );
    }
}


void AdvancedSequencer::CatchEventsBefore()
{
    MIDITimedBigMessage msg;
    MIDITimedBigMessage *msgp;
    MIDIMultiTrackIterator iter( seq->GetState()->multitrack );
    int trk;
    int events_sent = 0;

    if (GetCurrentMIDIClockTime() == 0)
        return;
    std::cout << "Catch events before started ..." << std::endl;

    iter.GoToTime( 0 );
    if (!mgr->GetDriver(0)->IsPortOpen())
        mgr->GetDriver(0)->OpenPort();
    while ( iter.GetCurEvent( &trk, &msgp ) && msgp->GetTime() < seq->GetCurrentMIDIClockTime() )
    {
        msg = *msgp;
        if (msg.IsControlChange() &&
            ! (msg.IsVolumeChange() || msg.IsPanChange()) &&
            !GetTrackMute(trk))
        {   // only send CC messages not in the sequencer state

            OutputMessage(msg);
            events_sent++;
        }
        else
        if ( msg.IsSysEx() )  // TODO: which SysEx should we send?
        {
            OutputMessage(msg);
            MIDITimer::Wait( 10 );
            events_sent++;
        }
        else
        if (!msg.IsChannelMsg() && !msg.IsMetaEvent())
        {   // TODO: which other messages should we send???
            // DumpMIDITimedBigMessage(&msg);
            OutputMessage( msg );   // prudentially sends an unrecognized message
            events_sent++;
        }
        iter.GoToNextEvent();

    }
    for (int i = 1; i < GetNumTracks(); i++) {
        if (!GetTrackMute(i)) {
            int channel = FindFirstChannelOnTrack(i);
            if (channel == -1)
                continue;
            const MIDISequencerTrackState* state = seq->GetTrackState(i);
            msg.SetProgramChange(channel, state->program);
            OutputMessage(msg);
            msg.SetVolumeChange(channel, state->volume);
            OutputMessage(msg);
            msg.SetPanChange(channel, state->pan);
            OutputMessage(msg);
            msg.SetPitchBend(channel, state->bender_value);
            OutputMessage(msg);
            events_sent += 4;
            // reverb and chorus already sent as CC
        }
    }

    std::cout << "CatchEventsBefore finished: events sent: " << events_sent << std::endl;
    //if (!was_open)
    //    mgr->GetDriver(0)->ClosePort();
}


void AdvancedSequencer::CatchEventsBefore(int trk) {
    MIDITimedBigMessage msg;
    MIDITrack* t = tracks->GetTrack(trk);

    for (int i = 0; i < t->GetNumEvents(); ++i )
    {
        msg = t->GetEvent( i );
        if ( msg.GetTime() >= seq->GetCurrentMIDIClockTime() )
        {
            break;
        }

        if (msg.IsChannelMsg()) // channel messages
        {
            if ( msg.IsControlChange() || msg.IsProgramChange() || msg.IsPitchBend() )
            {   // only send these messages
                OutputMessage(msg);
            }
        }
        else
        if ( msg.IsMetaEvent() )
        {
            if ( msg.IsTempo() )
            {   // discards all meta events except tempo messages
                OutputMessage(msg);
            }
        }
        else
        if ( msg.IsSysEx() )      // TODO: which SysEx should we send???
        {
            OutputMessage(msg);
            MIDITimer::Wait( 10 );
        }
        else
        {   // TODO: which other messages should we send???
            // DumpMIDITimedBigMessage(&msg);
            OutputMessage( msg );   // prudentially sends an unrecognized message
        }
    }
}



/* OLD
bool AdvancedSequencer::OpenMIDI (int in_port, int out_port, int timer_resolution) {
    CloseMIDI();
    if (!driver->StartTimer(timer_resolution)) return false;
    if (in_port != -1) driver->OpenMIDIInPort (in_port);
    if (driver->OpenMIDIOutPort (out_port)) return true;
    else return false;
}


void AdvancedSequencer::CloseMIDI() {
    Stop();
    driver->StopTimer();
    Sleep (100);
    driver->CloseMIDIInPort();
    driver->CloseMIDIOutPort();
}

void AdvancedSequencer::SetMultiTrack(MIDIMultiTrack* mlt, TimeSigList* ts) {
    Reset();
    MIDISequencerGUIEventNotifier* nt = seq->GetState()->notifier;
    if (!file_loaded) delete seq->GetState()->multitrack;
                                        // delete the MIDIMultiTrack if it is the dummy one
    delete seq;

    seq = new MIDISequencer(mlt, nt);
    mgr->SetSeq(seq);
    timesigs = ts;                      // it's not owned by the class
    file_loaded = true;

    ExtractWarpPositions();             // with last modifies no more need for Clear()
    Reset();
}

*/


