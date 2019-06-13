/*!
 * \file
 * \author Christopher Stender
 * \date 2018-02-15
 * \version 0.1
 *
 * \brief Universal JSON API library.
 *
 * \details
 * libjapi is a universal JSON API library.
 *
 * \copyright
 * Copyright (c) 2018 Fraunhofer IIS.
 * All rights reserved.
 */

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h> /* strcasecmp */
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "japi.h"

#include "creadline.h"
#include "japi_intern.h"
#include "japi_pushsrv_intern.h"
#include "japi_pushsrv.h"
#include "japi_utils.h"
#include "rw_n.h"
#include "networking.h"
#include "prntdbg.h"


/* Look for a request handler matching the name 'name'.
 *
 * NULL is returned if no suitable handler was found.
 */
static japi_req_handler japi_get_request_handler(japi_context *ctx, const char *name)
{
	japi_request *req;

	req = ctx->requests;
	while (req != NULL) {

		if (strcasecmp(name, req->name) == 0) {
			return req->func;
		}
		req = req->next;
	}

	return NULL;
}

/* Steps performed while processing a JSON request:
 * - Convert the received message into a JSON object
 * - Extract the request name
 * - Search a suitable request handler
 * - Call the request handler
 * - Prepare the JSON response
 * - Free memory
 */
int japi_process_message(japi_context *ctx, const char *request, char **response, int socket)
{
	const char* req_name;
	const char* pushsrv_name;
	json_object *jreq;
	json_object *jresp;
	japi_req_handler req_handler;
	int ret;

	assert(ctx != NULL);
	assert(request != NULL);
	assert(response != NULL);
	assert(socket != -1);

	ret = -1;
	*response = NULL;
	jresp = json_object_new_object(); /* Response object */

	/* Create JSON object from received message */
	jreq = json_tokener_parse(request);
	if (jreq == NULL) {
		fprintf(stderr, "ERROR: json_tokener_parse() failed. Received message: %s\n", request);
		return -1;
	}

	/* Look for subscribe/unsubscribe service and add/remove client socket if found */
	if ((pushsrv_name = japi_get_value_as_str(jreq, "japi_pushsrv_subscribe")) != NULL) {
		japi_pushsrv_subscribe(ctx,socket,pushsrv_name,jresp);
	} else if ((pushsrv_name = japi_get_value_as_str(jreq, "japi_pushsrv_unsubscribe")) != NULL) {
		japi_pushsrv_unsubscribe(ctx,socket,pushsrv_name,jresp);
	} else if ((req_name = japi_get_value_as_str(jreq, "japi_request")) != NULL) {
		/* Try to find a suitable handler for the given request */
		req_handler = japi_get_request_handler(ctx, req_name);
		if (req_handler == NULL) {

			/* No request handler found? Check if a fallback handler was registered. */
			req_handler = japi_get_request_handler(ctx, "request_not_found_handler");

			if (req_handler == NULL) {
				fprintf(stderr, "ERROR: No suitable request handler found. Request was: %s\n", req_name);
				goto out_free;
			} else {
				fprintf(stderr, "WARNING: No suitable request handler found. Falling back to registered fallback handler. Request was: %s\n", req_name);
			}
		}

		/* Prepare response */
		json_object_object_add(jresp, "japi_response", json_object_new_string(req_name));

		/* Call request handler */
		req_handler(ctx, jreq, jresp);
	} else {
		/* Get request name */
		if (req_name == NULL) {
			fprintf(stderr, "ERROR: No keyword found!\n");
			goto out_free;
		}
	}

	/* Stringify response */
	*response = japi_get_jobj_as_ndstr(jresp);
	json_object_put(jresp);

	ret = 0;

out_free:
	/* Free JSON request object */
	json_object_put(jreq);

	return ret;
}

int japi_destroy(japi_context *ctx)
{
	japi_request *req, *req_next;
	japi_pushsrv_context *psc, *psc_next;

	if (ctx == NULL) {
		fprintf(stderr, "ERROR: JAPI context is NULL.\n");
		return -1;
	}

	req = ctx->requests;
	while (req != NULL) {
		req_next = req->next;
		free(req);
		req = req_next;
	}
	free(ctx);

	psc = ctx->push_services;
	while (psc != NULL) {
		psc_next = psc->next;
		pthread_mutex_destroy(&(psc->lock));
		japi_pushsrv_destroy(psc);
		psc = psc_next;
	}

	return 0;
}

int japi_register_request(japi_context* ctx, const char *req_name, japi_req_handler req_handler)
{
	japi_request *req;

	/* Error handling */
	if (ctx == NULL) {
		fprintf(stderr, "ERROR: JAPI context is NULL.\n");
		return -1;
	}

	if ((req_name == NULL) || (strcmp(req_name,"") == 0)) {
		fprintf(stderr, "ERROR: Request name is NULL or empty.\n");
		return -2;
	}

	if (req_handler == NULL) {
		fprintf(stderr, "ERROR: Request handler is NULL.\n");
		return -3;
	}

	if (japi_get_request_handler(ctx,req_name) != NULL) {
		fprintf(stderr,"ERROR: A request handler called '%s' was already registered.\n",req_name);
		return -4;
	}

	req = (japi_request *)malloc(sizeof(japi_request));
	if (req == NULL) {
		perror("ERROR: malloc() failed");
		return -5;
	}

	req->name = req_name;
	req->func = req_handler;
	req->next = ctx->requests;

	ctx->requests = req;

	return 0;
}

