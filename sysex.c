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

#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/poll.h>
#include <time.h>

#include <alsa/asoundlib.h>

#include "pod6ctl.h"
#include "rbuf.h"
#include "bank.h"

static struct rbuf sybuf;

static snd_rawmidi_t *input;
static snd_rawmidi_t *output;

static int hello_rx = 0;

static void midi_open(const char *port_name)
{
	int err;

	rbuf_init(&sybuf);

	ERREXIT(snd_rawmidi_open(&input, &output, port_name, SND_RAWMIDI_NONBLOCK));
	ERREXIT(snd_rawmidi_nonblock(output, 0));
}

static void midi_close()
{
	int err;

	ERREXIT(snd_rawmidi_close(input));
	ERREXIT(snd_rawmidi_close(output));
}

static void sysex_send(unsigned char *cmd, int len)
{
	int err, i;

	debug("Sending: ");
	for (i = 0; i < len; i++) {
		debug("%02hhx",  *(cmd + i));
	}
	debug("\n");
	ERREXIT(snd_rawmidi_write(output, cmd, len));
}

static int sysex_read()
{
	int err;
	bool sysex = false;
	unsigned char c = 0;

	rbuf_rewind(&sybuf);

	for (;;) {
		debug(".");
		ERREXIT(snd_rawmidi_read(input, &c, sizeof(c)));
		if (err == 0)
			continue;
		debug("-");

		if (!sysex) {
			if (c == SYSEX_START)
				sysex = true;

			if (!sysex)
				debug("(!sysex) %02hhx\n", c);

			continue;
		}

		if (c == SYSEX_END) {
			debug("\n");
			break;
		}

		rbuf_append(&sybuf, c);
		debug("0x%02hhx, ", c);
	}

	return rbuf_curlen(&sybuf);
}

static void sysex_wait_on(unsigned char *sysex_msg, size_t cmpn)
{
	int n;

	for (;;) {
		int i;
		n = sysex_read();
		if (n == 0)
			continue;

		if (memcmp(sysex_msg, sybuf.head, cmpn) == 0)
			break;

		if (debug_mode) {
			printf("unexpected message rx\n");
			for (i = 0; i < cmpn; i++) {
				if (sysex_msg[i] != sybuf.head[i])
					debug("idx %d wanted %02hhx got %02hhx\n", i, 
						sysex_msg[i], sybuf.head[i]);
			}
		}
	}
}

static void sysex_hello()
{
	unsigned char hello_req[] = { SYSEX_START, 0x7e, 0x7f, 0x06, 0x01, SYSEX_END };
	unsigned char hello_res[] = { 	0x7e, 0x7f, 0x06, 0x02, 0x00,
					0x01, 0x0c, 0x00, 0x00, 0x00,
					0x03, 0x30, 0x32, 0x33, 0x30 };

	if (nohello)
		info("WARNING: Skipping device discovery!\n");

	debug("Probing... ");
	sysex_send(hello_req, sizeof(hello_req));
	debug("Waiting... ");
	sysex_wait_on(hello_res, sizeof(hello_res));

	if (hello_rx++ == 0)
		info("Found Line 6 POD 2.3\n");
}

static void sysex_to_bank(struct bank *b, int n)
{
	unsigned char bank_req[] = { SYSEX_START, 0x00, 0x01, 0x0c, 0x01, 0x00, 0x00, n, SYSEX_END };
	unsigned char bank_res[] = { 0x00, 0x01, 0x0c, 0x01, 0x01, 0x00, n , 0x00};
	unsigned char buf[BANK_SIZE];
	unsigned char *p;
	int i;

	sysex_send(bank_req, sizeof(bank_req));
	sysex_wait_on(bank_res, sizeof(bank_res));
	debug("Got bank (%d) %ld bytes\n", n, rbuf_curlen(&sybuf));

	EXIT_ON(rbuf_curlen(&sybuf) != (BANK_SIZE * 2 + sizeof(bank_res)), "unexpected bank length\n");

	p = &sybuf.head[sizeof(bank_res)];
	for (i = 0; i < BANK_SIZE; i++) {
		buf[i] = (*p << 4) | *(p + 1);
		p += 2;
	}

	memcpy(b, buf, sizeof(struct bank));
}

static void bank_to_sysex(struct bank *b, int n)
{
	unsigned char bank_req_hdr[] = { SYSEX_START, 0x00, 0x01, 0x0c, 0x01, 0x01, 0x00, n , 0x00};
	unsigned char *bytes = (unsigned char *)b;
	unsigned char *p;
	struct bank cur_b;
	int i;

	rbuf_rewind(&sybuf);
	for (i = 0; i < sizeof(bank_req_hdr); i++) {
		rbuf_append(&sybuf, bank_req_hdr[i]);
	}

	for (i = 0; i < sizeof(struct bank); i++) {
		rbuf_append(&sybuf, (*bytes & 0xf0) >> 4);
		rbuf_append(&sybuf, (*bytes & 0x0f));
		bytes++;
	}

	rbuf_append(&sybuf, SYSEX_END);
	sysex_send(sybuf.head, rbuf_curlen(&sybuf));

	if (debug_mode) {
		printf("sybuf size %ld:", rbuf_curlen(&sybuf));
		for (p = sybuf.head; p < sybuf.cur; p++) {
			printf(" %02hhx", *p);
		}
		printf("\n");
	}

	sysex_to_bank(&cur_b, n);
	if (memcmp(&b[n], &cur_b, sizeof(struct bank) != 0))
		info("Error writing bank %s\n", bank_ntostr(n));
}

void sysex_set_all(const char *port_name, struct bank b[])
{
	int n;
	time_t start;
	time_t end;

	midi_open(port_name);
	sysex_hello();

	start = time(NULL);
	for (n = 0; n < BANKS_NR; n++) {
		bank_to_sysex(&b[n], n);
		info("Writing bank %s\r", bank_ntostr(n));
	}
	end = time(NULL);

	info("Done writing banks (%ld sec).\n", end - start);

	midi_close();
}

void sysex_get_all(const char *port_name, struct bank b[])
{
	int n;
	struct bank *p;

	midi_open(port_name);
	sysex_hello();

	for (n = 0; n < BANKS_NR; n++) {
		sysex_to_bank(&b[n], n);
		info("Read bank %s\r", bank_ntostr(n));
		p++;
	}

	info("Done reading banks.\n");

	midi_close();
}

int sysex_get_bank(const char *port_name, struct bank *b, int n)
{
	EXIT_ON(b == NULL,"NULL dereference @ %s", __func__);

	midi_open(port_name);
	sysex_hello();
	sysex_to_bank(b, n);
	midi_close();

	return 0; // For now
}

int sysex_set_bank(const char *port_name, struct bank *b, int n)
{
	EXIT_ON(b == NULL,"NULL dereference @ %s", __func__);

	midi_open(port_name);
	sysex_hello();
	bank_to_sysex(b, n);
	midi_close();

	return 0; // For now
}

void program_change(const char *port_name, unsigned char n)
{
	unsigned char program_change_req[] = { 0xb0, 0xc0, n };

	midi_open(port_name);
	sysex_hello();
	sysex_send(program_change_req, sizeof(program_change_req));
	midi_close();
}
