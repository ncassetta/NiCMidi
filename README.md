NiCMidi
=======

A MIDI C++ library derived by J. D. Koftinoff jdksmidi
----------------------------------------------------

NiCMidi is a MIDI library written in C++ by Nicola Cassetta, which implements objects for playing, recording and editing MIDI content. Moreover it can load and save MIDI files. It was originally a fork of jdksmidi, an old similar library written by J. D. Koftinoff and no more updated (see https://github.com/jdkoftinoff/jdksmidi). The author has rewritten the old code, adding many new features and using a more modern C++ style.

NiCMidi uses the RTMidi library of Gary Scavone (see http://www.music.mcgill.ca/~gary/rtmidi/) for communications between the library software and the hardware MIDI ports regardless the underlying OS.

In NiCMidi the timing is provided by the C++ <std::chrono> classes, so you need to compile the library according to (at least) the C++ 0x11 standard.

This is the <a href="https://ncassetta.github.io/NiCMidi/docs/html" target="_blank">link to the documentation</a>.

