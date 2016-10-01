#include "../include/advancedsequencer.h"
#include "../include/filereadmultitrack.h"          // for LoadMIDIFile()

#include <iostream>


AdvancedSequencer::AdvancedSequencer(MIDISequencerGUINotifier *n) :
    multitrack (new MIDIMultiTrack (17)),
    seq (new MIDISequencer (multitrack, n)),
    mgr (new MIDIManager (seq, n)),

    num_measures(0),
    file_loaded (false),

    ctor_type (CTOR_1)              // remembers what objects are owned
{
    mgr->SetAutoSeqOpen(false);     // takes the control on out ports
    mgr->SetAutoStopProc(AutoStopProc, this);
}


AdvancedSequencer::AdvancedSequencer(MIDIMultiTrack* mlt, MIDISequencerGUINotifier *n) :
    multitrack (mlt),
    seq (new MIDISequencer (multitrack, n)),
    mgr (new MIDIManager (seq, n)),

    ctor_type (CTOR_2)              // remembers what objects are owned

{
    mgr->SetAutoSeqOpen(false);     // takes the control on out ports
    mgr->SetAutoStopProc(AutoStopProc, this);
    if (multitrack->GetNumTracksWithEvents() == 0 && multitrack->GetEndTime() == 0)
        file_loaded = false;        // the multitrack is empty
    else
        file_loaded = true;
    ExtractWarpPositions();     // sets warp_positions and num_measures
}


AdvancedSequencer::AdvancedSequencer(MIDIManager *mg) :
    multitrack ((MIDIMultiTrack *)(mg->GetSequencer()->GetState()->multitrack)),
    seq (mg->GetSequencer()),
    mgr (mg),

    ctor_type (CTOR_3)      // remembers what objects are owned

{
    mgr->SetAutoSeqOpen(false);     // takes the control on out ports
    mgr->SetAutoStopProc(AutoStopProc, this);
    if (multitrack->GetNumTracksWithEvents() == 0 && multitrack->GetEndTime() == 0)
        file_loaded = false;        // the multitrack is empty
    else
        file_loaded = true;
    ExtractWarpPositions();         // sets warp_positions and num_measures
}


AdvancedSequencer::~AdvancedSequencer() {
    Stop();
    if (ctor_type != CTOR_3) {
        delete mgr;
        delete seq;
    }
    if (ctor_type == CTOR_1)
        delete multitrack;
}


/*
void AdvancedSequencer::SetInputPort( int p)
{
    Stop();
    //CloseMIDI();
    in_port = p;
    //OpenMIDI(in_port, out_port);
}
*/


void AdvancedSequencer::SetMIDIThruChannel (int chan) {
    thru_rechannelizer.SetAllRechan (chan);
    mgr->AllNotesOff();
}


void AdvancedSequencer::SetMIDIThruTranspose (int amt) {
    thru_transposer.SetAllTranspose (amt);
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
    multitrack->Clear();

    file_loaded = LoadMIDIFile(realname, multitrack);
    seq->Reset();               // synchronizes the sequencer with the multitrack and goes to 0
    seq->GoToZero();
    ExtractWarpPositions();
    return file_loaded;
}


void AdvancedSequencer::UnLoad() {
    Stop();
    multitrack->Clear();
    multitrack->SetClksPerBeat(DEFAULT_CLKS_PER_BEAT);
    file_loaded = false;
    seq->Reset();
    seq->GoToZero();
    //Reset();
    ExtractWarpPositions();
}

// TODO: revise this
void AdvancedSequencer::Reset() {
    Stop();
    MIDITimer::Wait(500);    // pauses for 0.5 sec (TROUBLE WITHOUT THIS!!!! I DON'T KNOW WHY)
    //mgr->Reset();  TODO: check all mgr->Reset() should be unneeded     // closes ports and clear matrices
    seq->Reset();       // syncronize the num of tracks and reset track processors
    seq->GoToZero();    // update the sequencer state
}

/*
void AdvancedSequencer::GoToZero() {
    if (!file_loaded)
        return;

    Stop();                 // always stops if playing
    seq->GoToZero();
}
*/


