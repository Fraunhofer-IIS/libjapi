
# Getting started

## Prerequisites
- [json-c](https://github.com/json-c/json-c)
- [cmake version 3.5](https://cmake.org/)

## Installation

### Build
Clone the git repository and update the submodule:
```
$ git clone git@git01.iis.fhg.de:ks-ip-lib/software/libjapi.git
$ git submodule init
$ git submodule update
```

Run `cmake` in the libjapi repository.
```
$ mkdir build/
$ cd build/
$ cmake ../
```

A Makefile is generated. Run `make` to build the libjapi libraries.
```
$ make
```
The shared and static libraries are to be found in the directory `make` was run.

### Packages
The packages can be found <a href="../../repo/index.html">here</a>.