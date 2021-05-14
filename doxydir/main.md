
# libjapi

libjapi is a universal JSON API library. It receives newline-delimited JSON
(NDJSON) messages via TCP and calls registered C functions. A JSON response is
returned for each request. Furthermore, it is also possible to create push services,
which push custom JSON messages to the clients subscribed to them.

## Features
* Push Services
* Multi-Client support

## Getting started

### Prerequisites
- [json-c](https://github.com/json-c/json-c)
- [cmake version 3.5](https://cmake.org/)

### Installation

\anchor build
#### Build
Clone the git repository and update the submodule:
\code
$ git clone git@git01.iis.fhg.de:ks-ip-lib/software/libjapi.git
$ git submodule init
$ git submodule update
\endcode

Run \a cmake in the libjapi repository.
\code
$ mkdir build/
$ cd build/
$ cmake ../
\endcode

A Makefile is generated. Run \a make to build the libjapi libraries.
\code
$ make
\endcode
The shared and static libraries are to be found in the directory \a make was run.

#### Packages
The packages can be found <a href="../../repo/index.html">here</a>.

\anchor demo
## Demo
You can clone the demo project, with examples for all features from the repository listed below:

\code
$ git clone --recurse-submodules git@git01.iis.fhg.de:ks-ip-lib/software/libjapi-demo.git
\endcode

First build the libjapi, the demo uses, in the \a libjapi directory with:
\code
$ make
\endcode

Back in the \a libjapi-demo directory run
\code
$ make run-static
\endcode
to start the server.

\note for the next step python3 needs to be installed.

Start the python client script in a separate tab with
\code
$ ./test.py
\endcode

The server can manage multiple clients. Try and run multiple clients simultaneously.

To end the client(s) send a CTRL-C. Same for the server.

## Usage
* Create a JAPI context
* Write application specific functions
* Register these application specific functions to libjapi
* Start a JAPI server
* Enjoy the flexibility

The JAPI context saves information for communication, like sockets and registered services. JAPI context is created with:
\code
japi_context *ctx;

ctx = japi_init(NULL);
\endcode

\a japi_init() creates a new JAPI context and returns it. After that application specific functions can be written. They can be registered with \a japi_register_request():
\code
japi_register_request(ctx,"function_name",&function_handler);
\endcode

The function will be saved in the passed JAPI context. The server which manages the requests, is started with:
\code
japi_start_server(ctx,8080);
\endcode

It will need the JAPI context and the port. If the server started, requests can be send in JSON format. For details see the next \ref formats "topic".

## Pass libjapi extern arguments
If there are arguments like e.g C-Objects or C-Structs, they can be passed with \a japi_init(). The pointer to the struct or object passed to \a japi_init() as argument will be saved in the \a japi_context struct and can be accessed trough the \a userptr.

Example:
\code
ctx = japi_init(object_pointer);
ctx->userptr ... #access to passed argument
\endcode

\anchor formats
## Communication concepts

There are two types of communication - requests and push-services.

### JSON requests
JSON requests are synchronous requests. A valid request results in an response.

### Push-services
Push services are asynchronous responses from the server. They can be subscribed and unsubscribed through synchron request. The push service will send JSON responses until it's unsubscribed by a JSON request to the server, by the respective client.

If subscribed trough an synchronous JSON requests, push services are pushed dependent on there configured interval, to the subscribed clients. Internally they are run in an own thread. That means, the service messages are pushed independently from the server. One push service (thread) can manage multiple clients, that are subscribed to him.

\note \a japi_pushsrv_list is registered by default.

\anchor commSpecs
## JSON specification for communication types

### Request format
JSON requests to the libjapi always start with the JSON key \a japi_request followed by the japi request name:
\code
{
  "japi_request": "<request_name>",
  "args": {
  	"<request-specific-key-1>": "<request-specific-value-1>",
	"<request-specific-key-n>": "<request-specific-value-n>"
  }
}
\endcode

A missing \a japi_request key in a JSON request will result in errors. The JSON keys in \a args are additional values the request needs for processing. It's the same for self-defined request-functions; the needed keys are defined by the functions. See \ref clientExample "here" for detailed examples.

#### Providing request number
An optional key `japi_request_no` can be included with each request and will be added to the response accordingly. `japi_request_no` can be any valid JSON value.

\code
{
  ...
  "japi_request_no": "<request_number>",
  ...
}
\endcode

Example request with request number:
\code
{
  "japi_request": "some_request",
  "japi_request_no": "2020-02-20 20:22:20.202"
}
\endcode

Response including request number:
\code
{
  "japi_response": "some_request",
  "japi_request_no": "2020-02-20 20:22:20.202"
}
\endcode

### Response format
All requests return a response. The reponse format begins always with the JSON key \a japi_response followed by the request, that returned the response:
\code
{
  "japi_response": "<request_name>",
  "data": {
  	"<request-specific-return-key-1>": "<request-specific-return-value-1>",
  	"<request-specific-return-key-n>": "<request-specific-return-value-n>"
  }
}
\endcode

Like the JSON request the keys and values in \a data, are request specific; additional information, like return values of a function.

#### Response including request arguments
If configured via `japi_include_args_in_response(ctx, true)`, the response will additionally include the arguments given in the request:
\code
{
  "japi_response": "<request_name>",
  "args": {
    "<request-specific-key-1>": "<request-specific-value-1>",
    "<request-specific-key-n>": "<request-specific-value-n>"
  },
  "data": {
    "<request-specific-return-key-1>": "<request-specific-return-value-1>",
    "<request-specific-return-key-n>": "<request-specific-return-value-n>"
  }
}
\endcode

### Request and response format for push services
It's the same for the push services, with the difference that there is an additional key \a service or \a services to identify the service to be targeted by the requests.
For the push services like \a japi_pushsrv_subscribe, \a japi_pushsrv_unsubscribe, \a japi_pushsrv_list etc. an additional key value will be the \a service, or \a services for \a japi_pushsrv_list, so it's possible to backtrack the response to the origin of the request. Push service requests, responses and push-messages should look like this:
Push service list request:
\code
{
  "japi_request": "japi_pushsrv_list",
}
\endcode

Push service list response:
\note The service list in \a services can be empty if there is no push service registered.

\code
{
  "japi_response": "japi_pushsrv_list",
  "data": {
  	"services": [
  		"push-service-name-1",
  		"push-service-name-n"
  ]}
}
\endcode

Push service subscribe request:
\code
{
  "japi_request": "japi_pushsrv_subscribe",
  "args": {
  	"service": "<push_service_name>"
  }
}
\endcode

Push service subscribe response:
\code
{
  "japi_response": "japi_pushsrv_subscribe",
  "data": {
   	"service": "<push_service_name>",
   	"success": {true/false}
  }
}
\endcode

Push service unsubscribe request:
\code
{
  "japi_request": "japi_pushsrv_unsubscribe",
  "args": {
  	"service": "<push_service_name>"
  }
}
\endcode

Push service unsubscribe response:
\code
{
  "japi_response": "japi_pushsrv_unsubscribe",
  "data": {
   	"service": "<push_service_name>",
   	"success": {true/false}
  }
}
\endcode

Push service messages after successful subscribing a push service:

\note The push service messages just return the push service name, from which they are send.

\code
{
  "japi_pushsrv": "<push_service_name>",
  "data": {
  	"<push-service-specific-return-key-1>": "<push-service-specific-return-value-1>",
  	"<push-service-specific-return-key-n>": "<push-service-specific-return-value-n>"
  }
}
\endcode

\note In JSON the order of the key-value pair lines are not ensured.

### Communication example

<div style="width:1550px;">
\code
         Create JAPI context
         Register services
         Register push+services
         japi_start_server()
                                       Server                                              Client1                                            Client2
                                             +                                                +                                                 +
   Start registered push-services            +---------------------------> push_temperature() |  {                                              |
                                             |                                     +          |   "japi_request": "get_temperature",            |
                                             |                                     |          |   "args": {                                     |
                                             |                                     |          |     "unit": "kelvin"                            |
                                             |                                     |          |   }                                             |
                                             | <----------------------------------------------+  }                                              |
{                                            |                                     |          |                                                 |
 "japi_response": "get_temperature",         |                                     |          |                                                 |
 "data": {                                   |                                     |          |                                                 |
   "temperature": 300.0,                     |                                     |          |                                                 |
   "unit": "kelvin"                          |                                     |          |                                                 |
  }                                          |                                     |          |                                                 |
}                                            +----------------------------------------------> |                                                 |
                                             |                                     |          |                                                 | {
                                             |                                     |          |                                                 |  "japi_request": "japi_pushsrv_list",
                                             |                                     |          |                                                 | }
                                             | <------------------------------------------------------------------------------------------------+
{                                            |                                     |          |                                                 |
 "japi_response": "japi_pushsrv_list",       |                                     |          |                                                 |
 "data": {                                   |                                     |          |                                                 |
   "services": [                             |                                     |          |                                                 |
     "push_temperature",                     |                                     |          |                                                 |
 ]}                                          |                                     |          |                                                 |
}                                            +------------------------------------------------------------------------------------------------> |
                                             |                                     |          |                                                 |
                                             |                                     |          |                                                 |
                                             |                                     |          |  {                                              |
                                             |                                     |          |   "japi_request": "japi_pushsrv_subscribe",     |
                                             |                                     |          |   "args": {                                     |
                                             |                                     |          |     "service": "push_temperature"               |
                                             |                                     |          |   }                                             |
                                             |                                     |          |  }                                              |
{                                            | <----------------------------------------------+                                                 |
 "japi_response": "japi_pushsrv_subscribe",  |                                     |          |                                                 |
 "data": {                                   |                                     |          |                                                 | 
   "service": "push_temperature",            |                                     |          |                                                 |
   "success": true                           |                                     |          |                                                 |
  }                                          |                                     |          |                                                 |
}                                            +----------------------------------------------> |                                                 |
                                             |                                     |          |                                                 |
                                             | {                                   |          |                                                 |
                                             |  "data": {                          |          |                                                 |
                                             |    "temperature": 31.986693         |          |                                                 |
                                             |  },                                 +--------> |                                                 |
                                             |  "japi_pushsrv": "push_temperature" |          |                                                 |
                                             | }                                   |          |                                                 |
                                             |                                     +--------> |                                                 |
                                             |                                     |          |                                                 |{
                                             |                                     |          |                                                 | "japi_request": "japi_pushsrv_subscribe,
                                             |                                     |          |                                                 | "args": {
                                             |                                     +--------> |                                                 |   "service": "push_temperature"
                                             |                                     |          |                                                 | }
                                             |                                     |          |                                                 |}
{                                            + <-----------------------------------+----------+-------------------------------------------------+
 "japi_response": "japi_pushsrv_subscribe",  |                                     |          |                                                 |
 "data": {                                   |                                     |          |                                                 |
   "service": "push_temperature",            |                                     |          |                                                 |
   "success": true                           +------------------------------------------------------------------------------------------------> |
 }                                           |                                     |          |                                                 |
}                                            |                                     |          |                                                 |
                                             |                                     +----------------------------------------------------------> |
                                             |{                                    |          |                                                 |
                                             | "data": {                           |          |                                                 |
                                             |   "temperature": 42.136809          |          |                                                 |
                                             |  },                                 |          |                                                 |
                                             |  "japi_pushsrv": "push_temperature" +--------> |                                                 |
                                             | }                                   |          |                                                 |
                                             |                                     |          |                                                 |
                                             |                                     +----------------------------------------------------------> |
                                             |                                     |          |                                                 |
                                             |                                     |          | {                                               |
                                             |                                     |          |  "japi_request": "japi_pushsrv_unsubscribe",    |
                                             |                                     |          |  "args": {                                      |
                                             |                                     |          |    "service": "push_temperature"                |
                                             |                                     |          |  }                                              |
                                             |                                     |          | }                                               |
                                             | <----------------------------------------------+                                                 |
{                                            |                                     |          |                                                 | {
 "japi_response": "japi_pushsrv_unsubscribe",|                                     |          |                                                 |  "japi_request": "japi_pushsrv_unsubscribe",
 "data": {                                   |                                     |          |                                                 |  "args": {
   "service": "push_temperature",            +----------------------------------------------> |                                                 |    "service": "push_temperature"
   "success": true                           |                                     |          |                                                 |  }
 }                                           |                                     |          |                                                 | }
}                                            | <------------------------------------------------------------------------------------------------+
                                             |                                     |          |                                                 |
{                                            |                                     |          |                                                 |
 "japi_response": "japi_pushsrv_unsubscribe",|                                     |          |                                                 |
 "data": {                                   |                                     |          |                                                 |
   "service": "push_temperature",            +------------------------------------------------------------------------------------------------> |
   "success": true                           |                                     |          |                                                 |
 }                                           |                                     |          |                                                 |
}                                            v                                     v          v                                                 v


\endcode
</div>
## Examples

\anchor serverExample
### Server example
\dontinclude demo.cpp
Include required headers:
\skipline stdio
\skip japi
\until utils
Register services and start server:
\skip main
\until return ret
\line }

\anchor clientExample
### Client request example
\include test.py

## References
* https://github.com/json-c/json-c
* http://json-c.github.io/json-c/
* https://en.wikipedia.org/wiki/JSON
* https://alan-mushi.github.io/2014/10/28/json-c-tutorial-part-1.html
