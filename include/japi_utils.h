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

#ifndef __JAPI_UTILS_H__
#define __JAPI_UTILS_H__

#include <json-c/json.h>

/*!
 * \brief Returns a string with added newline
 *
 * Stringify the JSON object, copy it to a new allocated buffer and append a
 * newline. This is necessary to ensure that a call to write() will send the
 * message at once, meaning the message and the newline are not separated in
 * two strings.
 * The caller is responsible for freeing the returned string.
 *
 * \param jobj   The JSON object
 *
 * \returns On success, string is returned. On error, NULL is returned.
 */
char* japi_get_jobj_as_ndstr(json_object *jobj);

#endif /* __JAPI_UTILS_H__ */
