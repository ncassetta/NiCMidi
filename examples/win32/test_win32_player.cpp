/*
 *   Example file for NiCMidi - A C++ Class Library for MIDI
 *
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

/*
  A MS Windows GUI example of the usage of the AdvancedSequencer
  class together with a MIDISequencerGUINotifierWin32 and a SMPTE.
  It is a basic MIDI file player with a responsive GUI showing the
  file parameters while the song is playing. It uses standard Windows
  API for creating the GUI.
*/


#include <windows.h>
#include <cwchar>

#include "../../include/advancedsequencer.h"
#include "../../include/smpte.h"
#include "../../include/midi.h"


//  Declare functions
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
VOID ProcessNotifierMessage(MIDISequencerGUIEvent msg);
VOID LoadFile();
VOID SetControls();
const char* GetSmpteString();
VOID SetMIDIActivity();
VOID ResetDelays();


// Declare MIDI objects
AdvancedSequencer *sequencer;       // the sequencer
SMPTE smpte;                        // milliseconds to smpte converter


// Declare handles to the window controls (In a real application you may want to subclass your
// window class and make them local)

HWND hMainWin;                      // our main window
HWND hFileName;                     // filename box
HWND hTime;                         // time box
HWND hKey;                          // keysig box
HWND hTempo;                        // tempo box
HWND hMeas;                         // meas/beat box
HWND hSmpte;                        // smpte box
HWND hMarker;                       // marker box
HWND hTrackNames[16];               // array of boxes for track names
HWND hTrackChans[16];               // array of boxes for track channels
HWND hTrackPrgrs[16];               // array of boxes for track programs
HWND hTrackVols[16];                // array of boxes for track volumAes
HWND hTrackActs[16];                // array of boxes for track activity

// Declare other variables
UINT NotifierMessage = 0;           // the Windows message id to communicate between notifier and GUI
int MIDIActDelays[16];              // array of integers for temporizing MIDI activity boxes
const int ACT_DELAY = 4;

//
// WinMain
//

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    MSG messages;            // Here messages to the application are saved
    WNDCLASSEX wincl;        // Data structure for the windowclass
    char szClassName[ ] = "Test AdvancedSequencer in WIN32";    // Class name

    // The Window structure
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;        // This function is called by windows
    wincl.style = CS_DBLCLKS;                   // Catch double-clicks
    wincl.cbSize = sizeof (WNDCLASSEX);
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
                                                // Use default icon and mouse-pointer
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                  // No menu
    wincl.cbClsExtra = 0;                       // No extra bytes after the window class
    wincl.cbWndExtra = 0;                       // Structure or the window instance
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
                                                // Use Windows's default colour as the background of the window

    // Register the window class, and if it fails quit the program
    if (!RegisterClassEx (&wincl))
        return 0;

    // The class is registered, let's create the window
    hMainWin = CreateWindowEx (
           0,                                   // Extended possibilites for variation
           szClassName,                         // Class name
           "test WIN32 (C) by N. Cassetta",     // Title Text
           WS_OVERLAPPEDWINDOW,                 // Default window
           CW_USEDEFAULT,                       // Windows decides the position
           CW_USEDEFAULT,                       // where the window ends up on the screen
           800,                                 // The window width
           600,                                 // and height in pixels
           HWND_DESKTOP,                        // The window is a child-window to desktop
           NULL,                                // No menu
           hThisInstance,                       // Program Instance handler
           NULL                                 // No Window Creation data
           );

    // Now create the MIDI objects: the GUI notifier and the sequencer (to send messages to the window
    // the notifier needs to know its handle and the message id)
    MIDISequencerGUINotifierWin32 notifier (
        hMainWin                        // The window handle to which send messages
        );
    NotifierMessage = notifier.GetMsgId();  // auto gets from Windows a safe message id number
    sequencer = new AdvancedSequencer( &notifier );

    // Make the window visible on the screen
    ShowWindow (hMainWin, nCmdShow);

    // Run the message loop. It will run until GetMessage() returns 0
    while (GetMessage (&messages, NULL, 0, 0))
    {
        // Translate virtual-key messages into character messages
        TranslateMessage(&messages);
        // Send message to WindowProcedure
        DispatchMessage(&messages);
    }

    // The program return-value is 0 - The value that PostQuitMessage() gave
    return messages.wParam;
}


