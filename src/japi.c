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

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h> /* strcmp */
#include <strings.h> /* strcasecmp */
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "japi.h"

#include "creadline.h"
#include "japi_intern.h"
#include "japi_pushsrv.h"
#include "japi_pushsrv_intern.h"
#include "japi_utils.h"
#include "networking.h"
#include "prntdbg.h"
#include "rw_n.h"

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
int japi_process_message(japi_context *ctx, const char *request, char **response,
						 int socket)
{
	const char *req_name;
	json_object *jreq;
	json_object *jreq_no;
	json_object *jresp;
	json_object *jresp_data;
	json_object *jargs;
	japi_req_handler req_handler;
	int ret;
	bool args;

	assert(ctx != NULL);
	assert(response != NULL);
	assert(socket >= 0);

	ret = -1;
	*response = NULL;

	/* Create JSON object from received message */
	jreq = json_tokener_parse(request);
	if (jreq == NULL) {
		fprintf(stderr, "ERROR: json_tokener_parse() failed. Received message: %s\n",
				request);
		return -1;
	}

	/* Only create new JSON objects after a valid JSON request was parsed. */
	jresp = json_object_new_object(); /* Response object */
	jresp_data = json_object_new_object();

	if ((japi_get_value_as_str(jreq, "japi_request", &req_name)) == 0) {

		/* Prepare response */
		json_object_object_add(jresp, "japi_response",
							   json_object_new_string(req_name));

		/* Include japi_request_no in response, if included with request */
		if (json_object_object_get_ex(jreq, "japi_request_no", &jreq_no)) {
			json_object_get(jreq_no);
			json_object_object_add(jresp, "japi_request_no", jreq_no);
		}

		/* Get arguments as an JSON object */
		args = json_object_object_get_ex(jreq, "args", &jargs);

		/* Add an empty args JSON object if no args were given
		   Otherwise, include args with response, if configured. */
		if (!args) {
			json_object_object_add(jreq, "args", NULL);
			json_object_object_get_ex(jreq, "args", &jargs);
		} else {
			if (ctx->include_args_in_response) {
				json_object_get(jargs);
				json_object_object_add(jresp, "args", jargs);
			}
		}

		/* Subscribe/unsubscribe service needs client socket */
		if (strcasecmp(req_name, "japi_pushsrv_subscribe") == 0 ||
			strcasecmp(req_name, "japi_pushsrv_unsubscribe") == 0) {
			json_object_object_add(jargs, "socket", json_object_new_int(socket));
		}

		/* Try to find a suitable handler for the given request */
		req_handler = japi_get_request_handler(ctx, req_name);
		if (req_handler == NULL) {

			/* No request handler found? Check if a fallback handler was registered. */
			req_handler = japi_get_request_handler(ctx, "request_not_found_handler");

			if (req_handler == NULL) {
				fprintf(stderr,
						"ERROR: No suitable request handler found. Falling back to "
						"default fallback handler. Request was: %s\n",
						req_name);
				req_handler =
					japi_get_request_handler(ctx, "japi_request_not_found_handler");
			} else {
				fprintf(stderr,
						"WARNING: No suitable request handler found. Falling back to "
						"user registered fallback handler. Request was: %s\n",
						req_name);
			}
		}

		/* Call request handler */
		req_handler(ctx, jargs, jresp_data);

	} else {
		/* Get request name */
		if (req_name == NULL) {
			fprintf(stderr, "ERROR: No keyword found!\n");
			goto out_free;
		}
	}

	/* Add response arguments */
	json_object_object_add(jresp, "data", jresp_data);

	/* Stringify response */
	*response = japi_get_jobj_as_ndstr(jresp);
	json_object_put(jresp);

	ret = 0;

out_free:
	/* Free JSON request object */
	json_object_put(jreq);

	return ret;
}

int japi_shutdown(japi_context *ctx)
{
	if (ctx == NULL) {
		fprintf(stderr, "ERROR: JAPI context is NULL.\n");
		return -1;
	}

	ctx->shutdown = true;

	return 0;
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

	psc = ctx->push_services;
	while (psc != NULL) {
		psc_next = psc->next;
		japi_pushsrv_destroy(ctx, psc);
		psc = psc_next;
	}

	pthread_mutex_destroy(&(ctx->lock));
	free(ctx);

	return 0;
}

