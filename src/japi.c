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

#include "rw_n.h"
#include "networking.h"
#include "creadline.h"

/* Look for a JSON object with the key 'key' and return its value as a string.
 *
 * NULL is returned if there is no object with a key 'key' or if the value is
 * not a string type.
 */
static const char* japi_get_value_as_str(json_object *jobj, const char *key)
{
	json_object *jcmd;

	if (json_object_object_get_ex(jobj, key, &jcmd)) {
		if (json_object_is_type(jcmd, json_type_string)) {
			return json_object_get_string(jcmd);
		}
	}

	return NULL;
}

/* Look for a command handler matching the name 'name'.
 *
 * NULL is returned if no suitable handler was found.
 */
static japi_cmd_handler japi_get_command_handler(japi_context *ctx, const char *name)
{
	japi_command *cmd;

	cmd = ctx->commands;
	while (cmd != NULL) {

		if (strcasecmp(name, cmd->name) == 0) {
			return cmd->func;
		}
		cmd = cmd->next;
	}

	return NULL;
}

/* Stringify the JSON object, copy it to a new allocated buffer and append a
 * newline. This is necessary to ensure that a call to write() will send the
 * message at once, meaning the message and the newline are not separated in
 * two strings.
 *
 * The caller is responsible for freeing the returned string.
 */
static char* japi_get_response_as_str(json_object *jobj)
{
	const char *tmp_str;
	char *response;
	size_t tmp_str_len;

	tmp_str = json_object_to_json_string(jobj);
	tmp_str_len = strlen(tmp_str);

	response = malloc(strlen(tmp_str)+2);
	if (response == NULL) {
		perror("malloc");
		return NULL;
	}

	strcpy(response, tmp_str);
	response[tmp_str_len+0] = '\n';
	response[tmp_str_len+1] = '\0';

	return response;
}

/* Steps performed while processing a JSON request:
 * - Convert the received message into a JSON object
 * - Extract the command name
 * - Search a suitable command handler
 * - Call the command handler
 * - Prepare the JSON response
 * - Free memory
 */
static int japi_process_request(japi_context *ctx, const char *request, char **response)
{
	const char* cmd_name;
	json_object *jreq;
	json_object *jresp;
	japi_cmd_handler cmd_handler;
	int ret;

	ret = -1;
	*response = NULL;

	/* Create JSON object from received message */
	jreq = json_tokener_parse(request);
	if (jreq == NULL) {
		fprintf(stderr, "ERROR: json_tokener_parse() failed. Received message: %s\n", request);
		return -1;
	}

	/* Get command name */
	cmd_name = japi_get_value_as_str(jreq, "JAPI_COMMAND");
	if (cmd_name == NULL) {
		fprintf(stderr, "ERROR: JAPI_COMMAND not found!\n");
		goto out_free;
	}

	/* Try to find a suitable handler for the given command */
	cmd_handler = japi_get_command_handler(ctx, cmd_name);
	if (cmd_handler == NULL) {
		fprintf(stderr, "ERROR: No suitable command handler found. Command was: %s\n", cmd_name);
		goto out_free;
	}

	/* Call command handler */
	jresp = NULL;
	cmd_handler(ctx, jreq, &jresp);

	/* Prepare response */
	if (jresp != NULL) {
		*response = japi_get_response_as_str(jresp);
		json_object_put(jresp);
	}

	ret = 0;

out_free:
	/* Free JSON request object */
	json_object_put(jreq);

	return ret;
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
	ctx->commands = NULL;

	return ctx;
}

void japi_destroy(japi_context *ctx)
{
	japi_command *cmd, *cmd_next;

	if (ctx != NULL) {

		cmd = ctx->commands;
		while (cmd != NULL) {
			cmd_next = cmd->next;
			free(cmd);
			cmd = cmd_next;
		}

		free(ctx);
	}
}

int japi_register_command(japi_context* ctx, const char *cmd_name, japi_cmd_handler cmd_handler)
{
	japi_command *cmd;

	cmd = malloc(sizeof(japi_command));
	if (cmd == NULL) {
		perror("malloc() failed");
		return -1;
	}

	cmd->name = cmd_name;
	cmd->func = cmd_handler;
	cmd->next = ctx->commands;

	ctx->commands = cmd;

	return 0;
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

			/* Check whether there are new clients */
			if (FD_ISSET(server_socket, &fdrd)) {

				/* Drop new client immediately since we can
				 * only handle one connection at a time */
				int tmp_sock;
				tmp_sock = accept(server_socket, NULL, NULL);
				close(tmp_sock);
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
					japi_process_request(ctx, request, &response);

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
						continue;
					}
				} else {
					fprintf(stderr, "ERROR: creadline() failed (ret = %i)\n", ret);
					close(client_socket);
					break;
				}
			}
		}
	}

	close(server_socket);

	return 0;
}

