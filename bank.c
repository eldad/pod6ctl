/*
 *  Line 6 Pod 2.3 MIDI Editing Tool
 *  Banks
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

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include "pod6ctl.h"
#include "bank.h"

extern bool verbose;
extern bool debug_mode;

static const char *amp_models[32] = {
	"Tube Preamp",
	"Line 6 Clean",
	"Line 6 Crunch",
	"Line 6 Drive",
	"Line 6 Layer",
	"Small Tweed",
	"Tweed Blues",
	"Black Panel",
	"Modern Class A",
	"Brit Class A",
	"Brit Blues",
	"Brit Classic",
	"Brit Hi Gain",
	"Treadplate",
	"Modern Hi Gain",
	"Fuzz Box",
	"Jazz Clean",
	"Line 6 Twang",
	"Line 6 Crunch #2",
	"Line 6 Blues",
	"Line 6 Insane",
	"Small Tweed #2",
	"Boutique #3",
	"Black Panel #2",
	"Brit Class A #3",
	"Brit Class A #2",
	"California Crunch #1",
	"California Crunch #2",
	"Boutique #1",
	"Treadplate #2",
	"Modern Hi Gain #2",
	"Boutique #2"
};

int amp_features(struct bank *b)
{
	switch (b->amp_model) {
		case 0:
		case 6:
		case 8:
		case 11:
		case 12:
		case 13:
		case 15:
		case 17:
		case 23:
		case 25:
		case 26:
		case 27:
			return AMP_PRESENCE;
		case 1:
		case 2:
		case 3:
		case 10:
		case 16:
		case 24:
			return AMP_PRESENCE | AMP_BRIGHT;
		case 4:
			return AMP_PRESENCE | AMP_BRIGHT | AMP_DRIVE2;
		case 5:
		case 7:
		case 9:
		case 14:
		case 18:
		case 19:
		case 20:
		case 21:
		case 28:
		case 29:
		case 30:
		case 31:
			return AMP_NORMAL;
		case 22:
			return AMP_BRIGHT;
		default:
			return -1;
	}
}

static const char *effects[16] = {
	"Chorus 2",		/* 0 */
	"Flanger 1",    	/* 1 */
	"Rotary",		/* 2 */
	"Flanger 2",		/* 3 */
	"Delay/Chorus 1",	/* 4 */
	"Delay/Tremolo",	/* 5 */
	"Delay",		/* 6 */
	"Delay/Compressor",	/* 7 */
	"Chorus 1",		/* 8 */
	"Tremolo",		/* 9 */
	"Bypass",		/* 10 */
	"Compressor",		/* 11 */
	"Delay/Chrous 2",	/* 12 */
	"Delay/Flanger 1",	/* 13 */
	"Delay/Swell",		/* 14 */
	"Delay/Flanger 2"	/* 15 */
};

int effect_type(struct bank *b)
{
	switch (b->effect_type) {
	case 0:
	case 4:
	case 8:
	case 12:
		return EFFECT_CHORUS;
	case 1:
	case 3:
	case 13:
	case 15:
		return EFFECT_FLANGE;
	case 2:
		return EFFECT_ROTARY;
	case 5:
	case 9:
		return EFFECT_TREM;
	case 6:
	case 10:
		return EFFECT_NONE;
	case 7:
	case 11:
		return EFFECT_COMP;
	case 14:
		return EFFECT_VOL;
	default:
		return -1;
	}
}

static const char *cabinets[16] = {
	"1x8 1960 Fender Tweed Champ",
	"1x12 1952 Fender Tweed Deluxe",
	"1x12 1960 Vox AC-15",
	"1x12 1964 Fender Blackface Deluxe",
	"1x12 Line 6",
	"2x12 1965 Fender Blackface Twin",
	"2x12 1967 Vox AC-30",
	"2x12 1995 Matchless Chieftain",
	"2x12 Line 6",
	"4x10 1959 Fender Bassman",
	"4x10 Line 6",
	"4x12 1996 Marshall with Vintage 30s",
	"4x12 1978 Marshall with stock 70s",
	"4x12 1968 Marhsall Basketweave with Greenbacks",
	"4x12 Line 6",
	"No Cabinet"
};

static const char *compression_ratios[6] = {
	"Off",
	"1.4:1",
	"2:1",
	"3:1",
	"6:1",
	"Infinity:1"
};

