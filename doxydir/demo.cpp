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
 * \copyright
 * Copyright (c) 2018 Fraunhofer IIS.
 * All rights reserved.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h> /* sleep */

#include <japi.h>
#include <japi_pushsrv.h>
#include <japi_utils.h>

/*
 * User defined push temperature service routine.
 * Simulates a circular sinus push value.
 */
int push_temperature(japi_pushsrv_context *psc)
{
	json_object *jmsg;
	double i;

	assert(psc != NULL);

	jmsg = json_object_new_object();
	while (psc->enabled) {

		for (i = 0.0; i <= 3.14; i += 0.1) {
			/* Create JSON response string */
			json_object_object_add(jmsg,"temperature",json_object_new_double(30+10*sin(i)));

			/* Push message */
			japi_pushsrv_sendmsg(psc,jmsg);
			sleep(1);
		}

	}
	json_object_put(jmsg);

	return 0;
}

/* User defined push service routine */
int push_counter(japi_pushsrv_context *psc)
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

	return 0;
}

static void rnf_handler(japi_context *ctx, json_object *request, json_object *response)
{
	json_object_object_add(response, "japi_response_msg", json_object_new_string("ERROR: No request handler found!"));
}

static void get_temperature(japi_context *ctx, json_object *request, json_object *response)
{
	double temperature;
	const char *unit;

	/*
	 * TODO: Read the temperature from a sensor...
	 */
	temperature = 27.0;

	/* Provide the temperature in KELVIN (if requested)
	 * or CELSIUS (default) */
	unit = japi_get_value_as_str(request, "unit");
	if (unit != NULL && strcmp(unit, "kelvin") == 0) {
		temperature += 273;
	} else {
		unit = "celsius";
	}

	/* Prepare and provide response */
	json_object_object_add(response, "temperature", json_object_new_double(temperature));
	json_object_object_add(response, "unit", json_object_new_string(unit));
}

int main(int argc, char *argv[])
{
	int ret;
	japi_context *ctx;
	japi_pushsrv_context *psc_counter, *psc_temperature;

	/* Read port */
	if (argc != 2) {
		fprintf(stderr, "ERROR: Missing argument or wrong amount of arguments.\n" \
				"Usage:\n\t%s <port>\n", argv[0]);
		return -1;
	}

	/* Create JSON API context */
	ctx = japi_init(NULL);
	if (ctx == NULL) {
		fprintf(stderr, "ERROR: Failed to create japi context\n");
		return -1;
	}

	/* Register JSON API requests */
	japi_register_request(ctx, "request_not_found_handler", &rnf_handler);
	japi_register_request(ctx, "get_temperature", &get_temperature);

	/* Register push services */
	psc_counter = japi_pushsrv_register(ctx, "push_counter");
	psc_temperature = japi_pushsrv_register(ctx, "push_temperature");

	/* Start push threads */
	japi_pushsrv_start(psc_counter,&push_counter);
	japi_pushsrv_start(psc_temperature,&push_temperature);

	/* Provide JSON API interface via TCP */
	ret = japi_start_server(ctx, argv[1]);

	/* Wait for the threads to finish */
	japi_pushsrv_stop(psc_counter);
	japi_pushsrv_stop(psc_temperature);

	/* Destroy JAPI context */
	japi_destroy(ctx);

	return ret;
}