void AdvancedSequencer::GoToTime (MIDIClockTime t) {
    if (!file_loaded)
        return;

    // figure out which warp item we use
    // try warp to the last warp point BEFORE the
    // requested measure

    unsigned int warp_to_item = 0;
    for (; warp_to_item < warp_positions.size(); warp_to_item++) {
        if (warp_positions[warp_to_item].cur_clock > t)
            break;
    }
    if (warp_to_item == warp_positions.size() && warp_to_item != 0)
        warp_to_item--;

    if (mgr->IsSeqPlay()) {
        mgr->SeqStop();
        seq->SetState (&warp_positions[warp_to_item]);
        seq->GoToTime (t);
        mgr->SeqPlay();
    }
    else {
        seq->SetState (&warp_positions[warp_to_item]);
        seq->GoToTime (t);
        for (unsigned int i = 0; i < seq->GetNumTracks(); ++i)
            seq->GetTrackState ( i )->note_matrix.Clear();
    }
}


void AdvancedSequencer::GoToMeasure (int measure, int beat) {
    if (!file_loaded)
        return;

    // figure out which warp item we use
    // try warp to the last warp point BEFORE the
    // requested measure
    unsigned int warp_to_item = measure / MEASURES_PER_WARP;

    if (warp_to_item >= warp_positions.size())
        warp_to_item = warp_positions.size() - 1;

    if (mgr->IsSeqPlay()) {
        mgr->SeqStop();
        seq->SetState (&warp_positions[warp_to_item]);
        seq->GoToMeasure (measure, beat);
        mgr->SeqPlay();
    }
    else {
        seq->SetState (&warp_positions[warp_to_item]);
        seq->GoToMeasure (measure, beat);
        for (unsigned int i = 0; i < seq->GetNumTracks(); ++i)
            seq->GetTrackState ( i )->note_matrix.Clear();
    }
}


void AdvancedSequencer::Play () {
    if (!file_loaded)
        return;

    mgr->SeqStop();
    if (mgr->GetRepeatPlay())
        GoToMeasure ( mgr->GetRepeatPlayStart() );

    mgr->OpenOutPorts();
    CatchEventsBefore();
    // this intercepts any CC, SYSEX and TEMPO messages and send them to the out port
    // allowing to start with correct values; we could incorporate this in the
    // sequencer state, but it would track even CC (not difficult) and SYSEX messages

    mgr->SeqPlay();
}


void AdvancedSequencer::Stop() {
    if (!file_loaded || !mgr->IsSeqPlay())
        return;

    mgr->SeqStop();
    mgr->CloseOutPorts();
    //mgr->AllNotesOff();       // already done by SeqStop()
    //mgr->Reset();
    // stops on a beat (and clear midi matrix)
    GoToMeasure(seq->GetState()->cur_measure, seq->GetState()->cur_beat);
}


void AdvancedSequencer::OutputMessage(MIDITimedMessage& msg, unsigned int port) {
    bool was_open = true;
    MIDIOutDriver* driver = mgr->GetOutDriver(port);
    if (!driver->IsPortOpen()) {
        was_open = false;
        driver->OpenPort();
    }
    driver->OutputMessage(msg);
    if (!was_open)
        driver->ClosePort();
}


void AdvancedSequencer::SetRepeatPlay (bool f, int start_measure, int end_measure) {
    if (!file_loaded)
        return;

    if (start_measure < end_measure && start_measure >= 0 && end_measure <= num_measures)
        mgr->SetRepeatPlay (f, start_measure, end_measure);
    else
        mgr->SetRepeatPlay(false, 0, 0);

    if (mgr->IsSeqPlay() && mgr->GetRepeatPlay()) {
        mgr->SeqStop();
        GoToMeasure (start_measure);
        CatchEventsBefore();
        mgr->SeqPlay();
    }
}

/* NOTE BY NC: soloing and muting has been enhanced to keep count of muted CC, SYSEX changes previously muted
 * so when we unmute a track it sounds with correct parameters even if they weren't tramsmitted before
 * So member function are changed
 */


void AdvancedSequencer::SoloTrack (int trk) {
    if (!file_loaded)
        return;
        // unsoloing done by UnSoloTrack()
    if (mgr->IsSeqPlay()) {
        for (unsigned int i = 0; i < seq->GetNumTracks(); ++i) {
            if (i == (unsigned)trk) continue;
            if (GetTrackChannel(i) != -1) {
                mgr->GetOutDriver(seq->GetTrackPort(trk))->AllNotesOff(GetTrackChannel(i));
                seq->GetTrackState (i)->note_matrix.Clear();
            }
        }
    }
    seq->SetSoloMode (true, trk);
    if (mgr->IsSeqPlay())
    // track could be muted before soloing: this set appropriate CC, PC, etc
    // not previously sent
        CatchEventsBefore(trk);     // MUST be here! We must previously unmute the track!
}


void AdvancedSequencer::UnSoloTrack()  {
    if (!file_loaded)
        return;
    if (mgr->IsSeqPlay())
        // this set appropriate CC, PC, etc for previously muted tracks
        CatchEventsBefore();
    seq->SetSoloMode (false);
}


