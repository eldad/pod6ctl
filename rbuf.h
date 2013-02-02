/*
 *  Line 6 Pod 2.3 MIDI Editing Tool
 *  Resizeable Buffer
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

#ifndef RBUF_H
#define RBUF_H

#define RBUF_CHUNKSIZE 32

struct rbuf {
	unsigned char *head;
	unsigned char *cur;
	unsigned char *tail;
};

#define rbuf_foreach(_rbuf, buf) \
	for (buf = (_rbuf)->head; \
		buf < (_rbuf)->cur; \
		buf++)

inline size_t rbuf_curlen(struct rbuf *buf)
{
	return (buf->cur - buf->head);
}

inline size_t rbuf_len(struct rbuf *buf)
{
	return (buf->tail - buf->head);
}

inline void rbuf_rewind(struct rbuf *buf)
{
	buf->cur = buf->head;
}

inline void rbuf_init(struct rbuf *buf)
{
	if (buf->head)
		free(buf->head);
	
	buf->head = malloc(RBUF_CHUNKSIZE);
	buf->tail = buf->head + RBUF_CHUNKSIZE;
	buf->cur = buf->head;
}

inline void rbuf_append(struct rbuf *buf, unsigned char c)
{
	if (!buf->head) {
		fprintf(stderr, "error: rbuf not initialized\n");
		return;
	}

	if (buf->cur == buf->tail) {
		int cur = rbuf_curlen(buf);
		int len = rbuf_len(buf) + RBUF_CHUNKSIZE;

		buf->head = realloc(buf->head, len);
		buf->tail = buf->head + len;
		buf->cur = buf->head + cur;
	}

	*(buf->cur) = c;
	buf->cur++;
}

#endif
