
# Usage

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

Note, that "function_name" may not start with "japi_" which is used to mark internal commands.

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

## Default handler
There is a default `japi_request_not_found_handler` which reacts with an error
message if an unknown request is received. If you want to change that behavior,
you can register a `request_not_found_handler` which will then be used instead
to behave as you desire.