//  This function is called by the Windows function DispatchMessage()
//  hwnd is the window handle
//  message is the message id
//  other parameters are used for identifying the widget that sent the message
LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == NotifierMessage)         // If the message is originated from the sequencer notifier ...
    {
        ProcessNotifierMessage( (MIDISequencerGUIEvent)lParam );
                                            // ... process it with our dedicated function to upgrade the GUI
        return 0;
    }

    switch (message) {                      // handle other messages (originated from Windows)

        // The WM_CREATE message is sent by Windows at the creation of the main window.
        // Here it creates all its children widgets: this is only windows API stuff and you
        // probably may want to do it with a dedicated UI toolkit.
        // However here we create some buttons and static (i.e. text boxes) controls. The buttons have
        // a (HMENU) parameter that will be used in case WM_COMMAND for identifying the button which
        // sent the command.
        // No library related content in case WM_CREATE.
         case WM_CREATE:

            // "Load" button
            CreateWindowW(L"button",        // Preregistered class (type) of window: in this case a button
                          L"Load",          // Button label
                WS_VISIBLE | WS_CHILD ,     // Attributes
                10, 10, 50, 25,             // x, y, w, h (in the parent window)
                hwnd,                       // Handle to the parent
                (HMENU) 1,                  // Parameter used for identifying the control
                NULL, NULL);                // Other unused parameters

            // "Rew" button
            CreateWindowW(L"button", L"Rew",// Same
                WS_VISIBLE | WS_CHILD ,
                100, 10, 50, 25,
                hwnd, (HMENU) 2, NULL, NULL);

            // "Play" button
            CreateWindowW(L"button", L"Play",
                WS_VISIBLE | WS_CHILD ,
                155, 10, 50, 25,
                hwnd, (HMENU) 3, NULL, NULL);

            // "Stop" button
            CreateWindowW(L"button", L"Stop",
                WS_VISIBLE | WS_CHILD ,
                210, 10, 50, 25,
                hwnd, (HMENU) 4, NULL, NULL);

            // Back step button
            CreateWindowW(L"button", L"<<",
                WS_VISIBLE | WS_CHILD ,
                265, 10, 50, 25,
                hwnd, (HMENU) 5, NULL, NULL);

            // Forward step button
            CreateWindowW(L"button", L">>",
                WS_VISIBLE | WS_CHILD ,
                320, 10, 50, 25,
                hwnd, (HMENU) 6, NULL, NULL);

            // Filename text box
            hFileName = CreateWindowW(L"static", L"UNLOADED",
                WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP,
                400, 10, 375, 25,
                hwnd, (HMENU) 7, NULL, NULL);

            // Timesig text box
            hTime = CreateWindowW(L"static", NULL,
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                100, 45, 50, 25,
                hwnd, (HMENU) 8, NULL, NULL);

            // Keysig text box
            hKey = CreateWindowW(L"static", NULL,
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                155, 45, 50, 25,
                hwnd, (HMENU) 7, NULL, NULL);

            // Tempo text box
            hTempo = CreateWindowW(L"static", NULL,
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                210, 45, 50, 25,
                hwnd, (HMENU) 8, NULL, NULL);

            // Meas - beat text box
            hMeas = CreateWindowW(L"static", NULL,
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                265, 45, 50, 25,
                hwnd, (HMENU) 7, NULL, NULL);

            // SMPTE text box
            hSmpte = CreateWindowW(L"static", NULL,
                WS_VISIBLE | WS_CHILD | SS_CENTER,
                400, 45, 105, 25,
                hwnd, (HMENU) 7, NULL, NULL);

            // Marker text box
            hMarker = CreateWindowW(L"static", NULL,
                WS_VISIBLE | WS_CHILD | SS_LEFT,
                510, 45, 265, 25,
                hwnd, (HMENU) 7, NULL, NULL);

            // For every track ...
            for (int i = 0; i < 16; i++) {
                wchar_t s[5];
                _snwprintf(s, 5, L"%d", i+1);   // ??? Should be swprintf, but MINGW says "not defined"

                // Track number text box
                CreateWindowW(L"static", s,
                    WS_VISIBLE | WS_CHILD | SS_LEFT,
                    10, 80+30*i, 30, 25,
                    hwnd, NULL, NULL, NULL);

                // Track name text box
                hTrackNames[i] = CreateWindowW(L"static", NULL,
                    WS_VISIBLE | WS_CHILD | SS_LEFT,
                    45, 80+30*i, 180, 25,
                    hwnd, NULL, NULL, NULL);

                // Track channel text box
                hTrackChans[i] = CreateWindowW(L"static", NULL,
                    WS_VISIBLE | WS_CHILD | SS_LEFT,
                    230, 80+30*i, 70, 25,
                    hwnd, NULL, NULL, NULL);

                // Track program (patch) text box
                hTrackPrgrs[i] = CreateWindowW(L"static", NULL,
                    WS_VISIBLE | WS_CHILD | SS_LEFT,
                    305, 80+30*i, 180, 25,
                    hwnd, NULL, NULL, NULL);

                // Track volume text box
                hTrackVols[i] = CreateWindowW(L"static", NULL,
                    WS_VISIBLE | WS_CHILD | SS_LEFT,
                    490, 80+30*i, 70, 25,
                    hwnd, NULL, NULL, NULL);

                // Track MIDI activity text box
                hTrackActs[i] = CreateWindowW(L"static", NULL,
                    WS_VISIBLE | WS_CHILD | SS_LEFT,
                    565, 83+30*i, 19, 19,
                    hwnd, NULL, NULL, NULL);
            }
            break;

        // The WM_COMMAND message is sent by buttons: so we identify the button that sent the message
        // (LOWORD(wParam) is the MENU param in the corresponding CreateWindowW) and call the appropriate
        // AdvancedSequencer method
        case WM_COMMAND:

            if (LOWORD(wParam) == 1) {      // Load
                // load a file into the sequencer and reinitialize the GUI
                LoadFile();
            }

            if (LOWORD(wParam) == 2) {      // Rew
                // reset the MIDI activity boxes delay parameter
                ResetDelays();
                // rewind the sequencer
                sequencer->GoToZero();
            }

            if (LOWORD(wParam) == 3) {      // Play
                ResetDelays();
                // start playing the file
                sequencer->Play();
                // start a Windows timer for updating the smpte box: the timer
                // sends a WM_TIMER message to the main window every 1/20 second
                SetTimer(hwnd,              // window to which send WM_TIMER messages
                         1,                 // timer id
                         50,                // timer interval (50 msecs)
                         NULL);             // unused
            }

            if (LOWORD(wParam) == 4) {      // Stop
                // stop playback
                sequencer->Stop();
                // stop the timer (1 is the timer id)
                KillTimer(hMainWin, 1);
                // reset the track activity boxes
                ResetDelays();
                SetMIDIActivity();
            }

            if (LOWORD(wParam) == 5) {      // Step backward
                ResetDelays();
                // go to previous measure
                sequencer->GoToMeasure(sequencer->GetCurrentMeasure() - 1);
            }

            if (LOWORD(wParam) == 6) {      // Step forward
                ResetDelays();
                // go to next measure
                sequencer->GoToMeasure(sequencer->GetCurrentMeasure() + 1);
            }
            break;

        // The WM_TIMER message is sent by the timer for updating the SMPTE box
        // and the track activity boxes
        case WM_TIMER:

            if (LOWORD(wParam) == 1) {      // timer id
                // update the Smpte box with the current SMPTE string
                SetWindowText( hSmpte, GetSmpteString());
                // update the track activity boxes
                SetMIDIActivity();
            }
            break;

        // The WM_DESTROY message is sent by Windows when the program ends
        case WM_DESTROY:
            PostQuitMessage (0);       // send a WM_QUIT to the message queue
            break;

        default:                      // for messages that we don't deal with
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}


