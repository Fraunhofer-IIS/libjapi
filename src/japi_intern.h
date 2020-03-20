/*!
 * \file
 * \author Deniz Armagan
 * \date 2019-05-13
 * \version 0.1
 *
 * \brief Internal functions of the JSON API library.
 *
 * \details
 * libjapi is a universal JSON API library.
 *
 * \copyright
 * Copyright (c) 2019 Fraunhofer IIS.
 * All rights reserved.
 */

#ifndef __JAPI_INTERN_H__
#define __JAPI_INTERN_H__

#include <json-c/json.h>

#include "japi_pushsrv.h"

/*!
 * \brief Process the JSON request
 *
 * Steps performed while processing a JSON request:
 * - Convert the received message into a JSON object
 * - Extract the request name
 * - Search a suitable request handler
 * - Call the request handler
 * - Prepare the JSON response
 * - Free memory
 *
 * \param ctx		Japi context
 * \param request	Request to process
 * \param response	From request build response
 * \param socket	Network socket
 *
 * \returns	On success, 0 returned. On error, -1 is returned.
 */
int japi_process_message(japi_context *ctx, const char *request, char **response, int socket);

/*!
 * \brief Remove client from push service
 *
 * Remove client socket from given push service.
 *
 * \param psc	JAPI push service context
 * \param socket	Socket to remove
 *
 * \returns	On success, 0 is returned. On error, -1 if client is not removed, is returned.
 */
int japi_pushsrv_remove_client(japi_pushsrv_context *psc, int socket);

/*!
 * \brief Remove client
 *
 * Remove client from JAPI context and pushs service context.
 *
 * \param ctx		JAPI context
 * \param socket	The socket to be removed
 *
 * \returns	On success, 0 is returned. On error, -1 if client is not removed, is returned.
 */
int japi_remove_client(japi_context *ctx, int socket);

/*!
 * \brief Remove all clients
 *
 * Remove all clients from JAPI context and pushs service context.
 *
 * \param ctx		JAPI context
 *
 * \returns	On success, 0 is returned. On error, -1 is returned if a client cannot be removed.
 */
int japi_remove_all_clients(japi_context *ctx);

/*!
 * \brief Add client
 *
 * Add client to JAPI context client list.
 *
 * \param ctx		JAPI context
 * \param socket	The socket to be added
 *
 * \returns	On success, 0 is returned. On error, -1 if memory allocation failed, is returned.
 */
int japi_add_client(japi_context *ctx, int socket);

/*!
* \brief Remove client from all push services
*
* Removes given client from all JAPI push service clients.
*
* \param ctx		JAPI context
* \param socket	The socket to be removed
*/
void japi_pushsrv_remove_client_from_all_pushsrv(japi_context *ctx, int socket);

#endif /* __JAPI_INTERN_H__ */
