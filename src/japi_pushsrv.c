/*!
 * \file
 * \author Deniz Armagan
 * \date 2019-03-27
 * \version 0.1
 *
 * \brief Universal JAPI Push Services library.
 *
 * \details
 * japi_pushsrv is a universal JSON API library.
 *
 * \copyright
 * Copyright (c) 2019 Fraunhofer IIS.
 * All rights reserved.
 */

#include <assert.h>
#include <json-c/json.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "japi_pushsrv.h"
#include "japi_utils.h"

#include "rw_n.h"

/*
 * Saves client socket, if passed push service is registered
 */
void japi_pushsrv_subscribe(japi_context *ctx, int socket, const char* pushsrv_name, json_object *jobj)
{
	japi_pushsrv_context *psc;

	/* Error handling */
	assert(ctx != NULL);
	assert(socket != -1);
	assert(pushsrv_name != NULL);
	assert(jobj != NULL);

	psc = ctx->push_services;

	/* Search for push service in list and save socket, if found */
	while (psc != NULL) {
		if (strcasecmp(pushsrv_name,psc->pushsrv_name) == 0) {
			psc->subscribed_clients[0] = socket;
			break;
		}
		psc = psc->next;
	}

	/* Create JSON response object */
	if (psc != NULL) {
		json_object_object_add(jobj,"japi_pushsrv_response",json_object_new_string(pushsrv_name));
		json_object_object_add(jobj,"success",json_object_new_boolean(TRUE));
	} else {
		json_object_object_add(jobj,"japi_pushsrv_response",json_object_new_string(pushsrv_name));
		json_object_object_add(jobj,"success",json_object_new_boolean(FALSE));
		json_object_object_add(jobj,"message",json_object_new_string("Push service not found."));
	}
}

/*
 * Removes client socket, if passed push service is registered
 */
void japi_pushsrv_unsubscribe(japi_context *ctx, int socket, const char* pushsrv_name, json_object *jobj)
{
	japi_pushsrv_context *psc;

	/* Error handling */
	assert(ctx != NULL);
	assert(socket != -1);
	assert(pushsrv_name != NULL);
	assert(jobj != NULL);

	psc = ctx->push_services;
	bool registered = false; /* Service registered? */
	bool unsubscribed = false; /* Service unsubscribed? */

	/* Search for push service in list and remove socket, if found & socket is set */
	while (psc != NULL) {
		if (strcasecmp(pushsrv_name,psc->pushsrv_name) == 0) {
			registered = true;
			if (psc->subscribed_clients[0] != -1) {
				psc->subscribed_clients[0] = -1;
				unsubscribed = true;
				break;
			}
		}
		psc = psc->next;
	}

	/* Create JSON response object */
	if (registered && unsubscribed) { /* Subscribed */
		json_object_object_add(jobj,"japi_pushsrv_response",json_object_new_string(pushsrv_name));
		json_object_object_add(jobj,"success",json_object_new_boolean(TRUE));
	} else if (registered && !unsubscribed) { /* Registered, but not subsrcibed */
		json_object_object_add(jobj,"japi_pushsrv_response",json_object_new_string(pushsrv_name));
		json_object_object_add(jobj,"success",json_object_new_boolean(FALSE));
		json_object_object_add(jobj,"message",json_object_new_string("Can't unsubscribe a service that wasn't subscribed before."));
	} else { /* Not registered */
		json_object_object_add(jobj,"japi_pushsrv_response",json_object_new_string(pushsrv_name));
		json_object_object_add(jobj,"success",json_object_new_boolean(FALSE));
		json_object_object_add(jobj,"message",json_object_new_string("Push service not found."));
	}
}

/* Check if there is a duplicate name for a request */
static bool pushsrv_isredundant(japi_context *ctx, const char *pushsrv_name)
{
	japi_pushsrv_context *psc;
	bool duplicate;

	duplicate = FALSE;
	psc = ctx->push_services;

	while (psc != NULL) {
		if (strcmp(psc->pushsrv_name, pushsrv_name) == 0) {
			duplicate = TRUE;
			break;
		}
		psc = psc->next;
	}
	return duplicate;
}

/* Free memory for duplicated push service name and element */
static void free_pushsrv(japi_pushsrv_context *psc)
{
	free(psc->pushsrv_name);
	free(psc);
}

/*
 * Registers push-service and returns pointer to that service object.
 */
japi_pushsrv_context* japi_pushsrv_register(japi_context* ctx, const char* pushsrv_name)
{
	japi_pushsrv_context *psc;

	if (ctx == NULL) {
		fprintf(stderr,"ERROR: JAPI context is NULL.\n");
		return NULL;
	}

	if ((pushsrv_name == NULL) || (strcmp(pushsrv_name,"") == 0)) {
		fprintf(stderr,"ERROR: Push service name is NULL or empty.\n");
		return NULL;
	}

	if (pushsrv_isredundant(ctx,pushsrv_name)) {
		fprintf(stderr,"ERROR: A push service called '%s' was already registered.\n",pushsrv_name);
		return NULL;
	}

	/* Reserve memory for japi_pushsrv_context struct */
	psc = (japi_pushsrv_context *)malloc(sizeof(japi_pushsrv_context));
	if (psc == NULL) {
		perror("ERROR: malloc() failed");
		return NULL;
	}

	/* Duplicate string in case that pushsrv_name comes from
	a namespace that is left after this function */
	psc->pushsrv_name = strdup(pushsrv_name);
	if (psc->pushsrv_name == NULL) {
		free(psc);
		return NULL;
	}

	/* Initialize struct */
	psc->subscribed_clients[0] = -1;
	psc->thread_id = 0;
	psc->routine = NULL;
	psc->enabled = false;

	/* Point to last struct */
	psc->next = ctx->push_services;
	ctx->push_services = psc;

	return psc;
}

