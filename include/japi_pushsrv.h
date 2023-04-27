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

#ifndef __JAPI_PUSHSRV_H__
#define __JAPI_PUSHSRV_H__

#include <json-c/json.h>
#include <stdbool.h>
#include <pthread.h>

#include "japi.h"

#ifdef __cplusplus
extern "C" {
#endif

struct __japi_pushsrv_context; /* Tell routine there is a struct to be defined */

/*!
 * \brief JAPI push service routine.
 */
typedef void (*japi_pushsrv_routine)(struct __japi_pushsrv_context *psc);

/*!
 * \brief JAPI push service context
 *
 * A JAPI push service context stores the name of a push services
 * and subcribed clients.
 */
typedef struct __japi_pushsrv_context {
	char* pushsrv_name; /*!< Name of the push service */
	pthread_t thread_id; /*!< ID of the thread */
	japi_pushsrv_routine routine; /*!< Function to call */
	volatile bool enabled; /*!< Flag to end routine */
	pthread_mutex_t lock; /*!< Mutual access lock */
	struct __japi_client *clients; /*!< Pointer to the JAPI client context */
	struct __japi_pushsrv_context *next; /*!< Pointer to the next push service or NULL */
	void *userptr; /*!< Pointer to user data */
} japi_pushsrv_context;

/*!
 * \brief Register a JAPI push service
 *
 * Register a JAPI push service specified by pushsrv_name.
 *
 * \param ctx		JAPI context
 * \param pushsrv_name	Push-Service name
 *
 * \returns	On success, a pointer to the japi_push_service context is returned. On error, NULL is returned
 */
japi_pushsrv_context* japi_pushsrv_register(japi_context *ctx, const char *pushsrv_name);

/*!
 * \brief Unsubscribes and frees memory space for all push service clients
 *
 * Iterates through push service clients and unsubscribes and frees memory.
 *
 * \param psc	JAPI push service context
 *
 * \returns On success, 0 is returned. On error, -1 is returned.
 */
int japi_pushsrv_destroy(japi_pushsrv_context *psc);

/*!
 * \brief Send messages to all subscribed clients
 *
 * Send message to all subscribed clients of a push service.
 *
 * \param psc	JAPI push service context
 * \param jmsg	JSON push message
 *
 * \returns	On success, number of successfull send messages is returned. 0 is returned, if no client is subscribed. On error, -1 is returned.
 */
int japi_pushsrv_sendmsg(japi_pushsrv_context *psc, json_object *jmsg);

/*!
 * \brief Start push service routine
 *
 * Create thread and start push service routine.
 *
 * \param psc		JAPI push service context
 * \param routine	Thread Push service routine
 *
 * \returns	On success, 0 is returned. On error, -1 is returned.
 */
int japi_pushsrv_start(japi_pushsrv_context *psc, japi_pushsrv_routine routine);

/*!
 * \brief Stop push service routine
 *
 * Wait for thread to end and close it.
 *
 * \param psc	JAPI push service context
 *
 * \returns	On success, 0 is returned. On error, -1 is returned, -2 if thread is not running.
 */
int japi_pushsrv_stop(japi_pushsrv_context *psc);

#ifdef __cplusplus
}
#endif

#endif /* __JAPI_PUSHSRV_H__ */
