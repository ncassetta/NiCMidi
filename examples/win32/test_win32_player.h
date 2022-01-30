/*
 *   Example file for NiCMidi - A C++ Class Library for MIDI
 *
 *   Copyright (C) 2021, 2022  Nicola Cassetta
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

#ifndef TEST_WIN32_H_INCLUDED
#define TEST_WIN32_H_INCLUDED

#include <windows.h>

#include "../../include/advancedsequencer.h"
#include "../../include/smpte.h"
#include "../../include/midi.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/* Declare other functions called by WinMain and WindowProcedure */
VOID ProcessNotifierMessage(MIDISequencerGUIEvent msg);
VOID LoadFile();
VOID SetControls();
const char* GetSmpteString();
VOID SetMIDIActivity();
VOID ResetDelays();

#endif // TEST_WIN32_H_INCLUDED
