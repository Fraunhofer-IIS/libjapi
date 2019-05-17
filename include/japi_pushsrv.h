/*!
 * \file
 * \author Deniz Armagan
 * \date 2019-03-25
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

#ifndef __JAPI_PUSHSRV_H__
#define __JAPI_PUSHSRV_H__

#include <json-c/json.h>
#include <stdbool.h>

#include "japi.h"

struct __japi_pushsrv_context; /* Tell routine there is a struct to be defined */

/*!
 * \brief JAPI push service routine.
 */
typedef int (*japi_pushsrv_routine)(struct __japi_pushsrv_context *psc);

/*!
 * \brief JAPI push service context
 *
 * A JAPI push service context stores the name of a push services
 * and subcribed clients.
 */
typedef struct __japi_pushsrv_context {
	char* pushsrv_name; /*!< Name of the push service */
	int subscribed_clients[10]; /*!< List of the subscribed clients */
	pthread_t thread_id; /*!< ID of the thread */
	japi_pushsrv_routine routine; /*!< Function to call */
	volatile bool enabled; /*!< Flag to end routine */
	struct __japi_pushsrv_context *next; /*!< Pointer to the next push service or NULL */
} japi_pushsrv_context;

/*!
 * \brief Register a JAPI push service
 *
 * Register a JAPI push service specified by pushsrv_name.
 *
 * \param ctx                 JAPI context
 * \param pushsrv_name        Push-Service name
 *
 * \returns On success, a pointer to the japi_push_service context is returned. On error, NULL is returned.
 */
japi_pushsrv_context* japi_pushsrv_register(japi_context *ctx, const char *pushsrv_name);

/*!
 * \brief Subscribe a registered JAPI push service
 *
 * Subscribe a registered JAPI push service specified by pushsrv_name.
 *
 * \param ctx     JAPI context
 * \param socket  Client socket
 * \param pushsrv_name Push-Service name
 * \param jresp   Response JSON object
 */
void japi_pushsrv_subscribe(japi_context *ctx, int socket, const char* pushsrv_name, json_object *jresp);

/*!
 * \brief Unsubscribe a registered JAPI push service
 *
 * Unsubscribe a registered JAPI push service specified by pushsrv_name.
 *
 * \param ctx           JAPI context
 * \param socket        Client socket
 * \param pushsrv_name  Push-Service name
 * \param jresp         Response JSON object
 */
void japi_pushsrv_unsubscribe(japi_context *ctx, int socket, const char* pushsrv_name, json_object *jresp);

/*!
 * \brief List registered JAPI push services as JAPI response
 *
 * Provides the names of all registered push-services as a JAPI response.
 *
 * \param ctx      JAPI context
 * \param request  Pointer to JAPI JSON request
 * \param response Pointer to JAPI JSON response
 */
void japi_pushsrv_list(japi_context *ctx, json_object *request, json_object *response);

/*!
 * \brief Frees memory space for allocated push service elements
 *
 * Iterates trough push services and frees memory for push service elements.
 *
 * \param ctx       JAPI context
 */
void japi_pushsrv_shutdown_all(japi_context *ctx);

/*!
 * \brief Send messages to all subscribed clients
 *
 * Send message to all subscribed clients of a push service.
 *
 * \param psc           JAPI push service context
 * \param jmsg          JSON push message
 *
 * \returns On success, number of successful send messages is returned. 0 is returned, if no client is subscribed. On error, -1 is returned.
 */
int japi_pushsrv_sendmsg(japi_pushsrv_context *psc, json_object *jmsg);

 /*!
  * \brief Start push service routine
  *
  * Create thread and start push service routine.
  *
  * \param psc            JAPI push service context
  * \param routine Thread Push service routine
  *
  * \returns On success, 0 is returned. On error, -1 is returned.
  */
int japi_pushsrv_start(japi_pushsrv_context *psc, japi_pushsrv_routine routine);

 /*!
  * \brief Stop push service routine
  *
  * Wait for thread to end and close it.
  *
  * \param psc	JAPI push service context
  *
  * \returns On success, 0 is returned. On error, -1 is returned, -2 if thread is not running.
  */
int japi_pushsrv_stop(japi_pushsrv_context *psc);


#endif /* __JAPI_PUSHSRV_H__ */
