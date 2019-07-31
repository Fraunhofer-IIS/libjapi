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

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Get string value from json key
 *
 * Return string for the given json key value.
 *
 * \param jobj	The json-object to parse
 * \param key	The corresponding key value
 *
 * \returns	On success, string is returned. On error, NULL is returned.
 */
const char* japi_get_value_as_str(json_object *jobj, const char *key);

/*!
 * \brief Get boolean value from json key
 *
 * Return boolean for the given json key value.
 *
 * \param jobj	The json-object to parse
 * \param key	The corresponding key value
 *
 * \returns	On success, boolean is returned. On error, NULL is returned.
 */
bool japi_get_value_as_bool(json_object *jobj, const char *key);

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

#ifdef __cplusplus
}
#endif

#endif /* __JAPI_UTILS_H__ */
