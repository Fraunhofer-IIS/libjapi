# libjapi

libjapi is a universial JSON API library. It receives newline-delimited JSON
(NDJSON) messages via TCP and calls registered C functions. A JSON response can
be returned for each request.

How to make use of libjapi:
* Create a JAPI context
* Write application specific functions
* Register these application specific functions to libjapi
* Start a JAPI server
* Enjoy the flexibility


## TODO

* Improve README.md
* Review code
* Export japi_get_value_as_str() helper function 
* Provide default request_not_found_handler
* Switch to CMake


## References
* https://github.com/json-c/json-c
* http://json-c.github.io/json-c/
* https://en.wikipedia.org/wiki/JSON
* https://alan-mushi.github.io/2014/10/28/json-c-tutorial-part-1.html

