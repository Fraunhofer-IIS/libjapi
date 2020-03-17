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
#include <stdio.h>
#include <stdbool.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#include "japi_intern.h"
#include "japi_pushsrv_intern.h"
#include "japi_utils.h"
#include "prntdbg.h"

#include "rw_n.h"


/*!
* \brief Add client to push service
*
* Add client socket to given push service.
*
* \param socket	Socket to add
* \param pushsrv_name	The name of the push service
* \param psc	JAPI push service context
*
* \returns	On success, 0 is returned. On error, -1 if memory allocation failed.
*/
static int japi_pushsrv_add_client(japi_pushsrv_context *psc, int socket)
{
	japi_client *client;

	/* Error handling */
	assert(psc != NULL);
	assert(socket > 0);

	client = (japi_client*)malloc(sizeof(japi_client));
	if (client == NULL) {
		perror("ERROR: malloc() failed\n");
		return -1;
	}

	pthread_mutex_lock(&(psc->lock));
	client->socket = socket;
	client->next = psc->clients;
	psc->clients = client;
	pthread_mutex_unlock(&(psc->lock));

	return 0;
}

/*
 * Remove the client socket for the respective push service
 */
int japi_pushsrv_remove_client(japi_pushsrv_context *psc, int socket)
{
	japi_client *client, *prev;
	int ret = -1;

	/* Error handling */
	assert(psc != NULL);
	assert(socket >= 0);

	client = psc->clients;
	prev = NULL;

	/* Remove socket from list */
	while (client != NULL) {
		/* If first element */
		if ((client->socket == socket) && (prev == NULL)) {
			psc->clients = client->next;
			prntdbg("removing client %d from pushsrv %s\n",client->socket,psc->pushsrv_name);
			free(client);
			ret = 0;
			break;
		}
		/* If last element */
		if ((client->socket == socket) && (client->next == NULL)) {
			prev->next = NULL;
			prntdbg("removing client %d from pushsrv %s\n",client->socket,psc->pushsrv_name);
			free(client);
			ret = 0;
			break;
		}
		if (client->socket == socket) {
			prev->next = client->next;
			prntdbg("removing client %d from pushsrv %s\n",client->socket,psc->pushsrv_name);
			free(client);
			ret = 0;
			break;
		}

		prev = client;
		client = client->next;
	}

	return ret;
}

/*
 * Removes clients from all push services
 */
void japi_pushsrv_remove_client_from_all_pushsrv(japi_context *ctx, int socket)
{
	japi_pushsrv_context *psc;

	/* Error handling */
	assert(ctx != NULL);
	assert(socket >= 0);

	prntdbg("removing client %i from all pushsrv\n",socket);

	psc = ctx->push_services;
	while (psc != NULL) {
		pthread_mutex_lock(&(psc->lock));
		japi_pushsrv_remove_client(psc,socket);
		pthread_mutex_unlock(&(psc->lock));
		psc = psc->next;
	}
}

/*
 * Saves client socket, if passed push service is registered
 */
void japi_pushsrv_subscribe(japi_context *ctx, int socket, json_object *jreq, json_object *jresp)
{
	japi_pushsrv_context *psc;
	json_object *jval;
	const char* pushsrv_name;

	/* Error handling */
	assert(ctx != NULL);
	assert(socket != -1);
	assert(jreq != NULL);
	assert(jresp != NULL);

	psc = ctx->push_services;

	/* Get the push service name */
	if (!json_object_object_get_ex(jreq,"service",&jval) || jval == NULL) {
		json_object_object_add(jresp,"success",json_object_new_boolean(false));
		json_object_object_add(jresp,"message",json_object_new_string("Push service not found."));
		return;
	}
	pushsrv_name = json_object_get_string(jval);

	/* Search for push service in list and save socket, if found */
	while (psc != NULL) {
		if (strcasecmp(pushsrv_name,psc->pushsrv_name) == 0) {
			japi_pushsrv_add_client(psc,socket);
			break;
		}
		psc = psc->next;
	}

	json_object_object_add(jresp,"service",json_object_new_string(pushsrv_name));

	/* Create JSON response object */
	if (psc != NULL) {
		json_object_object_add(jresp,"success",json_object_new_boolean(true));
	} else {
		json_object_object_add(jresp,"success",json_object_new_boolean(false));
		json_object_object_add(jresp,"message",json_object_new_string("Push service not found."));
	}
}

/*
 * Removes client socket, if passed push service is registered
 */
