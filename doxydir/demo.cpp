/*!
 * \file
 * \author Christopher Stender
 * \date 2018-02-26
 * \version 0.1
 *
 * \brief libjapi demo application
 *
 * \details
 * This application demonstrates the usage of the JSON API library (libjapi).
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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h> /* sleep */

#include <japi.h>
#include <japi_pushsrv.h>
#include <japi_utils.h>

/* Create a resource structure to pass to libjapi*/
typedef struct resources
{
	double temperature;
}resources;

/*
 * User defined push temperature service routine.
 * Simulates a circular sinus push value.
 */
void push_temperature(japi_pushsrv_context *psc)
{
	json_object *jmsg;
	double i;

	assert(psc != NULL);

	/* Get back pointer on resources (i.e for example sensors values) */
	resources* sensor_values = (resources*) psc->userptr;

	jmsg = json_object_new_object();
	while (psc->enabled) {

		for (i = 0.0; i <= 3.14; i += 0.1) {
			/* Create JSON response string */
			json_object_object_add(jmsg,"temperature",json_object_new_double(sensor_values->temperature+10*sin(i)));

			/* Push message */
			japi_pushsrv_sendmsg(psc,jmsg);
			sleep(1);
		}

	}
	json_object_put(jmsg);
}

/* User defined push service routine */
void push_counter(japi_pushsrv_context *psc)
{
	json_object *jmsg;
	int i;

	assert(psc != NULL);

	i = 0;
	jmsg = json_object_new_object();

	while (psc->enabled) {
		/* Create JSON response string */
		json_object_object_add(jmsg,"counter",json_object_new_int(i));

		/* Push message */
		japi_pushsrv_sendmsg(psc,jmsg);

		i++;
		sleep(1);
	}
	json_object_put(jmsg);
}

static void rnf_handler(japi_context *ctx, json_object *request, json_object *response)
{
	json_object_object_add(response, "japi_response_msg", json_object_new_string("ERROR: No request handler found!"));
}

static void get_temperature(japi_context *ctx, json_object *request, json_object *response)
{
	double temperature;
	const char *unit;

	/* Get back pointer on resources (i.e for example sensors values) */
	resources* sensor_values = (resources*) ctx->userptr;
	temperature = sensor_values->temperature;

	/* Provide the temperature in KELVIN (if requested)
	 * or CELSIUS (default) */
	if (japi_get_value_as_str(request, "unit", &unit) != 0 ) {
		fprintf(stderr, "Failed to get string value from key 'unit'\n");
	}
	if (unit != NULL && strcmp(unit, "kelvin") == 0) {
		temperature += 273;
	} else {
		unit = "celsius";
	}

	/* Prepare and provide response */
	json_object_object_add(response, "temperature", json_object_new_double(temperature));
}

int main(int argc, char *argv[])
{
	int ret;
	japi_context *ctx;
	japi_pushsrv_context *psc_counter, *psc_temperature;

	/* Declare & initialise resources*/
	resources temperature_sensor = {17.0};

	/* Read port */
	if (argc != 2) {
		fprintf(stderr, "ERROR: Missing argument or wrong amount of arguments.\n" \
				"Usage:\n\t%s <port>\n", argv[0]);
		return -1;
	}

	/* Create JSON API context */
	ctx = japi_init(&temperature_sensor);
	if (ctx == NULL) {
		fprintf(stderr, "ERROR: Failed to create japi context\n");
		return -1;
	}

	/* Include request args in response */
	japi_include_args_in_response(ctx, true);

	/* Register JSON API requests */
	japi_register_request(ctx, "request_not_found_handler", &rnf_handler);
	japi_register_request(ctx, "get_temperature", &get_temperature);

	/* Register push services */
	psc_counter = japi_pushsrv_register(ctx, "push_counter");
	psc_temperature = japi_pushsrv_register(ctx, "push_temperature");

	/* Start push threads */
	japi_pushsrv_start(psc_counter, &push_counter);
	japi_pushsrv_start(psc_temperature, &push_temperature);

	/* Set maximal number of allowed clients. 0 for unlimited */
	japi_set_max_allowed_clients(ctx, 3);

	/* Provide JSON API interface via TCP */
	ret = japi_start_server(ctx, argv[1]);

	/* Destroy JAPI context */
	japi_destroy(ctx);

	return ret;
}
