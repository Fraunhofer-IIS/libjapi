/*!
 * \file
 * \author Deniz Armagan
 * \date 2019-04-10
 * \version 0.1
 *
 * \brief Universal JAPI helper functions library.
 *
 * \details
 * japi_utils is a universal JSON API helper library.
 *
 * \copyright
 * Copyright (c) 2019 Fraunhofer IIS.
 * All rights reserved.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h> /* printf, fprintf */
#include <string.h> /* strcasecmp */
#include <json-c/json.h>


/* Look for a JSON object with the key 'key' and return its value as a string.
 *
 * NULL is returned if there is no object with a key 'key' or if the value is
 * not a string type.
 */
const char* japi_get_value_as_str(json_object *jobj, const char *key)
{
	json_object *jval;

	assert(jobj != NULL);
	assert(key != NULL);

	if (json_object_object_get_ex(jobj, key, &jval)) {
		if (json_object_is_type(jval, json_type_string)) {
			return json_object_get_string(jval);
		}
	}

	return NULL;
}

/* Look for a JSON object with the key 'key' and return its value as a bool.
 *
 * NULL is returned if there is no object with a key 'key' or if the value is
 * not a boolean type.
 */
bool japi_get_value_as_bool(json_object *jobj, const char *key)
{
	json_object *jval;

	assert(jobj != NULL);
	assert(key != NULL);

	if (json_object_object_get_ex(jobj, key, &jval)) {
		if (json_object_is_type(jval, json_type_boolean)) {
			return json_object_get_boolean(jval);
		}
	}

	return NULL;
}


/* Stringify the JSON object, copy it to a new allocated buffer and append a
 * newline. This is necessary to ensure that a call to write() will send the
 * message at once, meaning the message and the newline are not separated in
 * two strings.
 *
 * The caller is responsible for freeing the returned string.
 */
char* japi_get_jobj_as_ndstr(json_object *jobj)
{
	const char *tmp_str;
	char *response;
	size_t tmp_str_len;

	tmp_str = json_object_to_json_string(jobj);
	tmp_str_len = strlen(tmp_str);

	response = (char *)malloc(strlen(tmp_str)+2);
	if (response == NULL) {
		perror("malloc");
		return NULL;
	}

	strcpy(response, tmp_str);
	response[tmp_str_len+0] = '\n';
	response[tmp_str_len+1] = '\0';

	return response;
}
