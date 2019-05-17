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

#endif /* __JAPI_INTERN_H__ */
