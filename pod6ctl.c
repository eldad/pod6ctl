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

#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <sys/poll.h>

#include <alsa/asoundlib.h>

#include "pod6ctl.h"
#include "rbuf.h"
#include "bank.h"
#include "sysex.h"

static char *port_name;
static bool overwrite = false;
static int bank_n = -1;
int nohello = false;
bool debug_mode;
bool verbose = false;

#define REQUIRE_MIDI() do { EXIT_ON(port_name == NULL, "Please specify MIDI port (-p)\n"); } while (0)
#define REQUIRE_BANK() do { EXIT_ON(bank_n < 0, "Please specify bank (1A - 9D) (-b)\n"); } while (0)

static void print_help()
{
	printf("ALSA MIDI Editing Tool for Line 6 Pod 2.3 (Raw MIDI)\n"
		"Version " VERSION "\n"
		"Copyright (c) 2013 Eldad Zack <eldad@fogrefinery.com> (Licensed under GPLv2; See COPYING)\n"
		"\nUsage: pod6ctl [command] [arg] <options>\n"
		"\nCommands:\n"
		" query                        Query one bank from POD\n"
		" save [filename]              Save all POD banks to file\n"
		" restore [filename]           Restore all banks from file to POD\n"
		" list [filename]              List banks in file\n"
		" name [name]                  Set bank name\n"
		" set [attr] [value]           Set an attribute to the value\n"
		" setdirect [attr] [value]     Set an attribute to a device value (for debugging; not recommended)\n"
		" attr                         Show the list of attributes\n"
		" writeb [pos] [value]         Writes a byte to a position, FOR DEBUGGING ONLY! DANGEROUS!\n"
		" select                       Select the current bank\n"
		" manual                       Engage manual mode\n"
		" tuner                        Engage tuner mode\n"
		"\nOptions:\n"
		" -p port       Raw MIDI Device (example: hw:2,0)\n"
		" -v            Verbose\n"
		" -D --debug    Debug\n"
		" -o            Allow file overwrite\n"
		" -h --help     Help\n"
		" -b            Bank (1A - 9D)\n"
		"\n");
}

static void query(char **unused)
{
	struct bank b;

	REQUIRE_MIDI();
	REQUIRE_BANK();

	sysex_get_bank(port_name, &b, bank_n);
	print_bank(&b);
}

static void save(char *argv[])
{
	const char *file_name = argv[0];
	int i, err;
	struct bank b[BANKS_NR];
	int fd;

	REQUIRE_MIDI();

	fd = open(file_name, O_WRONLY | O_CREAT | ((overwrite) ? 0 : O_EXCL), 0644);
	EXIT_ON(fd <= 0, "Error creating file (file must not exist): %s\n", file_name);

	sysex_get_all(port_name, b);

	if (verbose) {
		for (i = 0; i < BANKS_NR; i++) {
			printf("\nBank: %s (%d)\n", bank_ntostr(i), i);
			print_bank(&b[i]);
		}
	}
	
	err = write(fd, b, sizeof(b));
	EXIT_ON(err != sizeof(b), "Error writing (ret %d, errno %d)\n", err, errno);

	err = fsync(fd);
	EXIT_ON(!!err, "Error closing file (fsync) (err %d, errno %d)\n", err, errno);

	err = close(fd);
	EXIT_ON(!!err, "Error closing file (err %d, errno %d)\n", err, errno);

	info("Successfully wrote banks to '%s'\n", file_name);
}

static void load_banks(const char *file_name, struct bank b[])
{
	int err;
	int fd;

	fd = open(file_name, O_RDONLY);
	EXIT_ON(fd <= 0, "Error reading file: %s (fd %d, errno %d)\n", file_name, fd, errno);

	err = read(fd, b, sizeof(struct bank) * BANKS_NR);
	EXIT_ON(err != sizeof(struct bank) * BANKS_NR, "Error reading banks (file size mismatch)\n");

	close(fd);
}

static void list(char *argv[])
{
	const char *file_name = argv[0];
	int i;
	struct bank b[BANKS_NR];

	load_banks(file_name, b);

	for (i = 0; i < BANKS_NR; i++) {
		printf("Bank: %s (%d)\n", bank_ntostr(i), i);
		print_bank(&b[i]);
		printf("\n");
	}
}

static void restore(char *argv[])
{
	const char *file_name = argv[0];
	struct bank b[BANKS_NR];

	REQUIRE_MIDI();

	load_banks(file_name, b);
	sysex_set_all(port_name, b);
}