int japi_register_request(japi_context *ctx, const char *req_name,
						  japi_req_handler req_handler)
{
	japi_request *req;
	char *bad_req_name = "japi_";

	/* Error handling */
	if (ctx == NULL) {
		fprintf(stderr, "ERROR: JAPI context is NULL.\n");
		return -1;
	}

	if ((req_name == NULL) || (strcmp(req_name, "") == 0)) {
		fprintf(stderr, "ERROR: Request name is NULL or empty.\n");
		return -2;
	}

	if (req_handler == NULL) {
		fprintf(stderr, "ERROR: Request handler is NULL.\n");
		return -3;
	}

	if (japi_get_request_handler(ctx, req_name) != NULL) {
		fprintf(stderr,
				"ERROR: A request handler called '%s' was already registered.\n",
				req_name);
		return -4;
	}

	if (ctx->init && strncmp(req_name, bad_req_name, strlen(bad_req_name)) == 0) {
		fprintf(stderr, "ERROR: Request name is not allowed.\n");
		return -6;
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

japi_context *japi_init(void *userptr)
{
	japi_context *ctx;

	ctx = (japi_context *)malloc(sizeof(japi_context));
	if (ctx == NULL) {
		perror("ERROR: malloc() failed");
		return NULL;
	}

	ctx->init = false;
	ctx->userptr = userptr;
	ctx->requests = NULL;
	ctx->push_services = NULL;
	ctx->clients = NULL;
	ctx->num_clients = 0;
	ctx->max_clients = 0;
	ctx->include_args_in_response = false;
	ctx->shutdown = false;

	/* Initialize mutex */
	if (pthread_mutex_init(&(ctx->lock), NULL) != 0) {
		fprintf(stderr, "ERROR: mutex initialization has failed\n");
		return NULL;
	}

	/* Ignore SIGPIPE Signal */
	signal(SIGPIPE, SIG_IGN);

	/* Register the default fallback handler */
	japi_register_request(ctx, "japi_request_not_found_handler",
						  &japi_request_not_found_handler);
	/* Register subscribe/unsubscribe service function */
	japi_register_request(ctx, "japi_pushsrv_subscribe", &japi_pushsrv_subscribe);
	japi_register_request(ctx, "japi_pushsrv_unsubscribe", &japi_pushsrv_unsubscribe);
	/* Register list_push_service function  */
	japi_register_request(ctx, "japi_pushsrv_list", &japi_pushsrv_list);
	japi_register_request(ctx, "japi_cmd_list", &japi_cmd_list);

	ctx->init = true;

	return ctx;
}

/*
 * Set maximal allowed number of clients.
 * 0 is interpreted as unlimited number of allowed clients.
 */
int japi_set_max_allowed_clients(japi_context *ctx, uint16_t num)
{
	/* Error handling */
	if (ctx == NULL) {
		fprintf(stderr, "ERROR: JAPI context is NULL.\n");
		return -1;
	}

	ctx->max_clients = num;

	return 0;
}

/*
 * Include request arguments in response.
 */
int japi_include_args_in_response(japi_context *ctx, bool include_args)
{
	/* Error handling */
	if (ctx == NULL) {
		fprintf(stderr, "ERROR: JAPI context is NULL.\n");
		return -1;
	}

	ctx->include_args_in_response = include_args;

	return 0;
}

/*
 * Add new client element to list
 */
int japi_add_client(japi_context *ctx, int socket)
{
	japi_client *client;

	/* Error handling */
	assert(ctx != NULL);
	assert(socket >= 0);

	/* Create new client list element */
	client = (japi_client *)malloc(sizeof(japi_client));
	if (client == NULL) {
		perror("ERROR: malloc() failed");
		return -1;
	}

	/* Reset the clients buffer used by creadline_r */
	client->crl_buffer.nbytes = 0;

	pthread_mutex_lock(&(ctx->lock));
	prntdbg("adding client %d to japi context\n", socket);
	/* Add socket */
	client->socket = socket;

	/* Link list */
	client->next = ctx->clients;
	ctx->clients = client;
	/* Increment number of connected clients */
	ctx->num_clients++;
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
	assert(socket >= 0);

	client = ctx->clients;
	prev = NULL;
	ret = -1;

	japi_pushsrv_remove_client_from_all_pushsrv(ctx, socket);

	pthread_mutex_lock(&(ctx->lock));
	/* Remove client from list */
	while (client != NULL) {
		/* If first element */
		if ((client->socket == socket) && (prev == NULL)) {
			ctx->clients = client->next;
			prntdbg("removing client %d from japi context and close socket\n",
					client->socket);
			close(client->socket);
			free(client);
			ctx->num_clients--;
			ret = 0;
			break;
		}
		/* If last element */
		if ((client->socket == socket) && (client->next == NULL)) {
			prev->next = NULL;
			prntdbg("removing client %d from japi context and close socket\n",
					client->socket);
			close(client->socket);
			free(client);
			ctx->num_clients--;
			ret = 0;
			break;
		}
		if (client->socket == socket) {
			prev->next = client->next;
			prntdbg("removing client %d from japi context and close socket\n",
					client->socket);
			close(client->socket);
			free(client);
			ctx->num_clients--;
			ret = 0;
			break;
		}

		prev = client;
		client = client->next;
	}
	pthread_mutex_unlock(&(ctx->lock));

	return ret;
}

int japi_remove_all_clients(japi_context *ctx)
{
	japi_client *client, *following_client;

	/* Error Handling */
	assert(ctx != NULL);

	client = ctx->clients;
	while (client != NULL) {
		following_client = client->next;
		if (japi_remove_client(ctx, client->socket) != 0) {
			return -1;
		}
		client = following_client;
	}

	return 0;
}

int japi_start_server(japi_context *ctx, const char *port)
{
	int server_socket;

	server_socket = tcp_start_server(port);
	if (server_socket < 0) {
		fprintf(stderr, "ERROR: Failed to start tcp server on port %s\n", port);
		return -1;
	}

	int ret;
	int nfds;
	fd_set fdrd;
	struct timeval timeout;
	japi_client *client, *following_client;

	if (listen(server_socket, 1) != 0) {
		perror("ERROR: listen() failed\n");
		return -1;
	}

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

		timeout.tv_sec = 0;
		timeout.tv_usec = 200000;
		ret = select(nfds, &fdrd, NULL, NULL, &timeout);
		if (ret == -1) {
			perror("ERROR: select() failed\n");
			return -1;
		}

		/* Check if there is a request to shutdown the server */
		if (ctx->shutdown == true) {
			break;
		} else if (ret == 0) {
			continue;
		}

		client = ctx->clients; // Reset pointer to list

		while (client != NULL) {

			following_client = client->next;

			/* Check whether there is data to process */
			if (FD_ISSET(client->socket, &fdrd)) {

				int ret;
				char *request;
				char *response;

				do {

					ret = creadline_r(client->socket, (void **)&request,
									  &(client->crl_buffer));
					if (ret > 0) {

						response = NULL;

						/* Received a line, process it... */
						japi_process_message(ctx, request, &response, client->socket);

						/* After the request buffer is processed, the memory
						 *is not needed anymore and can be freed at this point. */
						free(request);

						/* Send response (if provided) */
						if (response != NULL) {
							ret = write_n(client->socket, response, strlen(response));
							free(response);

							if (ret <= 0) {
								/* Write failed */
								fprintf(stderr,
										"ERROR: Failed to send response to client %i "
										"(write returned %i)\n",
										client->socket, ret);
								japi_remove_client(ctx, client->socket);
								break;
							}
						}
					} else if (ret == 0) {
						if (request == NULL) {
							/* Received EOF (client disconnected) */
							prntdbg("client %d disconnected\n", client->socket);
							japi_remove_client(ctx, client->socket);
							break;
						} else {
							/* Received an empty line */
							free(request);
						}
					} else {
						fprintf(stderr, "ERROR: creadline() failed (ret = %i)\n", ret);
						japi_remove_client(ctx, client->socket);
						break;
					}

				} while (client->crl_buffer.nbytes != 0);
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
			if (ctx->max_clients == 0 || ctx->num_clients < ctx->max_clients) {
				japi_add_client(ctx, client_socket);
				prntdbg("client %d added\n", client_socket);
			} else {
				close(client_socket);
			}
		}
	}

	/* Clean up */
	japi_remove_all_clients(ctx);

	close(server_socket);

	return 0;
}

/*
 * Provide the names of all registered commands as a JAPI response.
 */
void japi_cmd_list(japi_context *ctx, json_object *request, json_object *response)
{
	japi_request *req;
	json_object *jstring;
	json_object *jarray;

	assert(ctx != NULL);
	assert(response != NULL);

	jarray = json_object_new_array();
	req = ctx->requests;

	/* Iterate through push service list and return JSON object  */
	while (req != NULL) {
		jstring = json_object_new_string(req->name); /* Create JSON-string */
		json_object_array_add(jarray, jstring); /* Add string to JSON array */
		req = req->next;
	}

	/* Add array to JSON-object */
	json_object_object_add(response, "commands", jarray);
}

/*
 * Default handler for reacting to unknown requests.
 */
void japi_request_not_found_handler(japi_context *ctx, json_object *request,
									json_object *response)
{
	json_object_object_add(response, "error",
						   json_object_new_string("no request handler found"));
}