void japi_pushsrv_unsubscribe(japi_context *ctx, int socket, json_object *jreq, json_object *jresp)
{
	japi_pushsrv_context *psc;
	json_object* jval;
	const char* pushsrv_name;

	/* Error handling */
	assert(ctx != NULL);
	assert(socket != -1);
	assert(jreq != NULL);
	assert(jresp != NULL);

	psc = ctx->push_services;
	bool registered = false; /* Service registered? */
	bool unsubscribed = false; /* Service unsubscribed? */

	/* Get the push service name */
	if (!json_object_object_get_ex(jreq,"service",&jval) || jval == NULL) {
		json_object_object_add(jresp,"success",json_object_new_boolean(false));
		json_object_object_add(jresp,"message",json_object_new_string("Push service not found."));
		return;
	}
	pushsrv_name = json_object_get_string(jval);

	/* Search for push service in list and remove socket, if found & socket is registered */
	while (psc != NULL) {
		if (strcasecmp(pushsrv_name,psc->pushsrv_name) == 0) {
			registered = true;
			if (japi_pushsrv_remove_client(psc,socket) >= 0) {
				unsubscribed = true;
				break;
			}
		}
		psc = psc->next;
	}

	json_object_object_add(jresp,"service",json_object_new_string(pushsrv_name));

	/* Create JSON response object */
	if (registered && unsubscribed) { /* Subscribed */
		json_object_object_add(jresp,"success",json_object_new_boolean(true));
	} else if (registered && !unsubscribed) { /* Registered, but not subscribed */
		json_object_object_add(jresp,"success",json_object_new_boolean(false));
		json_object_object_add(jresp,"message",json_object_new_string("Can't unsubscribe a service that wasn't subscribed before."));
	} else { /* Not registered */
		json_object_object_add(jresp,"success",json_object_new_boolean(false));
		json_object_object_add(jresp,"message",json_object_new_string("Push service not found."));
	}
}

/* Check if there is a duplicate name for a request */
static bool pushsrv_isredundant(japi_context *ctx, const char *pushsrv_name)
{
	japi_pushsrv_context *psc;
	bool duplicate;

	duplicate = false;
	psc = ctx->push_services;

	while (psc != NULL) {
		if (strcmp(psc->pushsrv_name, pushsrv_name) == 0) {
			duplicate = true;
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
	psc->thread_id = 0;
	psc->routine = NULL;
	psc->clients = NULL;
	psc->enabled = false;
	psc->userptr = ctx->userptr;

	if (pthread_mutex_init(&(psc->lock),NULL) != 0) {
		fprintf(stderr,"ERROR: mutex initialization has failed\n");
		return NULL;
	}

	/* Point to last struct */
	psc->next = ctx->push_services;
	ctx->push_services = psc;

	return psc;
}

/*
 * Iterate through push service and unsubscribe & free memory for all clients
 */
int japi_pushsrv_destroy(japi_pushsrv_context *psc)
{
	japi_client *client, *client_next;

	if (psc == NULL) {
		fprintf(stderr,"ERROR: push service context is NULL");
		return -1;
	}

	client = psc->clients;

	/* Iterates through push service client list and frees memory for every element and for the push service themself */
	pthread_mutex_lock(&(psc->lock));
	while (client != NULL) {
		client_next = client->next;
		japi_pushsrv_remove_client(psc,client->socket);
		client = client_next;
	}
	pthread_mutex_unlock(&(psc->lock));

	japi_pushsrv_stop(psc);

	pthread_mutex_destroy(&(psc->lock));
	free_pushsrv(psc);

	return 0;
}

/*
 * Provide the names of all registered push-services as a JAPI response.
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

	/* Iterate through push service list and return JSON object  */
	while (psc != NULL) {
		jstring = json_object_new_string(psc->pushsrv_name); /* Create JSON-string */
		json_object_array_add(jarray,jstring); /* Add string to JSON array */
		psc = psc->next;
	}

	/* Add array to JSON-object */
	json_object_object_add(response, "services", jarray);
}

/*
 * Send message to all subscribed clients of a push service
 */
int japi_pushsrv_sendmsg(japi_pushsrv_context *psc, json_object *jmsg_data)
{
	char *msg;
	int ret;
	int success; /* number of successfull send messages */
	japi_client *client, *following_client;
	json_object *jmsg;
	json_object *jdata;

	/* Return -1 if there is no message to send */
	if (jmsg_data == NULL) {
		fprintf(stderr,"ERROR: Nothing to send.\n");
		return -1;
	}

	/* Return 0 if no client is subscribed */
	if (psc->clients == NULL) {
		return 0;
	}

	ret = 0;
	success = 0;
	jmsg = json_object_new_object();
	jdata = NULL;

	json_object_object_add(jmsg,"japi_pushsrv",json_object_new_string(psc->pushsrv_name));
	jdata = json_object_get(jmsg_data); // increment refcount before calling json_object_object_add as jmesg_data may still be in use by the caller
	json_object_object_add(jmsg,"data",jdata);

	msg = japi_get_jobj_as_ndstr(jmsg);
	json_object_put(jmsg);

	pthread_mutex_lock(&(psc->lock));
	client = psc->clients;

	while (client != NULL) {
		prntdbg("pushsrv '%s': Sending message to client %d\n. Message: '%s'",psc->pushsrv_name,client->socket,msg);
		following_client = client->next; // Save pointer to next element

		ret = write_n(client->socket, msg, strlen(msg));

		if (ret <= 0) {
			/* If write failed print error and unsubscribe client */
			fprintf(stderr, "ERROR: Failed to send push service message to client %i (write returned %i)\n", client->socket, ret);
			/* Remove client from respective push service and free */
			japi_pushsrv_remove_client(psc,client->socket);
		} else {
			success++;
		}
		client = following_client;
	}
	pthread_mutex_unlock(&(psc->lock));

	free(msg);

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
