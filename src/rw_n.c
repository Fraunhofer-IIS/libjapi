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
 * \copyright
 * Copyright (c) 2018 Fraunhofer IIS.
 * All rights reserved.
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
