/*!
 * \file
 * \author Deniz Armagan
 * \date 2019-06-12
 * \version 0.1
 *
 * \brief Internal functions of the JSON API push servicelibrary.
 *
 * \details
 * libjapi is a universal JSON API library.
 *
 * \copyright
 * Copyright (c) 2019 Fraunhofer IIS.
 * All rights reserved.
 */

#ifndef __JAPI_PUSHSRV_INTERN_H__
#define __JAPI_PUSHSRV_INTERN_H__

#include <json-c/json.h>

/*!
 * \brief Subscribe a registered JAPI push service
 *
 * Subscribe a registered JAPI push service specified by pushsrv_name.
 *
 * \param ctx		JAPI context
 * \param socket	Client socket
 * \param jreq		Request JSON object
 * \param jresp		Response JSON object
 */
void japi_pushsrv_subscribe(japi_context *ctx, int socket, json_object *jreq, json_object *jresp);

/*!
 * \brief Unsubscribe a registered JAPI push service
 *
 * Unsubscribe a registered JAPI push service specified by pushsrv_name.
 *
 * \param ctx		JAPI context
 * \param socket	Client socket
 * \param jreq		Request JSON object
 * \param jresp		Response JSON object
 */
void japi_pushsrv_unsubscribe(japi_context *ctx, int socket,  json_object *jreq, json_object *jresp);

/*!
 * \brief List registered JAPI push services as JAPI response
 *
 * Provides the names of all registered push-services as a JAPI response.
 *
 * \param ctx		JAPI context
 * \param request 	Pointer to JAPI JSON request
 * \param response	Pointer to JAPI JSON response
 * \note Parameter 'request' declared, although not used in function.
 * Function declaration needs to be identical to respective handler.
 */
void japi_pushsrv_list(japi_context *ctx, json_object *request, json_object *response);

#endif /* __JAPI_PUSHSRV_INTERN_H__ */
