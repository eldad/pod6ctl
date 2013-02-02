/*
 *  Line 6 Pod 2.3 MIDI Editing Tool
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

#ifndef _POD6CTL_H
#define _POD6CTL_H

#include <stdbool.h>
#include <stdio.h>

#define ERREXIT(expr) { \
	err = expr;\
	if (err < 0) {\
		printf("%s:" #expr " error %d\n", __func__, err);\
		exit(1);\
	}\
}

#define EXIT_ON(expr, args...) do {\
	if (expr) {\
		printf(args);\
		exit(1);\
	}\
} while (0)

#define CLIENT_NAME	"pod6ctl"

#define SYSEX_START 0xf0
#define SYSEX_END 0xf7

extern bool debug_mode;
#define debug(args...) do { if (debug_mode) fprintf(stderr, args); } while (0)
#define info(args...) do { fprintf(stderr, args); } while (0)

extern int nohello;

#endif