japi_context* japi_init(void *userptr)
{
	japi_context *ctx;

	ctx = (japi_context *)malloc(sizeof(japi_context));
	if (ctx == NULL) {
		perror("ERROR: malloc() failed");
		return NULL;
	}

	ctx->userptr = userptr;
	ctx->requests = NULL;
	ctx->push_services = NULL;
	ctx->clients = NULL;

	/* Initialize mutex */
	if (pthread_mutex_init(&(ctx->lock),NULL) != 0) {
		fprintf(stderr,"ERROR: mutex initialization has failed\n");
		return NULL;
	}

 	/* Ignore SIGPIPE Signal */
	signal(SIGPIPE, SIG_IGN);

	/* Register list_push_service function  */
	japi_register_request(ctx, "japi_pushsrv_list", &japi_pushsrv_list);

	return ctx;
}

/*
 * Add new client element to list
 */
int japi_add_client(japi_context *ctx, int socket)
{
	japi_client *client;

	/* Error Handling */
	assert(ctx != NULL);
	assert(socket > 0);

	/* Create new client list element */
	client = (japi_client *)malloc(sizeof(japi_client));
	if (client == NULL) {
		perror("ERROR: malloc() failed");
		return -1;
	}

	pthread_mutex_lock(&(ctx->lock));
	prntdbg("adding client %d to japi context\n",socket);
	/* Add socket */
	client->socket = socket;

	/* Link list */
	client->next = ctx->clients;
	ctx->clients = client;
	pthread_mutex_unlock(&(ctx->lock));

	return 0;
}

/*
 * Remove client from client list
 */
int japi_remove_client(japi_context *ctx, int socket)
{
	japi_client *client, *prev;
	int ret;

	/* Error Handling */
	assert(ctx != NULL);
	assert(socket > 0);

	client = ctx->clients;
	prev = NULL;
	ret = -1;

	japi_pushsrv_remove_client_from_all_pushsrv(ctx,socket);

	pthread_mutex_lock(&(ctx->lock));
	/* Remove client from list */
	while (client != NULL) {
		/* If first element */
		if ((client->socket == socket) && (prev == NULL)) {
			ctx->clients = client->next;
			prntdbg("removing client %d from japi context and close socket\n",client->socket);
			close(client->socket);
			free(client);
			ret = 0;
			break;
		}
		/* If last element */
		if ((client->socket == socket) && (client->next == NULL)) {
			prev->next = NULL;
			prntdbg("removing client %d from japi context and close socket\n",client->socket);
			close(client->socket);
			free(client);
			ret = 0;
			break;
		}
		if (client->socket == socket) {
			prev->next = client->next;
			prntdbg("removing client %d from japi context and close socket\n",client->socket);
			close(client->socket);
			free(client);
			ret = 0;
			break;
		}

		prev = client;
		client = client->next;
	}
	pthread_mutex_unlock(&(ctx->lock));

	return ret;
}

int japi_start_server(japi_context *ctx, const char *port)
{
	int server_socket;

	server_socket = tcp_start_server(port);
	if (server_socket < 0) {
		fprintf(stderr, "ERROR: Failed to start tcp server on port %s\n", port);
		return -1;
	}

	while (1) {

		int ret;
		int nfds;
		fd_set fdrd;
		japi_client *client, *following_client;

		if (listen(server_socket, 1) != 0) {
			perror("ERROR: listen() failed\n");
			return -1;
		}

		ctx->crl_buffer.nbytes = 0;

		while (1) {

			FD_ZERO(&fdrd);

			/* Add server socket to set */
			FD_SET(server_socket, &fdrd);

			nfds = server_socket + 1;

			/* Add all file descriptors to set */
			client = ctx->clients;
			while (client != NULL) {
				FD_SET(client->socket, &fdrd);
				if (client->socket >= nfds) {
					nfds = client->socket + 1;
				}
				client = client->next;
			}

			ret = select(nfds, &fdrd, NULL, NULL, NULL);
			if (ret == -1) {
				perror("ERROR: select() failed\n");
				return -1;
			}

			client = ctx->clients; // Reset pointer to list

			while (client != NULL) {

				following_client = client->next;

				/* Check whether there is data to process */
				if (FD_ISSET(client->socket, &fdrd)) {

					int ret;
					char* request;
					char* response;

					ret = creadline_r(client->socket, (void**)&request, &(ctx->crl_buffer));
					if (ret > 0) {

						response = NULL;

						/* Received a line, process it... */
						japi_process_message(ctx, request, &response, client->socket);

						/* Send response (if provided) */
						if (response != NULL) {
							ret = write_n(client->socket, response, strlen(response));
							free(response);

							if (ret <= 0) {
								/* Write failed */
								fprintf(stderr, "ERROR: Failed to send response to client %i (write returned %i)\n",client->socket, ret);
								japi_remove_client(ctx,client->socket);
							}
						}
					} else if (ret == 0) {
						if(request == NULL) {
							/* Received EOF (client disconnected) */
							prntdbg("client %d disconnected\n",client->socket);
							japi_remove_client(ctx,client->socket);
						} else {
							/* Received an empty line */
						}
					} else {
						fprintf(stderr, "ERROR: creadline() failed (ret = %i)\n", ret);
						japi_remove_client(ctx,client->socket);
					}
					free(request);
				}
				client = following_client;
			}

			/* Check whether there are new clients */
			if (FD_ISSET(server_socket, &fdrd)) {
				int client_socket = 0;

				client_socket = accept(server_socket, NULL, NULL);
				if (client_socket < 0) {
					perror("ERROR: accept() failed\n");
					return -1;
				}
				japi_add_client(ctx,client_socket);
			}

		}
	}
	close(server_socket);

	return 0;
}
