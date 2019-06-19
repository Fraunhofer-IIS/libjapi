\mainpage

# libjapi

libjapi is a universial JSON API library. It receives newline-delimited JSON
(NDJSON) messages via TCP and calls registered C functions. A JSON response can
be returned for each request. Also it`s possible to register push services,
which push custom defined messages to the clients subscribed to them.

## Features
* Push Services
* Multi-Client support

## Getting started

### Prerequisites
- [json-c] (https://github.com/json-c/json-c)

### Installation
Run \a make in the libjapi repository.
\code
$ make
\endcode

## Demo
You can clone the demo project, with examples for all features from the repository listed below:

\code
$ git clone git@git01.iis.fhg.de:ks-ip-lib/software/libjapi-demo.git
\endcode

## Usage
* Create a JAPI context
* Write application specific functions
* Register these application specific functions to libjapi
* Start a JAPI server
* Enjoy the flexibility

### Examples

#### Server example
\dontinclude demo.cpp
Include required headers:
\skipline stdio
\skip japi
\until utils
Register services and start server:
\skip main
\until return ret
\line }

#### Client request example
\include test.py

## TODO
* Switch to CMake

## References
* https://github.com/json-c/json-c
* http://json-c.github.io/json-c/
* https://en.wikipedia.org/wiki/JSON
* https://alan-mushi.github.io/2014/10/28/json-c-tutorial-part-1.html