// This function loads a file into the sequencer
VOID LoadFile() {
    OPENFILENAME ofn;                       // A struct with appropriate values for the open file control
    char szFileName[MAX_PATH] = "";         // This will hold the filename

    // Sets the correct values for the OPENFILENAME struct
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Midi Files (*.mid)\0*.mid\0\0";  // this is the filename filter
    ofn.lpstrFile = szFileName;                         // the string which will hold the filename
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "mid";

    if(GetOpenFileName(&ofn)) {             // Shows the open file control and waits until it is closed
                                            // returning the filename in szFileName
        if (!sequencer->Load(szFileName)) { // Error: the sequencer could not load the file
            MessageBox(
                NULL,
                (LPCSTR)"File loading failed!",
                NULL,
                MB_ICONEXCLAMATION
            );
        }

        else {                              // File loading OK
            if (sequencer->GetNumTracks() > 17) {
                MessageBox(                 // Too many tracks! We have room only for 1 - 17
                    NULL,
                    (LPCSTR)"This file has more than 17 tracks!\nSome tracks won't be displayed",
                    (LPCSTR)"Warning!",
                    MB_ICONWARNING
                );
            }

            // update the filename textbox
            SetWindowText( hFileName, szFileName );

            // set the correct format and time offset for SMPTE
            sequencer->SetSMPTE(&smpte);

            // update the timesig, tempo ... controls
            SetControls();
        }
    }
}


