/*!
 * \file
 * \author Christopher Stender
 * \date 2018-02-15
 * \version 0.1
 *
 * \brief Networking helper functions
 *
 * \details
 * This module collects networking helper functions like tcp_start_server().
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

#ifndef __NETWORKING_H__
#define __NETWORKING_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Start a new TCP server.
 *
 * A new TCP server is started on a user specified port. Therefore a new socket
 * is created and an address is bound to it.
 *
 * \param port Port to listen on for incoming TCP connections.
 *
 * \returns On success, a file descriptor for the new socket is returned. On
 *          error, -1 is returned and errno ist set appropriately.
 */
int tcp_start_server(const char* port);

/*!
 * \brief Start a new TCP server using IPv4.
 *
 * A new TCP server is started on a user specified port using IPv4. Therefore a
 * new socket is created and an address is bound to it.
 *
 * \param port Port to listen on for incoming TCP connections.
 *
 * \returns On success, a file descriptor for the new socket is returned. On
 *          error, -1 is returned and errno ist set appropriately.
 */
int tcp4_start_server(const char* port);

/*!
 * \brief Start a new TCP server using IPv6.
 *
 * A new TCP server is started on a user specified port using IPv6. Therefore a
 * new socket is created and an address is bound to it.
 *
 * \param port Port to listen on for incoming TCP connections.
 *
 * \returns On success, a file descriptor for the new socket is returned. On
 *          error, -1 is returned and errno ist set appropriately.
 */
int tcp6_start_server(const char* port);

/*!
 * \brief Set socket options to enable TCP keepalive functions
 * 
 * Change default values to ensure that lost connections are recognised and
 * terminated.
 * 
 * \param socket_file_descriptor Socket to which the changes are applied.
 * 
 * \returns On success 0. On error -1.
 */
int enable_tcp_keepalive(int socket_file_descriptor);

#ifdef __cplusplus
}
#endif

#endif /* __NETWORKING_H__ */

