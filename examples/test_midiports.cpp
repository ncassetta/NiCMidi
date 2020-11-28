/*
 *   Example file for NiCMidi - A C++ Class Library for MIDI
 *
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

/*
  A simple progran which enumerates the MIDI IN and OUT ports
  present on your system.
*/


#include "../include/manager.h"

using namespace std;

int main() {
    if (MIDIManager::GetNumMIDIIns()) {
        cout << "MIDI IN PORTS:" << endl;
        for (unsigned int i = 0; i < MIDIManager::GetNumMIDIIns(); i++)
            cout <<'\t' << i << ": " << MIDIManager::GetMIDIInName(i) << endl;
    }
    else
        cout << "NO MIDI IN PORTS" << endl;
    if (MIDIManager::GetNumMIDIOuts()) {
        cout << "MIDI OUT PORTS:" << endl;
        for (unsigned int i = 0; i < MIDIManager::GetNumMIDIOuts(); i++)
            cout << '\t' << i << ": " << MIDIManager::GetMIDIOutName(i) << endl;
    }
    else
        cout << "NO MIDI OUT PORTS" << endl;
    cout << endl << endl << "Press ENTER" << endl;
    cin.get();
    return EXIT_SUCCESS;
}

