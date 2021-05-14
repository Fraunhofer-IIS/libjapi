
# Demo
You can clone the demo project, with examples for all features from the repository listed below:

```
$ git clone --recurse-submodules git@git01.iis.fhg.de:ks-ip-lib/software/libjapi-demo.git
```

First build the libjapi, the demo uses, in the `libjapi directory with:
```
$ make
```

Back in the `libjapi-demo directory run
```
$ make run-static
```
to start the server.

\note for the next step python3 needs to be installed.

Start the python client script in a separate tab with
```
$ ./test.py
```

The server can manage multiple clients. Try and run multiple clients simultaneously.

To end the client(s) send a CTRL-C. Same for the server.