void AdvancedSequencer::SetTrackMute (int trk, bool f) {
    if (!file_loaded)
        return;
    seq->GetTrackProcessor (trk)->mute = f;
    if (mgr->IsSeqPlay()) {
        int channel = GetTrackChannel(trk);
        if (channel == -1)
            return;
        if (f) {
            mgr->GetOutDriver(seq->GetTrackPort(trk))->AllNotesOff(channel);
            seq->GetTrackState (trk)->note_matrix.Clear();
        }
        else
            // track was muted: this set appropriate CC, PC, etc not previously sent
            CatchEventsBefore(trk);
    }
}


void AdvancedSequencer::UnmuteAllTracks() {
    if (!file_loaded)
        return;
    for (unsigned int i = 0; i < seq->GetNumTracks(); ++i) {
        if (seq->GetTrackProcessor(i)->mute) {
            //seq->GetTrackState (i)->note_matrix.Clear();
            seq->GetTrackProcessor (i)->mute = false;
        }
    }
    //mgr->AllNotesOff();
    if (mgr->IsSeqPlay())
        // this set appropriate CC, PC, etc for previously muted tracks
        CatchEventsBefore();
}


void AdvancedSequencer::SetTempoScale(double scale) {
    if (!file_loaded)
        return;
    bool was_playing = mgr->IsSeqPlay();
    // doesn't stop the sequencer! (avoids AllNotesOff())

    seq->SetTempoScale(scale);
    if (was_playing)
        mgr->SeqPlay(); // update manager internal time parameters
}


MIDIClockTime AdvancedSequencer::GetCurrentMIDIClockTime() const {
    MIDIClockTime time = seq->GetCurrentMIDIClockTime();
    if (mgr->IsSeqPlay()) {
        double ms_offset = mgr->GetCurrentTimeInMs() - seq->GetCurrentTimeInMs();
        double ms_per_clock = 60000.0 / (seq->GetState()->tempobpm *
                                seq->GetTempoScale() * multitrack->GetClksPerBeat());
        time += (MIDIClockTime)(ms_offset / ms_per_clock);
    }
    return time;
}


tMsecs AdvancedSequencer::GetCurrentTimeInMs() const {
// NEW: this is now effective also during playback
    if (mgr->IsSeqPlay())
        return mgr->GetCurrentTimeInMs();
    else
       return seq->GetCurrentTimeInMs();
}


/*
bool AdvancedSequencer::SetClksPerBeat (unsigned int cpb) {
    multitrack->SetClksPerBeat(cpb);
    seq->GetState()->Reset();
    return true;
}
*/


int AdvancedSequencer::GetCurrentMeasure() const {
    if (!file_loaded)
        return 0;
    return seq->GetCurrentMeasure();
}


int AdvancedSequencer::GetCurrentBeat() const {
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
    return seq->GetState ()->keysig_mode;
}


void AdvancedSequencer::SetTrackOutPort (int trk, unsigned int port) {
    if (mgr->IsSeqPlay() && port != seq->GetTrackPort(trk) && GetTrackChannel(trk) != -1) {
        mgr->GetOutDriver(seq->GetTrackPort(trk))->AllNotesOff(GetTrackChannel(trk));
        seq->GetTrackState (trk)->note_matrix.Clear();
    }
    seq->SetTrackPort(trk, port);
}


int AdvancedSequencer::GetTrackNoteCount (int trk) const {
    if (!file_loaded || !mgr->IsSeqPlay())
        return 0;
    else
        return seq->GetTrackState (trk)->note_matrix.GetTotalCount();
}


std::string AdvancedSequencer::GetTrackName (int trk) const {
    if (!file_loaded)
        return "";
    return seq->GetTrackState (trk)->track_name;
}


char AdvancedSequencer::GetTrackVolume (int trk) const {
    if (!file_loaded)
        return 100;
    return (int)(seq->GetTrackState (trk)->control_values[C_MAIN_VOLUME]);
}


char AdvancedSequencer::GetTrackProgram (int trk) const {
    if (!file_loaded)
        return 0;
    return seq->GetTrackState (trk)->program;
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
    return seq->GetTrackProcessor (trk)->velocity_scale * 0.01d;
}


void AdvancedSequencer::SetTrackRechannelize (int trk, int chan) {
    if (!file_loaded)
        return;
    if (mgr->IsSeqPlay() && GetTrackChannel(trk) != chan && !(GetTrackChannel(trk) == -1)) {
        mgr->GetOutDriver(seq->GetTrackPort(trk))->AllNotesOff(GetTrackChannel(trk));
        seq->GetTrackState (trk)->note_matrix.Clear();
    }
    seq->GetTrackProcessor (trk)->rechannel = chan;
}