static const char *rotary_switch[] = { "Slow", "Fast" };
static const char *simple_switch[] = { "Off", "On" };
static const char *volpos_switch[] = { "Pre-Tube", "Post-Tube" };
static const char *reverb_switch[] = { "Spring", "Hall" };

static const char *bank_idx[] = {
	"1A", "1B", "1C" ,"1D",
	"2A", "2B", "2C" ,"2D",
	"3A", "3B", "3C" ,"3D",
	"4A", "4B", "4C" ,"4D",
	"5A", "5B", "5C" ,"5D",
	"6A", "6B", "6C" ,"6D",
	"7A", "7B", "7C" ,"7D",
	"8A", "8B", "8C" ,"8D",
	"9A", "9B", "9C" ,"9D" };

const char *bank_ntostr(int n)
{
	return bank_idx[n];
}

int bank_strton(const char *s)
{
	int ret;

	if (strlen(s) != 2)
		return -EINVAL;

	if (s[0] < '1' || s[0] > '9')
		return -1;

	ret = (s[0] - '1') * 4;

	if (s[1] >= 'a' && s[1] <= 'd') {
		ret += s[1] - 'a';
		return ret;
	}

	if (s[1] >= 'A' && s[1] <= 'D') {
		ret += s[1] - 'A';
		return ret;
	}
	
	return -EINVAL;
}

void print_bank_ubytes(unsigned char *b)
{
	int i;

	for (i = 18; i < 47; i++) {
		if (i == 18 || (i >= 21 && i <= 23) || (i >= 25 && i <=27) ||
			(i >= 30 && i <= 33) || i == 35 || i == 37 || i == 47)
			printf("%03d: %02x\n", i, b[i]);
	}
}

void print_bank_bytes(unsigned char *b)
{
	int i;

	for (i = 0; i < BANK_SIZE; i++) {
		printf("%03d: %02x\n", i, b[i]);
	}
}

#define AMP_CHR(c)	((c > 20) && (c < 127))
char *bank_name_str(char *s, struct bank *b)
{
	size_t i;

	for (i = 0; i < BANK_NAME_LEN && AMP_CHR(b->bank_name[i]); i++) {
		s[i] = b->bank_name[i];
	}

	for (; i <= BANK_NAME_LEN; i++)
		s[i] = 0;
	
	return s;
}

const char *amp_model_name(struct bank *b)
{
	if (b->amp_model > 32)
		return NULL;

	return amp_models[b->amp_model];
}

const char *effect_name(struct bank *b)
{
	if (b->effect_type > 16)
		return NULL;

	return effects[b->effect_type];
}

const char *cabinet_name(struct bank *b)
{
	if (b->cabinet > 16)
		return NULL;

	return cabinets[b->cabinet];
}

const char *compression_ratio_name(struct bank *b)
{
	if (b->compression.ratio > 5)
		return NULL;

	return compression_ratios[b->compression.ratio];
}

struct bank_op {
	const char *name;
	const char *desc;
	int emask;
	int min;
	int max;
	int type;
	int scale;
	size_t offset;
	void (*set)(struct bank_op *, struct bank *, int);
	void (*setp)(struct bank_op *, struct bank *, int);
	int (*get)(struct bank_op *, struct bank *);
	int (*getp)(struct bank_op *, struct bank *);
	const char **sw;
	const char *units;
	int (*xfrm)(int, bool);
};

#define lengthof(x) ((size_t)(sizeof(x) / sizeof(__typeof__(*x))))
#define be16toh(b) ((*(b)) << 8 | (*(b + 1)))

/* Copied from Linux 3.8 list.h  */
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

int xfrm_gate_threshold(int val, bool xfrmin)
{
	if (xfrmin)
		return val - 96;
	
	return 96 + val;
}

int xfrm_mod_fb(int val, bool xfrmin)
{
	if (xfrmin)
		return (((val & 0x40) ? val & 0x3f : -val) * 100 / 0x3f);
	
	val = val * 0x3f / 100;

	return ((val > 0) ? 0x40 | val : -val);
}

void bank_set_be16(struct bank_op *op, struct bank *b, int val)
{
	unsigned char *p = ((unsigned char *)b) + op->offset;

	if (val < op->min || val > op->max)
		return;

	p[0] = val >> 8;
	p[1] = val & 0xff;
}

