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

#include <stdio.h> /* printf, fprintf */
#include <string.h> /* strcasecmp */
#include <sys/socket.h>
#include <unistd.h>

#include "japi.h"

#include "creadline.h"
#include "japi_pushsrv.h"
#include "japi_utils.h"
#include "rw_n.h"
#include "networking.h"

/* Look for a JSON object with the key 'key' and return its value as a string.
 *
 * NULL is returned if there is no object with a key 'key' or if the value is
 * not a string type.
 */
static const char* japi_get_value_as_str(json_object *jobj, const char *key)
{
	json_object *jval;

	if (json_object_object_get_ex(jobj, key, &jval)) {
		if (json_object_is_type(jval, json_type_string)) {
			return json_object_get_string(jval);
		}
	}

	return NULL;
}

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
static int japi_process_message(japi_context *ctx, const char *request, char **response, int socket)
{
	const char* req_name;
	const char* pushsrv_name;
	json_object *jreq;
	json_object *jresp;
	japi_req_handler req_handler;
	int ret;

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

void japi_destroy(japi_context *ctx)
{
	japi_request *req, *req_next;

	if (ctx != NULL) {

		req = ctx->requests;
		while (req != NULL) {
			req_next = req->next;
			free(req);
			req = req_next;
		}

		free(ctx);
	}
}

int japi_register_request(japi_context* ctx, const char *req_name, japi_req_handler req_handler)
{
	japi_request *req;

	req = malloc(sizeof(japi_request));
	if (req == NULL) {
		perror("malloc() failed");
		return -1;
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

	ctx = malloc(sizeof(japi_context));
	if (ctx == NULL) {
		perror("malloc() failed");
		return NULL;
	}

	ctx->userptr = userptr;
	ctx->socket = -1;
	ctx->requests = NULL;
	ctx->push_services = NULL;

	/* Register list_push_service function  */
	japi_register_request(ctx, "japi_pushsrv_list", &japi_pushsrv_list);

	return ctx;
}

int japi_start_server(japi_context *ctx, const char *port)
{
	int client_socket;
	int server_socket;

	server_socket = tcp_start_server(port);
	if (server_socket < 0) {
		fprintf(stderr, "Error: Failed to start tcp server on port %s\n", port);
		return -1;
	}

	while (1) {

		int ret;
		int nfds;
		fd_set fdrd;

		ctx->socket = -1;

		if (listen(server_socket, 1) != 0) {
			perror("listen() failed");
			return -1;
		}

		client_socket = accept(server_socket, NULL, NULL);
		if (client_socket < 0) {
			perror("accept() failed");
			return -1;
		}

		ctx->crl_buffer.nbytes = 0;
		ctx->socket = client_socket;

		while (1) {

			FD_ZERO(&fdrd);
			FD_SET(client_socket, &fdrd);
			FD_SET(server_socket, &fdrd);

			nfds = (server_socket < client_socket) ? client_socket+1 : server_socket+1;

			ret = select(nfds, &fdrd, NULL, NULL, NULL);
			if (ret == -1) {
				perror("select() failed");
				return -1;
			}

			/* Check whether there is data to process */
			if (FD_ISSET(client_socket, &fdrd)) {

				int ret;
				char* request;
				char* response;

				ret = creadline_r(client_socket, (void**)&request, &(ctx->crl_buffer));
				if (ret > 0) {

					response = NULL;

					/* Received a line, process it... */
					japi_process_message(ctx, request, &response, client_socket);

					/* Send response (if provided) */
					if (response != NULL) {
						ret = write_n(client_socket, response, strlen(response));
						free(response);

						if (ret <= 0) {
							/* Write failed */
							fprintf(stderr, "ERROR: Failed to send response to client (write returned %i)\n", ret);
							close(client_socket);
							break;
						}
					}

					free(request);

				} else if (ret == 0) {
					if(request == NULL) {
						/* Received EOF (client disconnected) */
						close(client_socket);
						break;
					} else {
						/* Received an empty line */
						free(request);
					}
				} else {
					fprintf(stderr, "ERROR: creadline() failed (ret = %i)\n", ret);
					close(client_socket);
					break;
				}
			}

			/* Check whether there are new clients */
			if (FD_ISSET(server_socket, &fdrd)) {

				/* Drop new client immediately since we can
				 * only handle one connection at a time */
				int tmp_sock;
				tmp_sock = accept(server_socket, NULL, NULL);
				close(tmp_sock);
			}

		}
	}
	japi_pushsrv_shutdown_all(ctx);
	close(server_socket);

	return 0;
}
