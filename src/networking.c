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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "networking.h"

/* taken from getaddrinfo manpage and slightly adapted */
static int tcp_start_server_on_addr_family(const char* port, int ai_family)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s;
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = ai_family;      /* Address family specified by user */
	hints.ai_socktype = SOCK_STREAM;  /* Stream socket */
	hints.ai_flags = AI_PASSIVE;      /* For wildcard IP address */
	hints.ai_protocol = 0;            /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	
	s = getaddrinfo(NULL, port, &hints, &result);
	if (s != 0) {
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
	    return -1;
	}
	
	/* getaddrinfo() returns a list of address structures.
	   Try each address until we successfully bind(2).
	   If socket(2) (or bind(2)) fails, we (close the socket
	   and) try the next address. */
	
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
	    	continue;

		/* Sometimes after a server shutdown the server socket can get
		into TIME_WAIT state and it is no longer possible to use it again.
		With SO_REUSEADDR enabled, the server socket can be reused
		instantaneously after a shutdown. setsockopt() needs to be called
		before bind. */

		int reuse = 1; /* Set SO_REUSEADDR to 1(True) */
		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
			perror("ERROR: setsocktop");
			fprintf(stderr, "WARNING: Reuse of server socket wont be possible. \n");
			/* The programm can go on. */
		};

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
			break;                  /* Success */
	
		close(sfd);
	}

	freeaddrinfo(result);           /* No longer needed */

	if (rp == NULL) {               /* No address succeeded */
		fprintf(stderr, "Could not bind\n");
		return -1;
	}

	return sfd;
}

int tcp_start_server(const char* port)
{
	return tcp_start_server_on_addr_family(port, AF_UNSPEC);
}

int tcp4_start_server(const char* port)
{
	return tcp_start_server_on_addr_family(port, AF_INET);
}

int tcp6_start_server(const char* port)
{
	return tcp_start_server_on_addr_family(port, AF_INET6);
}

