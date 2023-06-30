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
Clone the git repository and it's submodules:

    $ git clone --recurse-submodules git@git01.iis.fhg.de:ks-ip-lib/software/libjapi.git

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

## References
* https://github.com/json-c/json-c
* http://json-c.github.io/json-c/
* https://en.wikipedia.org/wiki/JSON
* https://alan-mushi.github.io/2014/10/28/json-c-tutorial-part-1.html