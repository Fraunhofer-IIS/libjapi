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

#include <stdio.h> /* printf, fprintf */
#include <string.h> /* strcasecmp */
#include <json-c/json.h>

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

	response = malloc(strlen(tmp_str)+2);
	if (response == NULL) {
		perror("malloc");
		return NULL;
	}

	strcpy(response, tmp_str);
	response[tmp_str_len+0] = '\n';
	response[tmp_str_len+1] = '\0';

	return response;
}
