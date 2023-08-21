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

#ifndef __JAPI_PUSHSRV_INTERN_H__
#define __JAPI_PUSHSRV_INTERN_H__

#include <json-c/json.h>

/*!
 * \brief Subscribe a registered JAPI push service
 *
 * Subscribe a registered JAPI push service specified by pushsrv_name.
 *
 * \param ctx		JAPI context
 * \param jreq		Request JSON object
 * \param jresp		Response JSON object
 */
void japi_pushsrv_subscribe(japi_context *ctx, json_object *jreq, json_object *jresp);

/*!
 * \brief Unsubscribe a registered JAPI push service
 *
 * Unsubscribe a registered JAPI push service specified by pushsrv_name.
 *
 * \param ctx		JAPI context
 * \param jreq		Request JSON object
 * \param jresp		Response JSON object
 */
void japi_pushsrv_unsubscribe(japi_context *ctx, json_object *jreq, json_object *jresp);

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