void bank_setp_be16(struct bank_op *op, struct bank *b, int val)
{
	bank_set_be16(op, b, (val * op->max / op->scale));
}

int bank_get_be16(struct bank_op *op, struct bank *b)
{
	unsigned char *p = ((unsigned char *)b) + op->offset;
	int val = 0;

	val = p[0] << 8 | p[1];

	return val;
}

int bank_getp_be16(struct bank_op *op, struct bank *b)
{
	return ((bank_get_be16(op, b) * op->scale) / op->max);
}

void bank_set_simple(struct bank_op *op, struct bank *b, int val)
{
	char *p = ((char *)b) + op->offset;

	if (val < op->min || val > op->max) {
		if (verbose)
			printf("*** out of range *** %d: %d < %d\n", val, op->min, op->max);
		return;
	}

	*p = val;
}

void bank_setp_simple(struct bank_op *op, struct bank *b, int val)
{
	if (op->xfrm)
		val = op->xfrm(val, false);
	else
		val = val * op->max / op->scale;

	bank_set_simple(op, b, val);
}

int bank_get_simple(struct bank_op *op, struct bank *b)
{
	char *p = ((char *)b) + op->offset;

	return *p;
}

int bank_op_scaled_value(struct bank_op *op, int val)
{
	if (op->xfrm)
		return op->xfrm(val, true);
	
	return val * op->scale / op->max;
}

int bank_getp_simple(struct bank_op *op, struct bank *b)
{
	int val = bank_get_simple(op, b);

	return bank_op_scaled_value(op, val);
}


#define OP_KNOB 0
#define OP_SWITCH 1

#define BE16_OP(d, member, _emask, _min, _max, _scale, _units) {\
	.name = #member,\
	.desc = d,\
	.min = _min,\
	.max = _max,\
	.type = OP_KNOB,\
	.scale = _scale,\
	.offset = offsetof(struct bank, member),\
	.set = bank_set_be16,\
	.setp = bank_setp_be16,\
	.get = bank_get_be16,\
	.getp = bank_getp_be16,\
	.emask = _emask,\
	.units = _units,\
}
#define BE16_STD_OP(d, member, _emask, _min, _max) BE16_OP(d, member, _emask, _min, _max, 100, "%")

#define U8_OP(d, member, _emask, _min, _max, _scale, _units, _xfrm) {\
	.name = #member,\
	.desc = d,\
	.min = _min,\
	.max = _max,\
	.type = OP_KNOB,\
	.scale = _scale,\
	.units = _units,\
	.offset = offsetof(struct bank, member),\
	.set = bank_set_simple,\
	.setp = bank_setp_simple,\
	.get = bank_get_simple,\
	.getp = bank_getp_simple,\
	.emask = _emask,\
	.xfrm = _xfrm,\
}
#define SIMPLE_RANGED_OP(d, member, _emask, _min, _max, _type) U8_OP(d, member, _emask, _min, _max, 100, "%", NULL)
#define SIMPLE_7B_OP(d, member) SIMPLE_RANGED_OP(d, member, 0, 0, 0x7f, NULL) 
#define SIMPLE_6B_OP(d, member) SIMPLE_RANGED_OP(d, member, 0, 0, 0x3f, NULL) 
#define SIMPLE_6B_DEP_OP(d, member, _emask) SIMPLE_RANGED_OP(d, member, _emask, 0, 0x3f, NULL) 

#define SWITCH_DEP_OP(d, member, s, _emask) {\
	.name = #member,\
	.desc = d,\
	.min = 0,\
	.max = lengthof(s) - 1,\
	.sw = s,\
	.type = OP_SWITCH,\
	.offset = offsetof(struct bank, member),\
	.set = bank_set_simple,\
	.get = bank_get_simple,\
	.emask = _emask,\
}

#define SWITCH_OP(d, member, s) SWITCH_DEP_OP(d, member, s, 0)
#define SIMPLE_SWITCH_OP(d, member) SWITCH_OP(d, member, simple_switch)
#define SIMPLE_SWITCH_DEP_OP(d, member, _emask) SWITCH_DEP_OP(d, member, simple_switch, _emask)

struct bank_op bank_ops[] = {
	SWITCH_OP("Amp Model", amp_model, amp_models),
	SWITCH_OP("Cabinet Model", cabinet, cabinets),
	SIMPLE_6B_OP("A.I.R. (acoustically integrated recording) Ambience Level", air),
	SWITCH_OP("Volume Pedal Position", volpos, volpos_switch),

