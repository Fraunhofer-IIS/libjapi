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

#ifndef __RW_N_H__
#define __RW_N_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Read a fixed number of bytes from a file descriptor.
 *
 * read_n reads count bytes from a file descriptor fd into the buffer starting
 * at buf. If the underlying read returns with less than count bytes read,
 * read_n() calls read() again until all bytes are read or an error or EOF
 * occurs.
 *
 * \param fd		File descriptor
 * \param buf		Pointer to the target buffer
 * \param count		Number of bytes to copy
 *
 * \returns		On success, the number of bytes read is returned (zero
 *				indicates end of file). On error, -1 is	returned, and errno
 *				is set appropriately.
 */
int read_n(int fd, void* buf, size_t count);

/*!
 * \brief Write a fixed number of bytes to a file descriptor.
 *
 * write_n writes count bytes from the buffer buf to the file descriptor fd.
 * If the underlying write() returns with less than count bytes written,
 * write_n() calls write() again until all bytes are written or an error
 * occurs.
 *
 * \param fd		File descriptor
 * \param buf		Pointer to the target buffer
 * \param count		Number of bytes to copy
 *
 * \returns		On success, the number of bytes written is returned. On
 *				error, -1 is returned, and errno is set appropriately.
 */
int write_n(int fd, const void* buf, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* __RW_N_H__ */