// This function updates all the sequencer related textboxes: it is called when a file is loaded and when
// the GUI receives from the notifier a GROUP_ALL (reset) message (for example rewind or step action)
VOID SetControls() {
    char s[300];

    // update the timesig, tempo, meas-beat, smpte, marker boxes
    sprintf (s, "%d/%d", sequencer->GetTimeSigNumerator(), sequencer->GetTimeSigDenominator());
    SetWindowText(hTime, s);
    SetWindowText(hKey, KeyName(sequencer->GetKeySigSharpsFlats(), sequencer->GetKeySigMode()));
    sprintf (s, "%3.2f", sequencer->GetTempoWithoutScale());
    SetWindowText (hTempo, s);
    sprintf (s, "%d:%d", sequencer->GetCurrentMeasure() + 1, sequencer->GetCurrentBeat() + 1);
    SetWindowText (hMeas, s);
    SetWindowText (hSmpte, GetSmpteString());
    if (sequencer->GetCurrentMarker().length() == 0)
        SetWindowText (hMarker, "---");
    else
        SetWindowText (hMarker, sequencer->GetCurrentMarker().c_str());

    // for every track, update the name, channel, program, volume boxes
    unsigned int i = 1;
    for (; i < std::min (sequencer->GetNumTracks(), 17u); i++) {

        if (sequencer->GetTrackName(i).length())
            SetWindowText (hTrackNames[i - 1], sequencer->GetTrackName (i).c_str());
        else {
            sprintf(s, "(track %d)", i);
            SetWindowText (hTrackNames[i - 1], s);
        }

        if (sequencer->GetTrackChannel(i) == -1)        // there aren't channel events or the track is empty
            SetWindowText (hTrackChans[i - 1], "---" );
        else {
            sprintf(s, "ch: %d", sequencer->GetTrackChannel(i) + 1);
            SetWindowText(hTrackChans[i - 1], s);
        }

        if (sequencer->GetTrackProgram(i) == -1)
            SetWindowText(hTrackPrgrs[i - 1], "---");
        else {
            if (sequencer->GetTrackChannel(i) == 9)     // channel 10
                SetWindowText(hTrackPrgrs[i - 1], GetGMDrumkitName((int)sequencer->GetTrackProgram(i), 1));
            else
                SetWindowText(hTrackPrgrs[i - 1], GetGMProgramName((int)sequencer->GetTrackProgram(i), 1));
        }
        if (sequencer->GetTrackVolume(i) == -1)
            SetWindowText(hTrackVols[i - 1], "vol: ---");
        else {
            sprintf (s, "vol: %d", sequencer->GetTrackVolume(i));
            SetWindowText(hTrackVols[i -1], s);
        }
    }

    // make blank unused widgets
    for ( ; i < 17; i++) {
        SetWindowText (hTrackNames[i - 1], "");
        SetWindowText (hTrackChans[i - 1], "");
        SetWindowText (hTrackPrgrs[i - 1], "");
        SetWindowText (hTrackVols[i - 1], "");
    }
}

// This function returns a text string in the SMPTE format h:mm:ss:ff corresponding to
//the current sequencer time
const char* GetSmpteString() {
    static char s[100];

    // Get from the sequencer the current time in msecs
    unsigned long msecs = sequencer->GetCurrentTimeMs();

    // Feed the smpte with msecs
    smpte.SetMilliSeconds(msecs);

    // Now it qives us the hours, minutes, secs and frames corresponding to our msecs
    sprintf (s, "%d:%02d:%02d:%02d", smpte.GetHours(), smpte.GetMinutes(),
                smpte.GetSeconds(), smpte.GetFrames());
    return s;
}