	SIMPLE_6B_OP("Channel Volume", chan_vol),
	SIMPLE_6B_OP("Drive", drive),
	SIMPLE_6B_DEP_OP("Drive 2", drive2, AMP_DRIVE2),
	SIMPLE_6B_OP("Bass", bass),
	SIMPLE_6B_OP("Middle", middle),
	SIMPLE_6B_OP("Treble", treble),
	SIMPLE_6B_DEP_OP("Presence", presence, AMP_PRESENCE),

	SIMPLE_SWITCH_OP("Distortion", distortion),
	SIMPLE_SWITCH_OP("Drive/Boost", drive_boost),
	SIMPLE_SWITCH_OP("EQ", eq),
	SIMPLE_SWITCH_DEP_OP("Bright", bright, AMP_BRIGHT),

	SIMPLE_SWITCH_OP("Delay", delay),
	BE16_OP("Delay Time", delay_time, 0, 0x0000, 0x7ffa, 3150, "ms"),
	SIMPLE_6B_OP("Delay Repeats", delay_repeats),
	SIMPLE_6B_OP("Delay Level", delay_level),

	SIMPLE_SWITCH_OP("Reverb", reverb),
	SWITCH_OP("Reverb Type", reverb_type, reverb_switch),
	SIMPLE_6B_OP("Reverb Level", reverb_level),
	SIMPLE_6B_OP("Reverb Decay", reverb_decay),
	SIMPLE_6B_OP("Reverb Tone", reverb_tone),
	SIMPLE_6B_OP("Reverb Diffusion", reverb_diffusion),
	SIMPLE_6B_OP("Reverb Density", reverb_density),

	SIMPLE_SWITCH_OP("Noise Gate", noise_gate),
	U8_OP("Gate Threshold", gate_threshold, 0, 0, 96, 0, "dB", xfrm_gate_threshold),
	SIMPLE_6B_OP("Gate Decay", gate_decay),
	
	SIMPLE_7B_OP("Wah Bottom Frequency", wah_bottom),
	SIMPLE_7B_OP("Wah Top Frequency", wah_top),
	
	SWITCH_OP("Effect Type", effect_type, effects),

	SIMPLE_SWITCH_OP("Modulator (Chorus/Rotary/Tremolo)", mod),

	SWITCH_DEP_OP("Rotary Speed", rotary.speed, rotary_switch, EFFECT_ROTARY),
	BE16_STD_OP("Rotary Max Speed", rotary.max_speed, EFFECT_ROTARY, 0x0064, 0x0b4e),
	BE16_STD_OP("Rotary Min Speed", rotary.min_speed, EFFECT_ROTARY, 0x0064, 0x0b4e),

	SWITCH_DEP_OP("Compression Ratio", compression.ratio, compression_ratios, EFFECT_COMP),

	SIMPLE_6B_DEP_OP("Volume Swell Time", volume.swell_time, EFFECT_VOL),
	U8_OP("Modulator Feedback", modulator.feedback, EFFECT_FLANGE | EFFECT_CHORUS, 0, 0x7f, 0, "%", xfrm_mod_fb),
	BE16_STD_OP("Modulator Predelay", modulator.predelay, EFFECT_MOD, 0x0000, 0x0306),
	BE16_STD_OP("Tremolo Speed", modulator.speed, EFFECT_TREM, 0x0019, 0x0c67),
	BE16_STD_OP("Tremolo Depth", modulator.depth, EFFECT_TREM, 0x0038, 0x7f38),
	BE16_STD_OP("Flange/Chorus Speed", modulator.speed, EFFECT_FLANGE | EFFECT_CHORUS, 0x0032, 0x18ce),
	BE16_STD_OP("Flange/Chorus Depth", modulator.depth, EFFECT_FLANGE | EFFECT_CHORUS, 0x0000, 0x0138),
};

static const char *bank_get_switch(struct bank_op *ops, int val)
{
	if (val < ops->min || val > ops->max)
		return "** out of range **";
		
	return ops->sw[val];
}

static void bank_print_switch(struct bank_op *ops)
{
	int i;

	for (i = ops->min; i <= ops->max; i++)
		printf("  [%d] %s\n", i, ops->sw[i]);
}