int AdvancedSequencer::GetTrackRechannelize (int trk) const {
    if (!file_loaded)
        return -1;
    return seq->GetTrackProcessor (trk)->rechannel;
}


int AdvancedSequencer::GetTrackChannel (int trk) const {
    if (!file_loaded)
        return -1;
    return (seq->GetTrackProcessor (trk)->rechannel == -1 ?
            multitrack->FindFirstChannelOnTrack(trk) : seq->GetTrackProcessor (trk)->rechannel);
}


void AdvancedSequencer::SetTrackTranspose (int trk, int trans) {
    if (!file_loaded)
        return;

    if (mgr->IsSeqPlay() && GetTrackTranspose(trk) != trans && !(GetTrackChannel(trk) == -1)) {
        mgr->GetOutDriver(seq->GetTrackPort(trk))->AllNotesOff(GetTrackChannel(trk));
        seq->GetTrackState (trk)->note_matrix.Clear();
    }
    seq->GetTrackProcessor (trk)->transpose = trans;
}


int AdvancedSequencer::GetTrackTranspose (int trk) const {
    if (!file_loaded)
        return 0;
    return seq->GetTrackProcessor (trk)->transpose;
}


void AdvancedSequencer::SetTrackTimeOffset (int trk, int time) {
    if (!file_loaded)
        return;

    bool was_playing = mgr->IsSeqPlay();
    if (was_playing) {
        mgr->SeqStop();
        //seq->GetTrackState (trk)->note_matrix.Clear();
    }

    seq->SetTrackTimeOffset(trk, time);
    seq->GoToTime(seq->GetCurrentMIDIClockTime());

    if (was_playing)
        mgr->SeqPlay();
}


int AdvancedSequencer::GetTrackTimeOffset (int trk) const {
    if (!file_loaded)
        return 0;
    return seq->GetTrackTimeOffset(trk);
}


std::string AdvancedSequencer::GetCurrentMarker() const {
    if (!file_loaded)
        return "";
    return seq->GetState()->marker_text;
}

