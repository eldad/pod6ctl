# pod6ctl

This software is only useful to owners of Line 6 POD (tm) devices.

It uses ALSA raw MIDI to query, save, restore and "deep-edit" the settings
via command line. It is quite feature complete from my point of view,
and since I do not own such a device anymore, I'll give it the 1.0
version number.

The only version supported is 2.3. It is possible to try it on other
versions (with the 'no hello' option; see man page), but please save
your settings before you try to do so.

To compile, simply clone and issue `make` in the source code directory.

Cheers,
Eldad