/*
 * Iterate trough push services and free memory for all elements
 */
void japi_pushsrv_shutdown_all(japi_context *ctx)
{
	japi_pushsrv_context *psc, *psc_next;

	assert(ctx != NULL);

	psc = ctx->push_services;
	if (psc == NULL) {
		return;
	}

	/* Iterates trough push service list and frees memory for every element */
	do {
		psc_next = psc->next;
		free_pushsrv(psc);
	} while (psc_next != NULL);
}

/*
 * Provide the names of all registered push-services as a JAPI response.
 *
 * NOTE: Parameter 'request' declared, although not used in function.
 * Function declaration needs to be identical to respective handler.
 */
void japi_pushsrv_list(japi_context *ctx, json_object *request, json_object *response)
{
	japi_pushsrv_context *psc;
	json_object *jstring;
	json_object *jarray;

	assert(ctx != NULL);
	assert(response != NULL);

	jarray = json_object_new_array();
	psc = ctx->push_services;

	/* Iterate trough push service list and return JSON object  */
	while (psc != NULL) {
		jstring = json_object_new_string(psc->pushsrv_name); /* Create JSON-string */
		json_object_array_add(jarray,jstring); /* Add string to JSON array */
		psc = psc->next;
	}

	/* Add array to JSON-object */
	json_object_object_add(response, "japi_pushsrv_response", jarray);
}

/*
* Unsubscribe push service and close socket
*/
static void japi_pushsrv_remove_client(japi_pushsrv_context *psc, int socket)
{
	psc->subscribed_clients[0] = -1;
	close(socket);
}

/*
 * Send message to all subscribed clients of a push service
 */
int japi_pushsrv_sendmsg(japi_pushsrv_context *psc, json_object *jmsg)
{
	char *msg;
	int ret;
	int success; /* number of successfull send messages */
	int socket;

	/* Return -1 if there is no message to send */
	if (jmsg == NULL) {
		fprintf(stderr,"ERROR: Nothing to send.\n");
		return -1;
	}

	/* Return 0 if no client is subscribed */
	if ((socket = psc->subscribed_clients[0]) == -1) {
		return 0;
	}

	ret = 0;
	success = 0;
	msg = japi_get_jobj_as_ndstr(jmsg);
	ret = write_n(socket, msg, strlen(msg));
	free(msg);

	if (ret <= 0) {
		/* If write failed print error and unsubscribe client */
		fprintf(stderr, "ERROR: Failed to send push service message to client %i (write returned %i)\n", socket, ret);
		/* Unsubscribe push service and close socket */
		japi_pushsrv_remove_client(psc,socket);
		return ret;
	} else {
		success++;
	}

	return success;
}

/*
 * Wrapper function that is executed by pthread_create and starts the desired push service routine
 */
static void *generic_pushsrv_runner(void *arg)
{
	japi_pushsrv_context *psc;
	japi_pushsrv_routine routine;

	psc = (japi_pushsrv_context*)arg;

	assert(psc != NULL);

	/* Start user routine */
	routine = psc->routine;
	routine(psc);

	return NULL;
}

/*
 * Save thread infos and create thread
 */
int japi_pushsrv_start(japi_pushsrv_context *psc, japi_pushsrv_routine routine)
{
	if (psc == NULL) {
		fprintf(stderr,"ERROR: No push service context passed. Not starting thread.\n");
		return -1;
	}

	if (routine == NULL) {
		fprintf(stderr,"ERROR: No routine passed. Not starting thread.\n");
		return -2;
	}

	psc->enabled = true;
	psc->routine = routine;

	if (pthread_create(&(psc->thread_id),NULL,generic_pushsrv_runner,(void*)psc) != 0) {
		fprintf(stderr, "ERROR: Error creating push service thread.\n");
		psc->enabled = false;
		return -3;
	}

	return 0;
}

/*
 * Stop pushsrv routine
 */
int japi_pushsrv_stop(japi_pushsrv_context *psc)
{
	if (psc == NULL) {
		fprintf(stderr,"ERROR: No push service context passed. Can't stop thread.\n");
		return -1;
	}

	if (psc->enabled == false) {
		fprintf(stderr,"ERROR: Thread not running.\n");
		return -2;
	}

	/* Tell routine to end */
	psc->enabled = false;

	/* Wait for thread to end and close it */
	if (pthread_join(psc->thread_id,NULL) != 0) {
		fprintf(stderr, "ERROR: Error joining push service routine '%s'\n",psc->pushsrv_name);
		return -3;
	}

	return 0;
}