void AdvancedSequencer::SetChanged() {
        // IMPORTANT: REWRITTEN: WAS BUGGY!!!!!
    bool was_playing = false;
    if (mgr->IsSeqPlay()) {
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

/* MOVED TO MIDIMultiTrack

int AdvancedSequencer::FindFirstChannelOnTrack (int trk) {
    int first_channel = -1;
    if (!file_loaded || trk >= GetNumTracks())
        return first_channel;

    MIDITrack *t = multitrack->GetTrack (trk);

    if (t) {
        // go through all events
        // until we find a channel message
        // and then return the channel number
        for (unsigned int i = 0; i < t->GetNumEvents(); ++i) {
            MIDITimedMessage *msg = t->GetEventAddress (i);
            if (msg->IsChannelMsg()) {
                    first_channel = msg->GetChannel();
                    break;
            }
        }
    }
    return first_channel;
}
*/


void AdvancedSequencer::ExtractWarpPositions()
{
    if ( !file_loaded ) {
        warp_positions.clear();
        num_measures = 0;
        return;
    }

    MIDISequencerGUINotifier* notifier = seq->GetState()->notifier;

    Stop();
    // warp_positions is now a vector of objects ( not pointers ) so we can minimize memory dealloc/alloc

    MIDIClockTime cur_time = seq->GetCurrentMIDIClockTime();

    // temporarily disable the gui notifier
    bool notifier_mode = false;

    if (notifier) {
        notifier_mode = notifier->GetEnable();
        notifier->SetEnable ( false );
    }

    unsigned int num_warp_positions = 0;
    while (seq->GoToMeasure (num_warp_positions * MEASURES_PER_WARP)) {
        // save current sequencer state at this position
        if (num_warp_positions < warp_positions.size())
            // copy if it's already contained ...
            warp_positions[num_warp_positions] = *seq->GetState();
        else
            // ... or push back
            warp_positions.push_back(MIDISequencerState(*seq->GetState()));

        num_warp_positions++;
    }
    if (warp_positions.size() > num_warp_positions)
        // adjust vector size if it was greater than actual
        warp_positions.resize(num_warp_positions, MIDISequencerState(multitrack, notifier));

    // now find the actual number of measures
    num_measures = (num_warp_positions - 1) * MEASURES_PER_WARP;
    while (seq->GoToMeasure(num_measures + 1))
        num_measures++;

    seq->GoToTime(cur_time);

    // re-enable the gui notifier if it was enabled previously
    if (notifier) {
        notifier->SetEnable ( notifier_mode );
        // cause a full gui refresh now
        notifier->Notify (MIDISequencerGUIEvent::GROUP_ALL);
    }
}


void AdvancedSequencer::CatchEventsBefore() {
    MIDITimedMessage msg;
    MIDITimedMessage *msgp;
    MIDIMultiTrackIterator iter( seq->GetState()->multitrack );
    int events_sent = 0;

    if (GetCurrentMIDIClockTime() == 0)         // nothing to do
        return;
    std::cout << "Catch events before started ..." << std::endl;

    //mgr->OpenOutPorts();

    int trk;
    iter.GoToTime(0);
    // re-send all sysex, except real-time ones
    while (iter.GetCurEvent( &trk, &msgp ) && msgp->GetTime() < seq->GetCurrentMIDIClockTime()) {
        msg = *msgp;
        unsigned int port = seq->GetTrackPort(trk);

        if (msg.IsSysEx() &&
            !(msg.GetSysEx()->IsGMReset() || msg.GetSysEx()->IsGSReset() || msg.GetSysEx()->IsXGReset())) {
            OutputMessage(msg, port);
            events_sent++;
        }
        iter.GoToNextEvent();
    }

    // now set program, controls and pitch bend
    for (int i = 1; i < GetNumTracks(); i++) {
        if (!GetTrackMute(i)) {
            int channel = GetTrackChannel(i);
            unsigned int port = seq->GetTrackPort(i);
            if (channel == -1)
                continue;
            const MIDISequencerTrackState* state = seq->GetTrackState(i);
            // set the current program
            msg.SetProgramChange(channel, state->program);
            OutputMessage(msg, port);
            // set thecurrent pitch bend value
            msg.SetPitchBend(channel, state->bender_value);
            OutputMessage(msg, port);
            events_sent += 2;
            // set the controllers
            for (unsigned int j = 0; j < C_ALL_NOTES_OFF; j++) {
                if (state->control_values[j] != -1) {
                    msg.SetControlChange(channel, j, state->control_values[j]);
                    OutputMessage(msg, port);
                    events_sent++;
                }   // TODO: RPN and NRPN
            }
        }
    }

    //mgr->CloseOutPorts();
    std::cout << "CatchEventsBefore finished: events sent: " << events_sent << std::endl;
}


void AdvancedSequencer::CatchEventsBefore(int trk) {
    MIDITimedMessage msg;
    MIDITrack* t = multitrack->GetTrack(trk);
    unsigned int port = seq->GetTrackPort(trk);
    int events_sent = 0;

    if (GetCurrentMIDIClockTime() == 0)         // nothing to do
        return;
    std::cout << "Catch events before started for track " << trk << " ..." << std::endl;

    //mgr->OpenOutPorts();

    // restore program, pitch bend and controllers as registered in the sequencer stste
    if (!GetTrackMute(trk)) {

        for (unsigned int i = 0; i < t->GetNumEvents(); ++i) {
            msg = t->GetEvent(i);
            if (msg.GetTime() >= seq->GetCurrentMIDIClockTime())
                break;

            if (msg.IsSysEx() &&
                !(msg.GetSysEx()->IsGMReset() || msg.GetSysEx()->IsGSReset() || msg.GetSysEx()->IsXGReset())) {
                OutputMessage(msg, port);
                events_sent++;
            }
        }

        int channel = GetTrackChannel(trk);
        if (channel != -1) {
            const MIDISequencerTrackState* state = seq->GetTrackState(trk);
            // set the current program
            msg.SetProgramChange(channel, state->program);
            OutputMessage(msg, port);
            // set thecurrent pitch bend value
            msg.SetPitchBend(channel, state->bender_value);
            OutputMessage(msg, port);
            events_sent += 2;
            // set the controllers
            for (unsigned int i = 0; i < C_ALL_NOTES_OFF; i++) {
                if (state->control_values[i] != -1) {
                    msg.SetControlChange(channel, i, state->control_values[i]);
                    OutputMessage(msg, port);
                    events_sent++;
                }   // TODO: RPN and NRPN
            }
        }
    }

    //mgr->CloseOutPorts();
    std::cout << "CatchEventsBefore finished: events sent: " << events_sent << std::endl;

}


void AdvancedSequencer::AutoStopProc(void* p) {
    AdvancedSequencer* seq = static_cast<AdvancedSequencer *>(p);
    seq->Stop();
}
