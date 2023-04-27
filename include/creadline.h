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
 *\copyright
 * Copyright (c) 2023 Fraunhofer IIS
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __CREADLINE_H__
#define __CREADLINE_H__

/*! Override the maximum line size here (default: 64 MiB) */
//#define CREADLINE_MAX_LINE_SIZE 10*1024*1024

/*! Define creadline's block size.
 *
 * A small block size leads to computation overhead while a large block size
 * may waste some memory. A good value might be 1024, 2048 or 4096.
 */
#define CREADLINE_BLOCK_SIZE 1024

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Buffer type for storing remaining bytes.
 */
typedef struct __creadline_buffer {
	char buf[CREADLINE_BLOCK_SIZE]; /*!< buffer for storing remaining bytes */
	int nbytes;                     /*!< number of bytes stored in the buffer */
} creadline_buf_t;

/*!
 * \brief Read a single line from a file descriptor (reentrant version).
 *
 * creadline_r reads a single line from a file descriptor (e.g. a socket). Read
 * characters are stored in internally allocated memory. If a newline character is
 * found remaining bytes are moved to the provided buffer and the dst pointer is
 * modified to point to the read line. The length of the \0 terminated line is
 * returned (excluding the '\0').
 * The caller is responsible for free'ing the memory dst is pointing to!
 *
 * Subsequent calls to creadline_r reuse the remaining bytes (if any) and
 * continue reading from the file descriptor.
 *
 * If EOF is read creadline_r returns 0 and sets the dst pointer to NULL if
 * no remaining bytes are left in the buffer. Otherweise -1 is returned.
 *
 * If a zero length line is read ("\n") creadline_r returns 0 and dst points
 * to a terminated string "\0".
 *
 * \param fd		File descriptor
 * \param dst		Pointer to a pointer to the read line
 * \param buffer	creadline buffer for storing remaining characters
 *
 * \returns  -1 on error,
 *            0 on EOF or when a zero-length line was read (check dst),
 *            length of the read line otherwise
 */
int creadline_r(int fd, void **dst, creadline_buf_t *buffer);

/*!
 * \brief Read a single line from a file descriptor.
 *
 * creadline behaves like creadline_r except that no buffer needs to be provided.
 * creadline is not reentrant and reading from a different file descriptor will
 * delete remaining bytes from a previous read. If you need to read lines from
 * different file descriptors use creadline_r instead.
 *
 * \param fd		File descriptor
 * \param dst		Pointer to a pointer to the read line
 *
 * \returns  -1 on error,
 *            0 on EOF or when a zero-length line was read (check dst),
 *            length of the read line otherwise
 */
int creadline(int fd, void **dst);

#ifdef __cplusplus
}
#endif

#endif /* __CREADLINE_H__ */