// This function is called by WindowProcedure when it receives a notifier message: it does essentially
// the same things that SetControls does, but updates only the appropriate control based on the param
// ev (the notifier event)
 VOID ProcessNotifierMessage(MIDISequencerGUIEvent ev) {
// the param ev is a MIDISequencerGUIEvent object (see the file notifier.h)
    char s[300];

    // Get the group (general type) of the event
    switch (ev.GetGroup()) {

        // This is a general GUI reset event: update all textboxes
        case MIDISequencerGUIEvent::GROUP_ALL:
            SetControls();
            break;

        // This is an event regarding conductor track: find the type and update appropriate textbox
        case MIDISequencerGUIEvent::GROUP_CONDUCTOR:
            switch (ev.GetItem()) {
                // Tempo (bpm) is changed
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_TEMPO:
                    sprintf (s, "%3.2f", sequencer->GetTempoWithoutScale());
                    SetWindowText ( hTempo, s );
                    break;
                // Timesig is changed
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_TIMESIG:
                    sprintf (s, "%d/%d", sequencer->GetTimeSigNumerator(), sequencer->GetTimeSigDenominator());
                    SetWindowText( hTime, s );
                    break;
                /* TODO: */
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_KEYSIG:
                    SetWindowText( hKey, KeyName(sequencer->GetState()->keysig_sharpflat,
                                                 sequencer->GetState()->keysig_mode) );
                    break;
                // Marker is changed
                case MIDISequencerGUIEvent::GROUP_CONDUCTOR_MARKER:
                    SetWindowText (hMarker, sequencer->GetCurrentMarker().c_str());
                    break;
            }
            break;

        // This is an event regarding transport (start, stop, etc): we monitor only
        // beat events to update the meas - beat box
        case MIDISequencerGUIEvent::GROUP_TRANSPORT:
            if (ev.GetItem() == MIDISequencerGUIEvent::GROUP_TRANSPORT_BEAT) {
                sprintf (s, "%d:%d", sequencer->GetCurrentMeasure() + 1, sequencer->GetCurrentBeat() + 1);
                SetWindowText ( hMeas, s );
            }
            break;

        // This is a track event: find the track (GetEventSubGroup) and the type (GetEventItem) and proceed
        case MIDISequencerGUIEvent::GROUP_TRACK: {
            int track = ev.GetSubGroup();
            // do nothing if track is the master track or is not shown (we can show only 0 ... 17)
            if (track == 0 || track >= 17)
                break;
            // Program (patch) is changed: we must distinguish between channel 10 (drums) and other channels
            if (ev.GetItem() == MIDISequencerGUIEvent::GROUP_TRACK_PROGRAM) {
                if (sequencer->GetTrackChannel(track) == 9)     // channel 10
                    SetWindowText(hTrackPrgrs[track - 1], GetGMDrumkitName((int)sequencer->GetTrackProgram(track), 1) );
                else
                    SetWindowText ( hTrackPrgrs[track - 1], GetGMProgramName((int)sequencer->GetTrackProgram(track), 1) );
            }
            // Volume is changed
            else if (ev.GetItem() == MIDISequencerGUIEvent::GROUP_TRACK_VOLUME) {
                sprintf (s, "vol: %d", sequencer->GetTrackVolume(track) );
                SetWindowText (hTrackVols[track - 1], s);
                // This is a MIDI activity: set the corresponding flag for this track
                MIDIActDelays[track - 1] = ACT_DELAY;
                break;
            }
        }
    }
}


// This function is called by the timer every 1/50 sec and controls the MIDI activity boxes.
VOID SetMIDIActivity() {
    char s[10];
    // find the number of tracks which we must monitor
    int active_tracks = (sequencer->GetNumTracks() <= 17 ? sequencer->GetNumTracks() : 17);
    // for every sequencer track ...
    for (int i = 1; i < active_tracks; i++) {
        // if any note is sounding in the track ...
        if (sequencer->GetTrackNoteCount(i))
            // ... set the track MIDI activity flag to its initial value
            MIDIActDelays[i - 1] = ACT_DELAY;       // ACT_DELAY currently 4 (200 msec delay after the activity)
        // no actual MIDI activity but delay still active
        else if (MIDIActDelays[i - 1] > 0)
            // decrement the delay flag
            MIDIActDelays[i - 1]--;
        // set the MIDI activity box on/off according to the activity flag
        if (MIDIActDelays[i - 1] > 0)
            sprintf (s, "X");
        else
            sprintf (s, "  ");
        SetWindowText ( hTrackActs[i - 1], s );
    }
}


// Reset all the Midi activity flags to 0 (no MIDI activity)
VOID ResetDelays() {
    for (int i = 0; i < 17; i++)
        MIDIActDelays[i] = 0;
}
