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
#include <stdio.h> /* printf, fprintf */
#include <string.h> /* strcasecmp */
#include <json-c/json.h>

#include "japi_utils.h"


/* Look for a JSON object with the key 'key' and return its value as a string.
 *
 * returns
 *  0 if legal string value could be fetched
 *  -1 if given JSON object is NULL
 *  -2 if given key is NULL
 *  -3 if given key doesn't exist
 *  -4 if the requested value is not a string type
 */
int japi_get_value_as_str(json_object *jobj, const char *key, const char** val)
{
	json_object *jval;

	/* Error Handling */
	if (jobj == NULL) {
		return -1;
	}
	if (key == NULL) {
		return -2;
	}

	/* Key doesn't exist */
	if (!json_object_object_get_ex(jobj, key, &jval)) {
		return -3;
	}

	/* The requested value is not a string type */
	if (!json_object_is_type(jval, json_type_string)) {
		return -4;
	}

	*val = json_object_get_string(jval);

	return 0;
}

/* Look for a JSON object with the key 'key' and return its value as a bool.
 *
 * returns
 *  0 if legal boolean value could be fetched
 *  -1 if given JSON object is NULL
 *  -2 if given key is NULL
 *  -3 if given key doesn't exist
 *  -4 if the requested value is not a boolean type
 */
int japi_get_value_as_bool(json_object *jobj, const char *key, bool *val)
{
	json_object *jval;

	/* Error Handling */
	if (jobj == NULL) {
		return -1;
	}
	if (key == NULL) {
		return -2;
	}

	/* Key doesn't exist */
	if (!json_object_object_get_ex(jobj, key, &jval)) {
		return -3;
	}

	/* The requested value is not a boolean type */
	if (!json_object_is_type(jval, json_type_boolean)) {
		return -4;
	}

	*val = json_object_get_boolean(jval);

	return 0;
}

/* Look for a JSON object with the key 'key' and return its value as a int.
 *
 * returns
 *  0 if legal int value could be fetched
 *  -1 if given JSON object is NULL
 *  -2 if given key is NULL
 *  -3 if given key doesn't exist
 *  -4 if the requested value is not a int type
 */
int japi_get_value_as_int(json_object *jobj, const char *key, int *val)
{
	json_object *jval;

	/* Error Handling */
	if (jobj == NULL) {
		return -1;
	}
	if (key == NULL) {
		return -2;
	}

	/* Key doesn't exist */
	if (!json_object_object_get_ex(jobj, key, &jval)) {
		return -3;
	}

	/* The requested value is not a int type */
	if (!json_object_is_type(jval, json_type_int)) {
		return -4;
	}

	*val = json_object_get_int(jval);

	return 0;
}
/* Look for a JSON object with the key 'key' and return its value as a int64.
 *
 * returns
 *  0 if legal int64 value could be fetched
 *  -1 if given JSON object is NULL
 *  -2 if given key is NULL
 *  -3 if given key doesn't exist
 *  -4 if the requested value is not a int64 type
 */
int japi_get_value_as_int64(json_object *jobj, const char *key, long long int *val)
{
	json_object *jval;

	/* Error Handling */
	if (jobj == NULL) {
		return -1;
	}
	if (key == NULL) {
		return -2;
	}

	/* Key doesn't exist */
	if (!json_object_object_get_ex(jobj, key, &jval)) {
		return -3;
	}

	/* The requested value is not a int64 type */
	if (!json_object_is_type(jval, json_type_int)) {
		return -4;
	}

	*val = json_object_get_int64(jval);

	return 0;
}

/* Look for a JSON object with the key 'key' and return its value as a double.
 *
 * returns
 *  0 if legal double value could be fetched
 *  -1 if given JSON object is NULL
 *  -2 if given key is NULL
 *  -3 if given key doesn't exist
 *  -4 if the requested value is not a double type
 */
int japi_get_value_as_double(json_object *jobj, const char *key, double *val)
{
	json_object *jval;

	/* Error Handling */
	if (jobj == NULL) {
		return -1;
	}
	if (key == NULL) {
		return -2;
	}

	/* Key doesn't exist */
	if (!json_object_object_get_ex(jobj, key, &jval)) {
		return -3;
	}

	/* The requested value is not a double type */
	if (!json_object_is_type(jval, json_type_double)) {
		return -4;
	}

	*val = json_object_get_double(jval);

	return 0;
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