#define test_bit(val, bit) ((val & bit) == bit)
void print_bank_ops()
{
	int i;

	printf("%-20s %s\n", "Attribute", "Description");

	for (i = 0; i < lengthof(bank_ops); i++) {
		struct bank_op *p = &bank_ops[i];

		printf("%-20s '%s'\n", p->name, p->desc);

		if (verbose) {
			switch (p->type) {
			case OP_KNOB:
				printf(" Type: Knob, Range: %d%s - %d%s\n", bank_op_scaled_value(p, p->min), p->units,
									 bank_op_scaled_value(p, p->max), p->units);
				break;
			case OP_SWITCH:
				printf(" Type: Switch, Range: %d - %d\n", p->min, p->max);
				EXIT_ON(!p->sw, "*** BUG *** Switch NULL reference");
				bank_print_switch(p);
				break;
			}

			if (test_bit(p->emask, EFFECT_COMP))
				printf(" Effective with compression\n");
			if (test_bit(p->emask, EFFECT_VOL))
				printf(" Effective with volume swell\n");
			if (test_bit(p->emask, EFFECT_ROTARY))
				printf(" Effective with rotary\n");
			if (test_bit(p->emask, EFFECT_TREM))
				printf(" Effective with tremolo\n");
			if (test_bit(p->emask, EFFECT_CHORUS))
				printf(" Effective with chorus\n");
			if (test_bit(p->emask, EFFECT_FLANGE))
				printf(" Effective with flange\n");
			printf("\n");
		}
	}
}

void print_bank(struct bank *b)
{
	int i;
	char buf[17];

	if (debug_mode) {
		printf("lengthof bank_ops %ld size %ld / %ld\n", lengthof(bank_ops), sizeof(bank_ops), sizeof(__typeof__(*bank_ops)));
		printf("offset of air %ld\n", offsetof(struct bank, distortion));
		printf("sizeof %ld\n", sizeof(struct bank));
	}

	bank_name_str(buf, b);
	printf("===========================\n");
	printf("Bank name: %s\n", buf);
	printf("===========================\n");

	for (i = 0; i < lengthof(bank_ops); i++) {
		struct bank_op *p = &bank_ops[i];
		int val = p->get(p, b);

		if (p->emask != 0 && ((effect_type(b) | amp_features(b)) & (p->emask)) == 0) {
			if (debug_mode)
				printf("*** skipping %s [%s] (got %04x want %04x)\n", p->desc, p->name, effect_type(b), p->emask);
			continue;
		}
	
		if (verbose)
			printf("[%s] ", p->name);

		printf("%s: ", p->desc);

		switch (p->type) {
		case OP_KNOB:
			printf("%d%s (%d)", p->getp(p, b), p->units, p->get(p, b));
			break;
		case OP_SWITCH:
			EXIT_ON(!p->sw, "Switch: NULL reference");
			printf("%s (%d)", bank_get_switch(p, val), val);
			break;
		}
	
		if (verbose)
			printf(" [%x] {%d:%d}", p->get(p, b), p->min, p->max);
		printf("\n");
	}

	if (debug_mode)
		print_bank_bytes((unsigned char *) b);
	else if (verbose)
		print_bank_ubytes((unsigned char *) b);

	printf("\n");
}

static void set_bank_param_v(struct bank *b, const char *cmd, const char *arg, bool v)
{
	int i;
	int val;

	for (i = 0; i < lengthof(bank_ops) - 1; i++) {
		struct bank_op *p = &bank_ops[i];

		if (strcmp(cmd, p->name) == 0) {
			val = strtol(arg, NULL, 0);
			if (v) {
				if (p->setp)
					p->setp(p, b, val);
				else
					fprintf(stderr, "Error: no percentual setting possible for '%s'\n", cmd);
			} else {
				if (p->set)
					p->set(p, b, val);
				else
					fprintf(stderr, "Error: no setting possible for '%s'\n", cmd);
			}
			return;
		}
	}
	fprintf(stderr, "Error: unknown command '%s'\n", cmd);
}

void set_direct_bank_param(struct bank *b, const char *cmd, const char *arg)
{
	set_bank_param_v(b, cmd, arg, false);
}

void set_scaled_bank_param(struct bank *b, const char *cmd, const char *arg)
{
	set_bank_param_v(b, cmd, arg, true);
}