#define STRNCMP_USER_CONST(ustr, cstr)	strncmp(ustr, cstr, strlen(cstr));
static void name(char *argv[])
{
	const char *s = argv[0];
	size_t slen;
	struct bank b;

	REQUIRE_MIDI();
	REQUIRE_BANK();

	slen = strlen(s);
	EXIT_ON(slen > 16, "Maximum bank name length (16) exceeded\n");

	sysex_get_bank(port_name, &b, bank_n);
	memset(b.bank_name, 20, BANK_NAME_LEN);
	memcpy(b.bank_name, s, slen);
	sysex_set_bank(port_name, &b, bank_n);

	fprintf(stderr, "Set bank name to '%s'\n", s);
}

static void set(char *argv[])
{
	struct bank b;

	REQUIRE_MIDI();
	REQUIRE_BANK();

	sysex_get_bank(port_name, &b, bank_n);
	set_scaled_bank_param(&b, argv[0], argv[1]);
	sysex_set_bank(port_name, &b, bank_n);
	print_bank(&b);
}

static void setdirect(char *argv[])
{
	struct bank b;

	REQUIRE_MIDI();
	REQUIRE_BANK();

	sysex_get_bank(port_name, &b, bank_n);
	set_direct_bank_param(&b, argv[0], argv[1]);
	sysex_set_bank(port_name, &b, bank_n);
	print_bank(&b);
}

static void attr(char *argv[])
{
	print_bank_ops();
}

static void writeb(char *argv[])
{
	struct bank b;
	void *raw = &b;
	int offset = strtol(argv[0], NULL, 0);
	int val = strtol(argv[1], NULL, 0);

	EXIT_ON(offset < 0 || offset > sizeof(struct bank), "Offset out of range. Do you know what you are doing?\n");
	EXIT_ON(val < 0 || val > 255, "Value out of range. Do you know what you are doing?\n");

	REQUIRE_MIDI();
	REQUIRE_BANK();

	sysex_get_bank(port_name, &b, bank_n);
	*((char *)(raw + offset)) = val;
	sysex_set_bank(port_name, &b, bank_n);
	print_bank(&b);
}

#define PROGRAM_MANUAL	0
#define PROGRAM_TUNER	37
static void select(char *argv[])
{
	REQUIRE_MIDI();
	REQUIRE_BANK();

	/* Bank program numbers are one-base */
	program_change(port_name, bank_n + 1);
}

static void manual(char *argv[])
{
	REQUIRE_MIDI();

	program_change(port_name, PROGRAM_MANUAL);
}

static void tuner(char *argv[])
{
	REQUIRE_MIDI();

	program_change(port_name, PROGRAM_TUNER);
}

struct op_desc {
	const char *name;
	int argc;
	void (*op)(char *[]);
};

#define OP(op_name, c) {\
	.name = #op_name,\
	.op = op_name,\
	.argc = c,\
}

#define lengthof(x) ((size_t)(sizeof(x) / sizeof(__typeof__(*x))))
static struct op_desc ops[] = {
	OP(query, 0),
	OP(save, 1),
	OP(restore, 1),
	OP(list, 1),
	OP(name, 1),
	OP(set, 2),
	OP(setdirect, 2),
	OP(attr, 0),
	OP(writeb, 2),
	OP(select, 0),
	OP(manual, 0),
	OP(tuner, 0),
};

static void parse_options(const int argc, char *argv[])
{
	static const char shopts[] = "hp:Dovb:";
	static const struct option longopts[] = {
		{
			.name = "help",
			.has_arg = 0,
			.flag = NULL,
			.val = 'h'
		},
		{
			.name = "nohello",
			.has_arg = 0,
			.flag = &nohello,
			.val = true
		},
		{ 0 }
	};
	int optskip = 0;
	int c, n;

	if (argc <= 1) {
		print_help();
		exit(0);
	}

	while ((c = getopt_long(argc, &argv[optskip], shopts, longopts, NULL)) != -1) {
		switch (c) {
			case 'h':
				print_help();
				exit(0);
			case 'p':
				port_name = optarg;
				break;
			case 'o':
				overwrite = true;
				break;
			case 'D':
				debug_mode = true;
				break;
			case 'v':
				verbose = true;
				break;
			case 'b':
				bank_n = bank_strton(optarg);
				break;
		}
	}

	if (optind == argc) {
		printf("Please specify command.\n");
		print_help();
		exit(0);
	}

	for (n = 0; n < lengthof(ops); n++) {
		if (strcmp(argv[optind], ops[n].name) == 0)
			break;
	}

	if (n >= lengthof(ops) || optind != (argc - ops[n].argc - 1)) {
		printf("Invalid invocation (%d : %d).\n", optind, (argc - ops[n].argc));
		print_help();
		exit(0);
	}

	ops[n].op(&argv[optind + 1]);
}

int main(int argc, char *argv[])
{
	EXIT_ON(sizeof(struct bank) != BANK_SIZE,"struct bank %ld != BANK_SIZE %d\n",
		sizeof(struct bank), BANK_SIZE);

	parse_options(argc, argv);
	
	return 0;
}


