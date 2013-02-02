/*
 *  Line 6 Pod 2.3 MIDI Editing Tool
 *  Sysex Interface
 *
 *  Copyright (c) 2013 Eldad Zack <eldad@fogrefinery.com>
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef _POD6CTL_SYSEX_H
#define _POD6CTL_SYSEX_H

void sysex_get_all(const char *port_name, struct bank b[]);
void sysex_set_all(const char *port_name, struct bank b[]);
int sysex_get_bank(const char *port_name, struct bank *b, int n);
int sysex_set_bank(const char *port_name, struct bank *b, int n);
void program_change(const char *port_name, unsigned char n);

#endif
