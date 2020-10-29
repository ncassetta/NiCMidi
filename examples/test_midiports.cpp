/*
  Simple progran which enumerates the MIDI IN and OUT ports
  present on your system

  Copyright (C) 2019 - 2020 N.Cassetta
  ncassetta@tiscali.it

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program;
  if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

