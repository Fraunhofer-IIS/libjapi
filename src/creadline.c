/*!
 * \file
 * \author Christopher Stender
 * \date 2018-02-15
 * \version 0.1
 *
 * \brief Read line from file descriptor
 *
 * \details
 * This readline implementation reads a single line from a file descriptor
 * (e.g. a socket). Two versions are provided.
 *
 * \copyright
 * Copyright (c) 2018 Fraunhofer IIS.
 * All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "creadline.h"

/* Do not change the maximum line size here! Define
 * CREADLINE_MAX_LINE_SIZE in the header file to
 * overwrite the default value. */
#ifndef CREADLINE_MAX_LINE_SIZE
/*! Internal definition of the maximum allowed line size. Can be overwritten by
 * defining CREADLINE_MAX_LINE_SIZE. */
#define __MAX_LINEBUF_SIZE__ 64*1024*1024
#else
#define __MAX_LINEBUF_SIZE__ CREADLINE_MAX_LINE_SIZE
#endif

static int strnpos(const char *s, int c, size_t maxlen)
{
	int pos;

	for (pos = 0; pos < maxlen; pos++) {

		if (s[pos] == c)
			return pos;

		if (s[pos] == '\0')
			return -1;
	}
	
	return -1;
}

int creadline_r(int fd, void **dst, creadline_buf_t *buffer)
{
	char *linebuf;
	size_t linebuf_size;
	int linebuf_nbytes;

	int readret;
	int nl_pos;
	int rem_nbytes;

	/* Initialize internal line buffer */
	linebuf_nbytes = 0;
	linebuf_size = CREADLINE_BLOCK_SIZE;
	linebuf = malloc(linebuf_size);
	if(linebuf == NULL) {
		perror("malloc() failed");
		goto error_ret;
	}

	/* Restore remaining characters from the last call */
	if (buffer->nbytes != 0) {
		strncpy(linebuf, buffer->buf, buffer->nbytes);
		linebuf_nbytes = buffer->nbytes;
		buffer->nbytes = 0;
	}

	/* Check if linebuf contains a newline character */
	nl_pos = strnpos(linebuf, '\n', linebuf_nbytes);

	while (nl_pos < 0) {

		/* No newline character found -> read more bytes and check again */

		/* Check if there is enough space left to call read again. If a new
		 * read could write beyond the line buffer it's size is doubled as long
		 * as __MAX_LINEBUF_SIZE__ is not reached. */

		if (linebuf_nbytes + CREADLINE_BLOCK_SIZE > linebuf_size) {

			char* new_linebuf = NULL;
			int new_linebuf_size = 2*linebuf_size;

			if (new_linebuf_size > __MAX_LINEBUF_SIZE__) {
				fprintf(stderr, "ERROR: Maximum line size of %i bytes exceeded!\n", __MAX_LINEBUF_SIZE__);
				goto error_free;
			}

			new_linebuf = realloc(linebuf, new_linebuf_size);
			if (new_linebuf == NULL) {
				perror("realloc() failed");
				goto error_free;
			}
			linebuf = new_linebuf;
			linebuf_size = new_linebuf_size;
		}

		/* Read more bytes... */
		readret = read(fd, linebuf+linebuf_nbytes, CREADLINE_BLOCK_SIZE);
		if (readret < 0) {

			if (errno == EINTR) { /* EINTR is not an error */
				continue;
			}

			perror("read() failed");
			goto error_free;

		} else if (readret == 0) { /* EOF */

			if (linebuf_nbytes == 0) {

				/* It is important to set dst to NULL because a return value
				 * of 0 can also indicate an zero-length string ("\0") */
				*dst = NULL;
				free(linebuf);
				return 0;

			} else {
				fprintf(stderr, "ERROR: Received EOF while line buffer is not empty\n");
				goto error_free;
			}
		}

		/* Check if the read data contains a newline character */
		nl_pos = strnpos(linebuf+linebuf_nbytes, '\n', readret);

		/* If a newline was found, get its absolute position */
		if(nl_pos >= 0) {
			nl_pos += linebuf_nbytes;
		}

		linebuf_nbytes += readret;
	}

	/* Found newline character */

	/* Copy characters located after the newline to the (external) buffer */
	rem_nbytes = linebuf_nbytes - nl_pos - 1;
	if (rem_nbytes > 0) {
		strncpy(buffer->buf, linebuf+nl_pos+1, rem_nbytes);
		buffer->nbytes = rem_nbytes;
	}

	/* Ignore '\r' before '\n' to handle also "\r\n" sequences */
	if ( (nl_pos > 0) && (linebuf[nl_pos-1] == '\r') ) {
		nl_pos--;
	}

	/* Replace '\n' (or '\r' in front of a '\n') by
	 * '\0' to terminate the string */
	linebuf[nl_pos] = '\0';

	/* Set dst pointer and return string length */
	*dst = linebuf;
	return nl_pos;

error_free:
	free(linebuf);

error_ret:
	*dst = NULL;
	return -1;
}

int creadline(int fd, void **dst)
{
	static int fd_last = -1;
	static creadline_buf_t buffer;

	/* Reset buffer if a new fd is used */
	if(fd_last != fd) {
		buffer.nbytes = 0;
		fd_last = fd;
	}

	return creadline_r(fd, dst, &buffer);
}

