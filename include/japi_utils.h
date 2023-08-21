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
 *\copyright
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

#ifndef __JAPI_UTILS_H__
#define __JAPI_UTILS_H__

#include <json-c/json.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Get string value from json key
 *
 * Return string for the given json key.
 *
 * \param jobj	The json-object to parse
 * \param key	The corresponding key to requested value
 * \param val	The requested value
 *
 * \returns	On success, 0 is returned. On error,
 *  -1 if given JSON object is NULL,
 *  -2 if given key is NULL,
 *  -3 if given key doesn't exist,
 *  -4 if the key value is not a string type, is returned.
 */
int japi_get_value_as_str(json_object *jobj, const char *key, const char **val);

/*!
 * \brief Get boolean value from json key
 *
 * Return boolean for the given json key.
 *
 * \param jobj	The json-object to parse
 * \param key	The corresponding key to requested value
 * \param val	The requested value
 *
 * \returns	On success, 0 is returned. On error,
 *  -1 if given JSON object is NULL,
 *  -2 if given key is NULL,
 *  -3 if given key doesn't exist,
 *  -4 if the key value is not a boolean type, is returned.
 */
int japi_get_value_as_bool(json_object *jobj, const char *key, bool *val);

/*!
 * \brief Get int value from json key
 *
 * Return int for the given json key.
 *
 * \param jobj	The json-object to parse
 * \param key	The corresponding key to requested value
 * \param val	The requested value
 *
 * \returns	On success, 0 is returned. On error,
 *  -1 if given JSON object is NULL,
 *  -2 if given key is NULL,
 *  -3 if given key doesn't exist,
 *  -4 if the key value is not a int type, is returned.
 */
int japi_get_value_as_int(json_object *jobj, const char *key, int *val);

/*!
 * \brief Get int64 value from json key
 *
 * Return int64 for the given json key.
 *
 * \param jobj	The json-object to parse
 * \param key	The corresponding key to requested value
 * \param val	The requested value
 *
 * \returns	On success, 0 is returned. On error,
 *  -1 if given JSON object is NULL,
 *  -2 if given key is NULL,
 *  -3 if given key doesn't exist,
 *  -4 if the key value is not a int64 type, is returned.
 */
int japi_get_value_as_int64(json_object *jobj, const char *key, long long int *val);

/*!
 * \brief Get double value from json key
 *
 * Return double for the given json key.
 *
 * \param jobj	The json-object to parse
 * \param key	The corresponding key to requested value
 * \param val	The requested value
 *
 * \returns	On success, 0 is returned. On error,
 *  -1 if given JSON object is NULL,
 *  -2 if given key is NULL,
 *  -3 if given key doesn't exist,
 *  -4 if the key value is not a double type, is returned.
 */
int japi_get_value_as_double(json_object *jobj, const char *key, double *val);

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
