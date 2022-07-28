
# Setup

## Prerequisites
- [json-c](https://github.com/json-c/json-c)
- [cmake version 3.6](https://cmake.org/)

## Installation

### Build
Clone the git repository and it's submodules:
\code
git clone git clone --recurse-submodules git@git01.iis.fhg.de:ks-ip-lib/software/libjapi.git
\endcode

Create a build directory in the libjapi repository and run `cmake`.
\code
cd libjapi
mkdir build/
cd build/
cmake ../
\endcode

A Makefile is generated. Run `make` to build the libjapi libraries.
\code
make
\endcode
A shared and a static library is built.

### Packages
Prebuild packages can be found <a href="../../repo/index.html">here</a>.

