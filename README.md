# libjapi

libjapi is a universal JSON to C API library. It receives newline-delimited
JSON (NDJSON) messages via TCP and calls registered C functions. A JSON
response is returned for each request. Furthermore, it is also possible to
create push services, which asynchronously push JSON messages to the clients
subscribed to them.

## Documentation
The documentation can be found [here](http://ks-ip-lib.git01.iis.fhg.de/software/libjapi/doc/html/index.html).

## Packages
Prebuild packages can be downloaded [here](http://ks-ip-lib.git01.iis.fhg.de/software/libjapi/repo/index.html).

## Features
* Synchronous communication (request, response)
* Asynchronous communication (register, push)
* Multi-client support

## Getting started

### Prerequisites
* [json-c](https://github.com/json-c/json-c)
* [cmake version 3.6](https://cmake.org/)

### Installation
Create a build directory and call *cmake* in that directory.

    $ mkdir build
    $ cd build/
    $ cmake ../

A Makefile is generated. Run 'make' to build the libjapi libraries.

    $ make

A shared and a static library is built.

## Demo
You can clone the [demo project](https://git01.iis.fhg.de/ks-ip-lib/software/libjapi-demo), with examples for all features from the repository listed below:

    $ git clone --recurse-submodules git@git01.iis.fhg.de:ks-ip-lib/software/libjapi-demo.git

## Usage & Examples
* Create a JAPI context
* Write application specific functions
* Register these application specific functions to libjapi
* Start a JAPI server
* Enjoy the flexibility

### Server example

    #include <assert.h>
    #include <stdio.h>
    #include <string.h>
    #include <unistd.h> /* sleep */

    #include <japi.h>
    #include <japi_pushsrv.h>
    #include <japi_utils.h>

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
            japi_pushsrv_context *psc_counter;

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

            /* Register JSON API request */
            japi_register_request(ctx, "get_temperature", &get_temperature);

            /* Register push service */
            psc_counter = japi_pushsrv_register(ctx, "push_counter");

            /* Start push thread */
            japi_pushsrv_start(psc_counter,&push_counter);

            /* Provide JSON API interface via TCP */
            ret = japi_start_server(ctx, argv[1]);

            /* Wait for the thread to finish */
            japi_pushsrv_stop(psc_counter);

            /* Destroy JAPI context */
            japi_destroy(ctx);

            return ret;
    }

### Client JSON request examples

    {
      "japi_request": "get_temperature",
      "args": {
      "unit": "kelvin"
      }
    }

    {
      "japi_request": "japi_pushsrv_list",
    }

    {
      "japi_request": "japi_pushsrv_subscribe",
      "args": {
      "service": "push_counter"
      }
    }

    {
      "japi_request": "japi_pushsrv_unsubscribe",
      "args": {
      "service": "push_counter"
      }
    }

