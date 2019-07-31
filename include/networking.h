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
 * \copyright
 * Copyright (c) 2018 Fraunhofer IIS.
 * All rights reserved.
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

#ifdef __cplusplus
}
#endif

#endif /* __NETWORKING_H__ */

