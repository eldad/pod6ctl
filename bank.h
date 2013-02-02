/*
 *  Line 6 Pod 2.3 MIDI Editing Tool
 *  Bank structure
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

#ifndef _POD6CTL_BANK_H
#define _POD6CTL_BANK_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Bank settings may depend on the selected effect and amp model */
#define EFFECT_MASK	0x77
#define EFFECT_NONE	0x00
#define EFFECT_COMP	0x01
#define EFFECT_VOL	0x02
#define EFFECT_ROTARY	0x04

#define EFFECT_MOD	0x70
#define EFFECT_TREM	0x10
#define EFFECT_CHORUS	0x20
#define EFFECT_FLANGE	0x40

#define AMP_MASK	0x0700
#define AMP_NORMAL	0x0000
#define AMP_BRIGHT	0x0100
#define AMP_PRESENCE	0x0200
#define AMP_DRIVE2	0x0400

/* Bank 0 = 1A, Bank 35 = 9D */
#define BANKS_NR 36

/* Bank name is a constant-sized string */
#define BANK_NAME_LEN	16

#define BANK_SIZE 71
struct bank { /* values are mostly 0 - 0x3f */
	unsigned char distortion;	/* 0, SW */
	unsigned char drive_boost;	/* 1, SW */
	unsigned char eq;		/* 2, SW */
	unsigned char delay;		/* 3, SW */
	unsigned char mod;		/* 4, SW */
	unsigned char reverb;		/* 5, SW */
	unsigned char noise_gate;	/* 6, SW */
	unsigned char bright;		/* 7, SW */
	unsigned char amp_model;	/* 8, SEL */
	unsigned char drive;		/* 9, DIAL */
	unsigned char drive2;		/* 10, DIAL */
	unsigned char bass;		/* 11 DIAL */
	unsigned char middle;		/* 12 DIAL */
	unsigned char treble;		/* 13 DIAL */
	unsigned char presence;		/* 14 DIAL */
	unsigned char chan_vol;		/* 15 DIAL */
	unsigned char gate_threshold;	/* 16, -96dB:0 (0:96)*/
	unsigned char gate_decay;	/* 17 DIAL */
	unsigned char u18;
	unsigned char wah_bottom;	/* 19 DIAL, 0:7f */
	unsigned char wah_top;		/* 20 DIAL, 0:7f */
	unsigned char u21;
	unsigned char u22;
	unsigned char u23;
	unsigned char volpos;		/* 24 SW, Pre-Tube(0), Post-Tube(1) */
	unsigned char u25;
	unsigned char u26;
	unsigned char u27;
	unsigned char delay_time[2];	/* 28-29 DIAL. (BE16 0:7ffa) */
	unsigned char u30;
	unsigned char u31;
	unsigned char u32;
	unsigned char u33;
	unsigned char delay_repeats;	/* 34 DIAL */
	unsigned char u35;
	unsigned char delay_level;	/* 36 DIAL */
	unsigned char u37;
	unsigned char reverb_type;	/* 38 SW, Spring(0)/Hall(1) */
	unsigned char reverb_decay;	/* 39 DIAL */
	unsigned char reverb_tone;	/* 40 DIAL */
	unsigned char reverb_diffusion;	/* 41 DIAL */
	unsigned char reverb_density;	/* 42 DIAL */
	unsigned char reverb_level;	/* 43 DIAL */
	unsigned char cabinet;		/* 44 SEL */
	unsigned char air;		/* 45 DIAL */
	unsigned char effect_type;	/* 46 SEL */
	unsigned char u47;
	union {	/* 48 */
		struct {
			unsigned char ratio; /* 48 DIAL */
		} compression;
		struct {
			unsigned char swell_time; /* 48 DIAL */
		} volume;
		struct {
			unsigned char speed; /* 48 SW, Slow(0) / Fast(1) */
			unsigned char max_speed[2]; /* 49-50 DIAL, BE16 0x0064:0xb4e */
			unsigned char min_speed[2]; /* 51-52 DIAL, BE16 0x0064:0xb4e */
		} rotary;
		struct {
			unsigned char speed[2];	/* 48,49 DIAL, Tremolo 0x0019-0x0c67, Chorus/Flange 0x0032:0x18ce */
			unsigned char depth[2];	/* 50,51 DIAL, Tremolo 0x0038-0x7f38, Chorus/Flange Depth 0:0x0138 */
			unsigned char feedback;	/* 52 DIAL, 0:0x7f, 0:0x3f (0:0x3f) negative, 0x40:0x7f (0:0x3f) positive */
			unsigned char predelay[2]; /* 53-54 DIAL, (BE16 0:306) */
		} modulator;
	};
	unsigned char bank_name[BANK_NAME_LEN];	/* 55-71 STRING, 0x20-Padded (no NULs). */
} __attribute__ ((__packed__));

void print_bank(struct bank *b);
void print_bank_ops();
void print_bank_bytes(unsigned char *b);
void print_bank_ubytes(unsigned char *b);

const char *amp_model_name(struct bank *b);
const char *effect_name(struct bank *b);
const char *cabinet_name(struct bank *b);
const char *compression_ratio_name(struct bank *b);

void set_direct_bank_param(struct bank *b, const char *cmd, const char *arg);
void set_scaled_bank_param(struct bank *b, const char *cmd, const char *arg);
	
int bank_strton(const char *s);
const char *bank_ntostr(int n);

#endif

