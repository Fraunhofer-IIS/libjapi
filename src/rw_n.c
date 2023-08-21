/*!
 * \file
 * \author Christopher Stender
 * \date 2018-02-15
 * \version 0.1
 *
 * \brief Read or write a fixed number of bytes
 *
 * \details
 * Use read_n() to read or write_n() to write a fixed number of bytes from or
 * to a file descriptor.
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

#include <unistd.h>

#include "rw_n.h"

int read_n(int fd, void* buf, size_t count)
{
	int bytes_read;
	int to_read = count;
	uint8_t* buffer = (uint8_t*)buf;

	while(to_read > 0) {
		bytes_read = read(fd, buffer, to_read);
		if(bytes_read <= 0)
			return bytes_read;

		buffer += bytes_read;
		to_read -= bytes_read;
	}

	return count;
}

int write_n(int fd, const void* buf, size_t count)
{
	int bytes_written;
	int to_send = count;
	uint8_t* buffer = (uint8_t*)buf;

	while(to_send > 0) {
		bytes_written = write(fd, buffer, to_send);
		if(bytes_written <= 0)
			return bytes_written;

		buffer += bytes_written;
		to_send -= bytes_written;
	}

	return count;
}
